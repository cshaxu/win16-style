#ifndef STYLE_GRAPHICS_FONT_PAIR_H
#define STYLE_GRAPHICS_FONT_PAIR_H
#include <windows.h>

/* The provider owns role-specific font/lease release. This object owns the
 * installed pair and DPI; next transfers independently acquired leases. A
 * failed replacement never destroys the old pair. */
typedef struct style_font_pair {HFONT title,menu;UINT dpi;} style_font_pair;
typedef void (*style_font_pair_release)(const style_font_pair *);
static inline BOOL style_font_pair_commit(style_font_pair *current,style_font_pair next,style_font_pair_release release)
{
    if(!next.title || !next.menu){release(&next);return FALSE;}
    release(current);*current=next;return TRUE;
}
static inline void style_font_pair_dispose(style_font_pair *current,style_font_pair_release release)
{release(current);*current=(style_font_pair){0};}
#endif
