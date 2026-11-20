#ifndef WIN16_CAPTION_ART_H
#define WIN16_CAPTION_ART_H

#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct win16_menu_art {
    CHAR check[14][15], arrow[11][8];
    BOOL available;
} win16_menu_art;

HRESULT win16_menu_art_read(HMODULE module,const WORD images[2],win16_menu_art *art);

#endif
