#include "config.h"
#include "records.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    RecordEntry buf[RECORDS_BUFFER_SIZE];
    int head;   // 다음에 쓸 위치
    int count;  // 현재 저장 개수
} RecordRing;

static RecordRing g_ring;
static const char* kLogFile = "records.log";
static void records_load_from_file(void);


void records_init(void)
{
    records_load_from_file(); // 시작 시 파일의 최신 RECORDS_BUFFER_SIZE개를 링버퍼로 복원
}

//파일에 기록 1개 저장장
static void log_append(const RecordEntry* e)
{
    FILE* f = fopen(kLogFile, "ab");
    if (!f) return;
    fwrite(e, sizeof(*e), 1, f);
    fclose(f);
}

//기록 저장할 때 뭘 저장하느냐
void records_push(int stage, int score, int blocks_used)
{
    RecordEntry e;
    e.stage = stage;
    e.score = score;
    e.blocks_used = blocks_used;

    // 링버퍼 저장
    g_ring.buf[g_ring.head] = e;
    g_ring.head = (g_ring.head + 1) % RECORDS_BUFFER_SIZE;

    if (g_ring.count < RECORDS_BUFFER_SIZE)
        g_ring.count++;

    // 파일 append
    log_append(&e);
}

//로그 카운트 하는 함수
long records_log_count(void)
{
    FILE* f = fopen(kLogFile, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    if (sz < 0) return 0;
    return sz / (long)sizeof(RecordEntry);
}

//페이지 넘김용 함수수
int records_log_read_latest_range(long start_from_latest, int n, RecordEntry* out)
{
    if (n <= 0) return 0;

    long total = records_log_count();
    if (total <= 0) return 0;
    if (start_from_latest < 0) start_from_latest = 0;
    if (start_from_latest >= total) return 0;

    FILE* f = fopen(kLogFile, "rb");
    if (!f) return 0;

    int got = 0;
    for (int i = 0; i < n; i++)
    {
        long latest_index = (total - 1) - (start_from_latest + i);
        if (latest_index < 0) break;

        long off = latest_index * (long)sizeof(RecordEntry);
        if (fseek(f, off, SEEK_SET) != 0) break;
        if (fread(&out[got], sizeof(RecordEntry), 1, f) != 1) break;
        got++;
    }

    fclose(f);
    return got;
}

//파일에 저장된 기록 10개를 가져와서 기록
static void records_load_from_file(void)
{
    memset(&g_ring, 0, sizeof(g_ring));

    long total = records_log_count();
    if (total <= 0) return;

    int n = (total < RECORDS_BUFFER_SIZE) ? (int)total : RECORDS_BUFFER_SIZE;

    FILE* f = fopen(kLogFile, "rb");
    if (!f) return;

    long start = total - n;
    fseek(f, start * (long)sizeof(RecordEntry), SEEK_SET);

    size_t got = fread(g_ring.buf, sizeof(RecordEntry), n, f);
    fclose(f);

    g_ring.head  = (int)(got % RECORDS_BUFFER_SIZE);
    g_ring.count = (int)got;
}

int records_read_latest_page(int page, int page_size, RecordEntry* out)
{
    if (!out) return 0;
    if (page < 0) return 0;
    if (page_size <= 0) return 0;

    long total = records_log_count();
    if (total <= 0) return 0;

    long start_from_latest = (long)page * (long)page_size; // 최신(0)에서 얼마나 떨어져서 읽을지
    if (start_from_latest >= total) return 0;

    // ✅ 최신 페이지(page=0)만 링버퍼 사용
    if (page == 0)
    {
        int n = (g_ring.count < page_size) ? g_ring.count : page_size;

        for (int i = 0; i < n; i++)
        {
            int idx = (g_ring.head - 1 - i + RECORDS_BUFFER_SIZE) % RECORDS_BUFFER_SIZE;
            out[i] = g_ring.buf[idx];
        }
        return n;
    }

    // ✅ 그 외 페이지는 파일에서 읽기
    long remain = total - start_from_latest;
    int n = (remain < page_size) ? (int)remain : page_size;

    return records_log_read_latest_range(start_from_latest, n, out);
}