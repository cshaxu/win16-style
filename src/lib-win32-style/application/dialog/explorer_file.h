#ifndef STYLE_EXPLORER_FILE_H
#define STYLE_EXPLORER_FILE_H
#include "lib-win32-style/application/dialog/classic_file_controller.h"
BOOL style_explorer_file_a(OPENFILENAMEA *,BOOL (WINAPI *)(OPENFILENAMEA *),const style_dialog_windows *);
BOOL style_explorer_file_w(OPENFILENAMEW *,BOOL (WINAPI *)(OPENFILENAMEW *),const style_dialog_windows *);
#endif
