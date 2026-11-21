#include <stdio.h>
#include "lib-win32-style/application/window/client_frame_lifecycle.h"
#include "lib-win32-style/application/menu/client_bar.h"

static HFONT failed_font;
static BOOL menu_owner;
static BOOL local_window(HWND window)
{return IsWindow(window);}
static style_client_frame *no_frame(HWND window)
{(void)window;return NULL;}
static DWORD fail_theme(style_client_frame *pending,BOOL use_colors)
{
    menu_owner=pending->menu_only;
    failed_font=CreateFontIndirectW(&(LOGFONTW){.lfHeight=-16});
    pending->fonts.title=failed_font;
    return use_colors?ERROR_RESOURCE_DATA_NOT_FOUND:ERROR_INVALID_PARAMETER;
}

static BOOL unpublished_on_theme_failure(void)
{
    static const style_client_frame_lifecycle_services services={
        .local_window=local_window,.state=no_frame,.load_theme=fail_theme
    };
    HWND window=CreateWindowExW(0,L"STATIC",L"Frame fixture",WS_OVERLAPPEDWINDOW,
        0,0,240,160,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window)return FALSE;
    RECT before,after;
    BOOL pass=GetWindowRect(window,&before);
    for(unsigned i=0;pass && i<2;++i){
        menu_owner=i==0;
        BOOL installed=i?style_client_frame_menu_owner_install(window,NULL,NULL,&services):
            style_client_frame_install(window,NULL,FALSE,NULL,NULL,&services);
        LOGFONTW font;
        pass=!installed && GetLastError()==ERROR_RESOURCE_DATA_NOT_FOUND && menu_owner==(i!=0) &&
            failed_font && !GetObjectW(failed_font,sizeof(font),&font) &&
            GetWindowRect(window,&after) && EqualRect(&before,&after);
    }
    DestroyWindow(window);
    return pass;
}

int main(void)
{
    style_client_frame *state=style_client_frame_state_create();
    if(!state)return 10;
    if(style_menu_bar_step(0,1,3,FALSE)!=1){
        style_client_frame_state_dispose(state);
        return 11;
    }
    style_client_frame_state_dispose(state);
    if(!unpublished_on_theme_failure())return 12;
    puts("WIN16_WINDOW_LINK_OK state=1 menu=1 theme_failure=2");
    return 0;
}
