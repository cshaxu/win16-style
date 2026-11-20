#ifndef WIN2K_PALETTE_H
#define WIN2K_PALETTE_H
#include "lib-win32-style/application/graphics/paint_frame.h"

/* USER color roles; the selected Windows 2000 standard scheme is recorded in
 * T27's original-image/source evidence. These are local paint inputs, never
 * a request to change the machine's system colors. */
typedef style_relief_palette win2k_palette;
extern const win2k_palette win2k_standard_palette;
#endif
