#ifndef WIN2K_SCROLL_LAYOUT_H
#define WIN2K_SCROLL_LAYOUT_H
#include "lib-win32-style/application/control/scroll_geometry.h"

/* Pure geometry for either axis. nominal is the axis-specific system arrow
 * metric. SCROLLINFO supplies the native range, position and page size. */
BOOL win2k_scroll_measure(const RECT *,BOOL vertical,int nominal,const SCROLLINFO *,style_scroll_layout *);
#endif
