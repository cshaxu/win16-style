#ifndef WIN2K_PALETTE_H
#define WIN2K_PALETTE_H
#include <windows.h>

/* USER color roles; the selected Windows 2000 standard scheme is recorded in
 * T27's original-image/source evidence. These are local paint inputs, never
 * a request to change the machine's system colors. */
typedef struct win2k_palette {
    COLORREF face,light,highlight,shadow,dark,text,gray,window;
} win2k_palette;
extern const win2k_palette win2k_standard_palette;
#endif
