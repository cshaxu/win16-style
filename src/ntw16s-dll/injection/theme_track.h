#ifndef NTW16S_THEME_TRACK_H
#define NTW16S_THEME_TRACK_H
#include <windows.h>
#include <uxtheme.h>

/* A ticket is valid only while no theme open/close has crossed the draw. */
typedef struct ntw16s_theme_ticket {
    ULONGLONG epoch;
    BOOL eligible;
    HTHEME theme;
} ntw16s_theme_ticket;

ntw16s_theme_ticket ntw16s_theme_open_begin(void);
#define NTW16S_THEME_OTHER 0
#define NTW16S_THEME_BUTTON 1
#define NTW16S_THEME_TEXTGLOW 2
void ntw16s_theme_open_end(ntw16s_theme_ticket, HTHEME, int);
void ntw16s_theme_open_cancel(void);
void ntw16s_theme_close_begin(HTHEME);
void ntw16s_theme_close_end(HTHEME, BOOL);
ntw16s_theme_ticket ntw16s_theme_button_begin(HTHEME);
BOOL ntw16s_theme_button_end(HTHEME, ntw16s_theme_ticket);
ntw16s_theme_ticket ntw16s_theme_glow_begin(HTHEME);
BOOL ntw16s_theme_glow_end(HTHEME, ntw16s_theme_ticket);
#endif
