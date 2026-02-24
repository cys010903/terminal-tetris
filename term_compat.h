#ifndef TERM_COMPAT_H
#define TERM_COMPAT_H

#include "gui.h" // Gui는 SDL을 끌고오면 안 됨(opaque)

// SDL-only: term 레이어는 Gui 핸들만 노출
void term_bind_gui(Gui* gui);
Gui* term_get_gui(void);

#endif