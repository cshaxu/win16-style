#ifndef WIN2K_PROGRESS_PAINT_H
#define WIN2K_PROGRESS_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/progress_snapshot.h"

BOOL win2k_progress_paint(HDC,const style_progress_snapshot *,DWORD,const win2k_palette *);
BOOL win2k_progress_supported(HWND);
#endif
