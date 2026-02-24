#include "term_compat.h"

static Gui* s_gui;

void term_bind_gui(Gui* gui) { s_gui = gui; }
Gui* term_get_gui(void) { return s_gui; }