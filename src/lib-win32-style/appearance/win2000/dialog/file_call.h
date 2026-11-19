#ifndef WIN2K_FILE_CALL_H
#define WIN2K_FILE_CALL_H
#include "lib-win32-style/application/dialog/classic_file_controller.h"
BOOL win2k_file_call_a(OPENFILENAMEA *,BOOL (WINAPI *)(OPENFILENAMEA *),const style_dialog_windows *);
BOOL win2k_file_call_w(OPENFILENAMEW *,BOOL (WINAPI *)(OPENFILENAMEW *),const style_dialog_windows *);
#endif
