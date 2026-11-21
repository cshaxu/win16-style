/* Windows 2000 comctl32 progress.c: ProPaint's physical inset, full blocks,
 * spacing and final clearance. Values and application colors come from the
 * shared native progress snapshot. */
#include "progress_paint.h"

BOOL win2k_progress_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    return !(style&(0xffffu&~(PBS_SMOOTH|PBS_VERTICAL))) &&
        !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_WINDOWEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
