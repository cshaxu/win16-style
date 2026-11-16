#ifndef WIN16_STYLE_SCROLL_LAYOUT_H
#define WIN16_STYLE_SCROLL_LAYOUT_H
#include "lib-win32-style/application/control/scroll_geometry.h"

/* Internal Win3.1 scrollbar geometry. The native-state adapter must supply an
 * effective min/max; modern nPage is not a thumb-size input. Coordinates are
 * caller-relative, with the same model for horizontal and vertical bars. */
typedef struct win16_scroll_metrics { int arrow,thumb,border; } win16_scroll_metrics;
BOOL win16_scroll_measure(const RECT *bar,BOOL vertical,
    const win16_scroll_metrics *metrics,LONG minimum,LONG maximum,LONG position,
    style_scroll_layout *layout);
style_scroll_part win16_scroll_hit(const style_scroll_layout *layout,POINT point,
    UINT disabled_arrows);
#endif
