#include <stdio.h>
#include "lib/win16-control/palette.h"
#include "lib/win16-control/basic_paint.h"
#include "lib/win16-control/scroll_assets.h"
#include "lib/win16-control/menu_bar.h"
#include "lib/win16-control/font.h"
#include "lib/win16-control/frame_layout.h"
#include "lib/win16-control/frame_view.h"
#include "lib/win16-control/list_paint.h"
#include "lib/win16-control/theme_state.h"
#include <stdlib.h>

static int bar_paints;
static BOOL test_layout(HWND w,void *context,win16_style_layout *layout)
{(void)w;(void)context;SetRect(&layout->menu,0,0,100,20);return TRUE;}
static void test_bar_paint(HWND w,void *context){(void)w;(void)context;++bar_paints;}
static int test_bar_hit(void *context,POINT point){(void)point;return *(int *)context;}
static void test_frame_paint(HWND w,void *context,HDC dc)
{(void)w;if(dc)++*(int *)context;}
static int forwarded_tabs;
static LRESULT test_forward_tabs(HWND w,WPARAM count,LPARAM values)
{(void)w;(void)count;(void)values;++forwarded_tabs;return TRUE;}
static int theme_calls;
static HRESULT test_native_theme(HWND w,LPCWSTR app,LPCWSTR ids)
{if(!w && app && ids && !lstrcmpW(app,L"Test") && !lstrcmpW(ids,L"Demo"))++theme_calls;return S_FALSE;}
static int button_paints;
static BOOL test_button_layout(HWND w,void *context,win16_style_layout *layout)
{(void)w;(void)context;ZeroMemory(layout,sizeof(*layout));SetRect(&layout->outer,0,0,200,200);layout->minimize=layout->outer;return TRUE;}
static void test_button_paint(HWND w,void *context,int hit)
{(void)w;(void)context;if(hit==HTMINBUTTON)++button_paints;}
int main(void)
{
    if(win16_control_theme_set(NULL,L"Test",L"Demo",test_native_theme)!=S_FALSE || theme_calls!=1)return 23;
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    if(palette.face!=RGB(192,192,192) || palette.edge!=RGB(0,0,0))return 1;
    for(int i=WIN16_SCROLL_UP;i<=WIN16_SCROLL_RIGHT_DISABLED;++i)
        if(!win16_control_scroll_glyph((enum win16_control_scroll_asset)i))return 2;
    if(win16_control_scroll_glyph((enum win16_control_scroll_asset)-1))return 3;
    HDC dc=CreateCompatibleDC(NULL);
    HBITMAP bitmap=dc?CreateCompatibleBitmap(dc,20,20):NULL;
    HGDIOBJ previous=bitmap?SelectObject(dc,bitmap):NULL;
    if(!dc || !bitmap || !previous || previous==HGDI_ERROR)return 4;
    if(win16_control_dialog_colors(dc,&palette)!=(LRESULT)GetStockObject(DC_BRUSH) ||
       GetBkColor(dc)!=palette.light || GetTextColor(dc)!=palette.edge || GetDCBrushColor(dc)!=palette.light)return 19;
    RECT bounds={0,0,17,17};
    win16_control_scroll_arrow(dc,bounds,win16_control_scroll_glyph(WIN16_SCROLL_UP),&palette);
    BOOL valid=GetPixel(dc,0,0)==palette.edge;
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
    if(!valid)return 5;
    HMENU root=CreatePopupMenu(),child=CreatePopupMenu();
    if(!root || !child)return 6;
    BOOL arrows=win16_control_menu_bar_arrow(NULL,root,root,VK_LEFT) &&
        win16_control_menu_bar_arrow(NULL,root,root,VK_RIGHT) &&
        !win16_control_menu_bar_arrow(NULL,root,child,VK_LEFT) &&
        !win16_control_menu_bar_arrow(NULL,NULL,root,VK_RIGHT) &&
        !win16_control_menu_bar_arrow(NULL,root,root,VK_UP);
    DestroyMenu(child);DestroyMenu(root);
    if(!arrows)return 7;
    win16_style_bar_item items[3]={{.text=L"&File"},{.text=L"E&&xit\t&X"},{.text=L"&Edit",.state=MFS_DISABLED}};
    win16_style_bar sample={.count=3,.items=items};
    if(win16_control_menu_bar_mnemonic(&sample,L'f')!=0 ||
       win16_control_menu_bar_mnemonic(&sample,L'x')!=-1 ||
       win16_control_menu_bar_mnemonic(&sample,L'e')!=-1)return 16;
    if(win16_control_menu_bar_next(0,-1,3,TRUE)!=-2 ||
       win16_control_menu_bar_next(-2,-1,3,TRUE)!=2 ||
       win16_control_menu_bar_next(2,1,3,FALSE)!=0 ||
       win16_control_menu_bar_next(1,0,3,FALSE)!=1)return 9;
    HWND owner=NULL;HMENU active=NULL,root_menu=NULL;RECT button={0,0,10,10};
    int hover=2,chosen=1,target=-3,direction=0;BOOL escaped=FALSE,doubled=FALSE;LPARAM click=0;
    const win16_control_menu_input_view input={&owner,&root_menu,&active,&button,&hover,test_bar_hit,
        &chosen,&target,&direction,&escaped,&doubled,&click};
    MSG message={.message=WM_MOUSEMOVE,.lParam=MAKELPARAM(5,5)};
    win16_control_menu_input(&message,&input);
    if(message.message!=WM_NULL || target!=2)return 14;
    message.message=WM_KEYDOWN;message.wParam=VK_ESCAPE;
    win16_control_menu_input(&message,&input);
    if(!escaped || message.message!=WM_KEYDOWN)return 15;
    UINT dpi=0;HFONT title=NULL,menu=NULL;LOGFONTW info={0};
    if(!win16_control_refresh_frame_fonts(NULL,&dpi,&title,&menu) || dpi!=96 || !title || !menu ||
       GetObjectW(title,sizeof(info),&info)!=sizeof(info) || info.lfHeight!=-MulDiv(10,96,72) || info.lfWeight!=FW_BOLD){
        if(title)DeleteObject(title);
        win16_style_system_release(menu);return 8;
    }
    DeleteObject(title);win16_style_system_release(menu);
    HWND window=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,20,20,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window)return 10;
    int pressed=HTMINBUTTON;BOOL hot=FALSE,button_detached=FALSE;
    const win16_control_frame_button_view button_view={NULL,&pressed,&hot,&button_detached,test_button_layout,test_button_paint};
    if(!win16_control_frame_button_motion(window,&button_view,WM_MOUSEMOVE,MAKELPARAM(0,0)) || !hot || button_paints!=1 ||
       !win16_control_frame_button_cancel(window,&button_view,WM_CAPTURECHANGED) || pressed || hot || button_paints!=2){DestroyWindow(window);return 24;}
    BOOL tabs_detached=FALSE;HFONT tabs_font=(HFONT)GetStockObject(SYSTEM_FONT);int tabs_count=0,*tabs_positions=NULL;
    const win16_control_tabstops_view tabs_view={&tabs_detached,&tabs_font,&tabs_count,&tabs_positions,test_forward_tabs};
    int stops[]={4,8};
    if(!win16_control_set_list_tabs(window,&tabs_view,2,(LPARAM)stops) || tabs_count!=2 ||
       !tabs_positions || tabs_positions[0]<=0 || tabs_positions[1]<=tabs_positions[0] || forwarded_tabs!=1){
        free(tabs_positions);DestroyWindow(window);return 20;}
    if(win16_control_set_list_tabs(window,&tabs_view,1,0) || forwarded_tabs!=1){free(tabs_positions);DestroyWindow(window);return 21;}
    tabs_detached=TRUE;
    if(!win16_control_set_list_tabs(window,&tabs_view,0,0) || tabs_count!=2 || forwarded_tabs!=2){free(tabs_positions);DestroyWindow(window);return 22;}
    free(tabs_positions);
    int frame_paints=0;win16_control_frame_ncpaint(window,1,&frame_paints,test_frame_paint,test_frame_paint);
    if(frame_paints!=1){DestroyWindow(window);return 18;}
    if(win16_control_frame_set_cursor(window,(WPARAM)window,MAKELPARAM(HTCLIENT,0)) ||
       win16_control_frame_set_cursor(window,0,MAKELPARAM(HTLEFT,0)) ||
       !win16_control_frame_set_cursor(window,(WPARAM)window,MAKELPARAM(HTLEFT,0))){DestroyWindow(window);return 17;}
    BOOL detached=FALSE,keyboard=TRUE;HWND focus=NULL;int selected=1;UINT bar_dpi=96;HFONT bar_font=(HFONT)GetStockObject(SYSTEM_FONT);
    const win16_control_bar_state_view state={NULL,&detached,&keyboard,&focus,&selected,&bar_font,&bar_dpi,test_layout,test_bar_paint};
    win16_control_menu_bar_leave(window,&state,FALSE);
    if(keyboard || selected!=-1 || bar_paints!=1){DestroyWindow(window);return 12;}
    detached=TRUE;win16_control_menu_bar_leave(window,&state,FALSE);
    if(bar_paints!=1){DestroyWindow(window);return 13;}
    RECT outer;win16_style_layout layout={0};LRESULT hit=0;
    BOOL got_rect=GetWindowRect(window,&outer);
    SetRect(&layout.menu,0,0,20,20);
    BOOL got_hit=got_rect && win16_control_frame_hit_test(window,&layout,MAKELPARAM(outer.left+5,outer.top+5),&hit);
    BOOL close=TRUE;
    if(win16_control_menu_bar_item_screen(window,&layout,bar_font,bar_dpi,MAKELPARAM(outer.left+5,outer.top+5),&close)!=-1 || close){DestroyWindow(window);return 25;}
    RECT old_client,new_client;win16_control_client_screen(window,&old_client);
    SetWindowPos(window,NULL,0,0,140,120,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    win16_control_restore_client(window,&old_client);win16_control_client_screen(window,&new_client);
    BOOL restored=EqualRect(&old_client,&new_client);
    DestroyWindow(window);
    if(!got_hit || hit!=HTBORDER || !restored)return 11;
    puts("WIN16_CONTROL_STANDALONE_LINK_OK palette=1 glyphs=12 pixel=1");
    return 0;
}
