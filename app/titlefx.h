#pragma once

// 메뉴/타이틀에서만 켜고, 게임플레이에선 끄는 용도
void titlefx_set_enabled(int on);

// scene_manager에서만 호출 (외부 씬은 호출 금지)
void titlefx_update(int dt_ms);
void titlefx_render(void);