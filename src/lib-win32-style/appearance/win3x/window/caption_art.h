#ifndef WIN16_CAPTION_ART_H
#define WIN16_CAPTION_ART_H

#include "lib-win32-style/appearance/msstyles/classic_theme.h"

enum { WIN16_CAPTION_ART_WIDTH=19, WIN16_SYSTEM_ART_WIDTH=18, WIN16_CAPTION_ART_HEIGHT=18 };

typedef struct win16_caption_art {
    BYTE roles[CLASSIC_IMAGE_ROLE_COUNT+1][2][WIN16_CAPTION_ART_WIDTH*WIN16_CAPTION_ART_HEIGHT];
    BOOL available;
} win16_caption_art;

/* Copies palette roles out of the mapped resource. Call outside loader lock. */
HRESULT win16_caption_art_read(HMODULE module,const classic_caption_images *images,win16_caption_art *art);

typedef struct win16_menu_art {
    CHAR check[14][15], arrow[11][8];
    BOOL available;
} win16_menu_art;

HRESULT win16_menu_art_read(HMODULE module,const WORD images[2],win16_menu_art *art);

#endif
