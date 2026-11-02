#ifndef WIN16_DIALOG_CHROME_H
#define WIN16_DIALOG_CHROME_H
#include <windows.h>
typedef struct win16_dialog_chrome_services {
    BOOL (*grid)(HWND,const void *);
    BOOL (*scroll)(HWND);
    BOOL (*remove)(HWND,const void *);
    BOOL (*combo)(HWND,const void *);
} win16_dialog_chrome_services;
void win16_dialog_old_chrome(HWND window,const win16_dialog_chrome_services *services);
void win16_dialog_font_chrome(HWND window,const win16_dialog_chrome_services *services);
BOOL win16_dialog_is_font_template(HWND window);
void win16_dialog_color_initialized(HWND window,void (*found)(HWND),void (*remove_surface)(HWND));
#endif
