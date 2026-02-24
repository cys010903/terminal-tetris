// records.c
#include "config.h"
#include "records.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>     // fsync
#include <fcntl.h>
#endif

typedef struct {
    RecordEntry buf[RECORDS_BUFFER_SIZE];
    int head;   // 다음에 쓸 위치
    int count;  // 현재 저장 개수
} RecordRing;

static RecordRing g_ring;

// ✅ flush 트리거용(추가)
static int s_dirty = 0;

static const char* kRecordFile = "records.log"; // 바이너리 RecordEntry 전용
static const char* kClearFile  = "clears.log";  // 클리어 이벤트 전용(텍스트)

static void records_load_from_file(void);

// ------------------------------------------------------------
// 내부 유틸
// ------------------------------------------------------------
static void ring_reset(void)
{
    memset(&g_ring, 0, sizeof(g_ring));
}

static void file_sync_path(const char* path)
{
#if defined(__unix__) || defined(__APPLE__)
    FILE* f = fopen(path, "ab");  // append 모드로 열고 즉시 동기화
    if (!f) return;

    fflush(f);
    fsync(fileno(f));
    fclose(f);
#else
    (void)path;
#endif
}

// 파일에 RecordEntry 1개 바이너리 append
static void log_append_record_bin(const RecordEntry* e)
{
    FILE* f = fopen(kRecordFile, "ab");
    if (!f) return;
    fwrite(e, sizeof(*e), 1, f);
    fclose(f);
}

// records.log(바이너리) 엔트리 개수
long records_log_count(void)
{
    FILE* f = fopen(kRecordFile, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    if (sz < 0) return 0;
    return sz / (long)sizeof(RecordEntry);
}

// 최신에서 start_from_latest만큼 떨어진 지점부터 n개를 역순으로 읽기
int records_log_read_latest_range(long start_from_latest, int n, RecordEntry* out)
{
    if (n <= 0) return 0;

    long total = records_log_count();
    if (total <= 0) return 0;
    if (start_from_latest < 0) start_from_latest = 0;
    if (start_from_latest >= total) return 0;

    FILE* f = fopen(kRecordFile, "rb");
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

// 시작 시 records.log 마지막 RECORDS_BUFFER_SIZE개를 링버퍼로 복원
static void records_load_from_file(void)
{
    ring_reset();

    long total = records_log_count();
    if (total <= 0) return;

    int n = (total < RECORDS_BUFFER_SIZE) ? (int)total : RECORDS_BUFFER_SIZE;

    FILE* f = fopen(kRecordFile, "rb");
    if (!f) return;

    long start = total - n;
    fseek(f, start * (long)sizeof(RecordEntry), SEEK_SET);

    size_t got = fread(g_ring.buf, sizeof(RecordEntry), (size_t)n, f);
    fclose(f);

    g_ring.head  = (int)(got % RECORDS_BUFFER_SIZE);
    g_ring.count = (int)got;
}

// ------------------------------------------------------------
// public API
// ------------------------------------------------------------
void records_init(void)
{
    records_load_from_file(); // 시작 시 파일의 최신 RECORDS_BUFFER_SIZE개를 링버퍼로 복원
    s_dirty = 0;              // init 시점엔 dirty 아님
}

// 기록 저장
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

    // 파일 append (기존 기능 유지)
    log_append_record_bin(&e);

    // ✅ flush 트리거(추가)
    s_dirty = 1;
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

    // 최신 페이지(page=0)만 링버퍼 사용 (기존 기능 유지)
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

    // 그 외 페이지는 파일에서 읽기 (기존 기능 유지)
    long remain = total - start_from_latest;
    int n = (remain < page_size) ? (int)remain : page_size;

    return records_log_read_latest_range(start_from_latest, n, out);
}

// 스테이지 클리어 로그는 별도 파일로 분리 (기존 기능 유지)
void records_log_append_stage_clear(int stage)
{
    if (stage <= 0) return; // 무한모드는 저장 안 함(원하면 바꿔도 됨)

    FILE* f = fopen(kClearFile, "a");
    if (!f) return;

    // 예: #CLEAR stage=3 ts=1700000000
    fprintf(f, "#CLEAR stage=%d ts=%ld\n", stage, (long)time(NULL));
    fclose(f);

    // ✅ flush 트리거(추가)
    s_dirty = 1;
}

// clears.log를 읽어서 stage cleared 테이블 채우기 (기존 기능 유지)
void records_load_stage_clears(uint8_t* cleared, int cleared_bytes)
{
    if (!cleared || cleared_bytes <= 0) return;

    FILE* f = fopen(kClearFile, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, (int)sizeof(line), f)) {
        if (strncmp(line, "#CLEAR", 6) != 0) continue;

        int st = 0;
        if (sscanf(line, "#CLEAR stage=%d", &st) == 1) {
            if (st >= 0 && st < cleared_bytes) cleared[st] = 1;
        }
    }

    fclose(f);
}

// ✅ 종료 직전 1회 호출: “의미 있는 flush” 추가
// - push/clear는 이미 fclose로 끝나지만, OS 캐시까지 확실히 밀고 싶으면 fsync 필요
void records_flush(void)
{
    if (!s_dirty) return;

    file_sync_path(kRecordFile);
    file_sync_path(kClearFile);

    s_dirty = 0;
}