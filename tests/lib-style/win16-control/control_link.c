#include "lib-style/common/menu_input.h"
#include <stdio.h>
#include "lib-style/win16-common/palette.h"
#include "lib-style/win16-control/basic_paint.h"
#include "lib-style/win16-control/scroll_assets.h"
#include "lib-style/win16-window/menu_bar.h"
#include "lib-style/win16-common/font.h"
#include "lib-style/win16-window/frame_layout.h"
#include "lib-style/win16-window/frame_view.h"
#include "lib-style/common/frame.h"
#include "lib-style/win16-window/frame_state.h"
#include "lib-style/win16-control/control_state.h"
#include "lib-style/win16-control/control_class.h"
#include "lib-style/win16-control/attachment_state.h"
#include "lib-style/win16-control/fileowner_message.h"
#include "lib-style/win16-control/list_paint.h"
#include "lib-style/win16-control/theme_kind.h"
#include "lib-style/common/theme_state.h"
#include "lib-style/win16-control/custom_draw.h"
#include "lib-style/win16-control/text_policy.h"
#include <dwmapi.h>
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
static int frame_policy_calls;static DWORD frame_policy_value;
static HRESULT WINAPI test_frame_policy(HWND w,DWORD attr,LPCVOID value,DWORD size)
{(void)w;if(attr!=DWMWA_NCRENDERING_POLICY || size!=sizeof(DWORD))return E_INVALIDARG;frame_policy_value=*(const DWORD *)value;++frame_policy_calls;return S_OK;}
static int draw_parts;
static BOOL custom_supported(HWND w){(void)w;return TRUE;}
static BOOL custom_part(HWND w,const NMCUSTOMDRAW *draw,const win16_style_palette *p)
{(void)w;(void)draw;(void)p;++draw_parts;return TRUE;}
static HRESULT test_native_theme(HWND w,LPCWSTR app,LPCWSTR ids)
{if(!w && app && ids && !lstrcmpW(app,L"Test") && !lstrcmpW(ids,L"Demo"))++theme_calls;return S_FALSE;}
static int button_paints;
static BOOL test_button_layout(HWND w,void *context,win16_style_layout *layout)
{(void)w;(void)context;ZeroMemory(layout,sizeof(*layout));SetRect(&layout->outer,0,0,200,200);layout->minimize=layout->outer;return TRUE;}
static void test_button_paint(HWND w,void *context,int hit)
{(void)w;(void)context;if(hit==HTMINBUTTON)++button_paints;}
static BOOL test_decode(HDC dc,const WCHAR *input,UINT count,WCHAR *output)
{(void)dc;for(UINT i=0;i<count;++i)output[i]=input[i];return TRUE;}
static BOOL test_report_caller(HMODULE module)
{return module==GetModuleHandleW(NULL);}
int main(void)
{
    if(win16_control_theme_kind(L"BUTTON")!=WIN16_CONTROL_THEME_BUTTON ||
       win16_control_theme_kind(L"combobox")!=WIN16_CONTROL_THEME_COMBOBOX ||
       win16_control_theme_kind(L"TEXTGLOW")!=WIN16_CONTROL_THEME_TEXTGLOW ||
       win16_control_theme_kind(NULL)!=WIN16_CONTROL_THEME_OTHER ||
       win16_control_theme_kind((LPCWSTR)(UINT_PTR)1)!=WIN16_CONTROL_THEME_OTHER ||
       win16_control_theme_kind(L"SCROLLBAR")!=WIN16_CONTROL_THEME_OTHER)return 104;
    if(!win16_control_vb_text_caller((const void *)test_report_caller,test_report_caller) ||
       win16_control_vb_text_caller((const void *)(UINT_PTR)1,test_report_caller) ||
       !win16_control_vb_label_caller((const void *)(UINT_PTR)1,TRUE) ||
       win16_control_vb_label_caller((const void *)(UINT_PTR)1,FALSE))return 106;
    win16_attachment_initial_classification initial={0};
    win16_control_vcl_candidates vcl={0};win16_control_vcl_extra_candidates extra={0};
    initial.owner=(HMODULE)(UINT_PTR)1;
    if(win16_attachment_eligibility(WS_CHILD,FALSE,FALSE,FALSE,FALSE,FALSE,&initial,&vcl,&extra)!=1)return 85;
    initial.owner=GetModuleHandleW(NULL);
    if(win16_attachment_eligibility(WS_CHILD,FALSE,FALSE,FALSE,FALSE,FALSE,&initial,&vcl,&extra)!=2)return 86;
    initial.tree=TRUE;initial.owner=(HMODULE)(UINT_PTR)1;
    if(win16_attachment_eligibility(WS_CHILD,FALSE,FALSE,FALSE,FALSE,FALSE,&initial,&vcl,&extra)!=2)return 87;
    initial.tree=FALSE;vcl.tab_candidate=TRUE;
    if(win16_attachment_eligibility(WS_CHILD,FALSE,FALSE,FALSE,FALSE,FALSE,&initial,&vcl,&extra)!=0)return 88;
    vcl.tab_candidate=FALSE;extra.vb_user_edge=TRUE;
    if(win16_attachment_eligibility(WS_CHILD,FALSE,FALSE,FALSE,FALSE,FALSE,&initial,&vcl,&extra)!=0)return 89;
    extra.vb_user_edge=FALSE;
    if(win16_attachment_eligibility(WS_CAPTION,FALSE,FALSE,FALSE,FALSE,FALSE,&initial,&vcl,&extra)!=0)return 90;
    puts("ATTACH_ELIGIBILITY_OK distinct_release=1 tree=1 vcl=1 edge=1 frame=1");
    attachment dispatch={0};
    if(win16_attachment_dispatch_kind(NULL,&dispatch,WM_PAINT,0,0,FALSE,FALSE,FALSE,FALSE)!=WIN16_ATTACHMENT_DISPATCH_NATIVE)return 91;
    dispatch.static_candidate=TRUE;
    if(win16_attachment_dispatch_kind(NULL,&dispatch,WM_PAINT,0,0,FALSE,FALSE,FALSE,FALSE)!=WIN16_ATTACHMENT_DISPATCH_STATIC)return 92;
    dispatch.detached=TRUE;
    if(win16_attachment_dispatch_kind(NULL,&dispatch,WM_PAINT,0,0,FALSE,FALSE,FALSE,FALSE)!=WIN16_ATTACHMENT_DISPATCH_NATIVE)return 93;
    dispatch.detached=FALSE;dispatch.modern_spin=TRUE;
    if(win16_attachment_dispatch_kind(NULL,&dispatch,WM_PRINTCLIENT,0,0,TRUE,TRUE,TRUE,FALSE)!=WIN16_ATTACHMENT_DISPATCH_STATUS)return 94;
    if(win16_attachment_dispatch_kind(NULL,&dispatch,WM_PRINTCLIENT,0,0,FALSE,TRUE,TRUE,FALSE)!=WIN16_ATTACHMENT_DISPATCH_COLOR)return 95;
    dispatch.cool_candidate=TRUE;
    if(win16_attachment_dispatch_kind(NULL,&dispatch,WM_PAINT,0,0,TRUE,TRUE,TRUE,FALSE)!=WIN16_ATTACHMENT_DISPATCH_COOL)return 96;
    dispatch.cool_candidate=FALSE;dispatch.spin=TRUE;
    if(!win16_attachment_spin_message(&dispatch,WM_LBUTTONDOWN) || win16_attachment_spin_message(&dispatch,WM_USER))return 97;
    if(win16_attachment_dispatch_kind(NULL,&dispatch,WM_USER,0,0,FALSE,FALSE,FALSE,TRUE)!=WIN16_ATTACHMENT_DISPATCH_STATIC)return 98;
    puts("ATTACH_DISPATCH_OK priority=1 detached=1 nested=1 spin=1");
    HDC policy_dc=CreateCompatibleDC(NULL);if(!policy_dc)return 81;
    if(!win16_control_report_wide_text(policy_dc,L"abc",3,TRUE) ||
       win16_control_report_wide_text(policy_dc,L"\x4e2d",1,TRUE) ||
       !win16_control_report_wide_text(policy_dc,NULL,1,FALSE) ||
       win16_control_report_wide_text(policy_dc,L"a",4097,TRUE) ||
       !win16_control_report_ansi_text(policy_dc,"abc",-1,TRUE) ||
       win16_control_report_ansi_text(policy_dc,NULL,0,TRUE) ||
       !win16_control_report_ansi_text(policy_dc,NULL,0,FALSE))return 82;
    LPCWSTR run=L"AB";UINT flags=ETO_GLYPH_INDEX;WCHAR *decoded=NULL;char *encoded=NULL;
    if(!win16_control_vb_glyph_prepare(policy_dc,&run,&flags,2,&decoded,&encoded,test_decode) ||
       flags || !decoded || !encoded || encoded[0]!='A' || encoded[1]!='B' || run!=decoded)return 101;
    HeapFree(GetProcessHeap(),0,decoded);HeapFree(GetProcessHeap(),0,encoded);
    run=L"\x4e2d";flags=ETO_GLYPH_INDEX;decoded=NULL;encoded=NULL;
    if(win16_control_vb_glyph_prepare(policy_dc,&run,&flags,1,&decoded,&encoded,test_decode) ||
       flags!=ETO_GLYPH_INDEX)return 102;
    if(decoded)HeapFree(GetProcessHeap(),0,decoded);
    if(encoded)HeapFree(GetProcessHeap(),0,encoded);
    run=L"A";flags=ETO_GLYPH_INDEX|ETO_RTLREADING;decoded=NULL;encoded=NULL;
    if(win16_control_vb_glyph_prepare(policy_dc,&run,&flags,1,&decoded,&encoded,test_decode) || decoded || encoded)return 103;
    SetTextAlign(policy_dc,TA_RTLREADING);
    if(win16_control_report_wide_text(policy_dc,L"a",1,TRUE) || win16_control_report_ansi_text(policy_dc,"a",1,TRUE))return 83;
    if(win16_control_static_text_font(policy_dc,L"a",1,DT_CALCRECT,(const void *)main,NULL,GetModuleHandleW(NULL)))return 84;
    DeleteDC(policy_dc);

    control_style *control=win16_control_state_create();
    if(!control || control->font || control->protocol || control->tab_positions || control->calls)return 38;
    control->tab_positions=malloc(2*sizeof(*control->tab_positions));
    control->font=win16_style_menu_font_acquire(96);
    control->combo_fonts[0]=win16_style_menu_font_acquire(96);control->combo_font_count=1;
    if(!control->tab_positions || !control->font || !control->combo_fonts[0]){win16_control_state_dispose(control);return 39;}
    win16_control_state_dispose(control);
    style_window *owned=win16_control_frame_state_create();
    if(!owned || owned->menus || owned->title_font || owned->menu_font || owned->calls)return 34;
    menu_loop outer_loop={.depth=1};owned->menus=&outer_loop;owned->menu_depth=2;
    win16_control_frame_state_exit_menu(owned);
    if(owned->menus!=&outer_loop || owned->menu_depth!=1){owned->menus=NULL;win16_control_frame_state_dispose(owned);return 37;}
    owned->menus=NULL;owned->menu_depth=0;
    owned->title_font=CreateFontW(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Arial");
    owned->menu_font=win16_style_menu_font_acquire(96);
    if(!owned->title_font || !owned->menu_font){win16_control_frame_state_dispose(owned);return 35;}
    HFONT former_title=owned->title_font;win16_control_frame_state_dispose(owned);
    LOGFONTW disposed;if(GetObjectW(former_title,sizeof(disposed),&disposed))return 36;
    if(style_theme_set(NULL,L"Test",L"Demo",test_native_theme)!=S_FALSE || theme_calls!=1)return 23;
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    NMCUSTOMDRAW custom={0};BOOL custom_active=FALSE;
    custom.dwDrawStage=CDDS_PREPAINT;
    if(win16_control_custom_draw_after(NULL,&custom,&custom_active,FALSE,CDRF_SKIPDEFAULT,&palette,custom_supported,custom_part)!=CDRF_SKIPDEFAULT || custom_active)return 26;
    if(win16_control_custom_draw_after(NULL,&custom,&custom_active,FALSE,CDRF_DODEFAULT,&palette,custom_supported,custom_part)!=(CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT) || !custom_active)return 27;
    custom.dwDrawStage=CDDS_ITEMPREPAINT;
    if(win16_control_custom_draw_after(NULL,&custom,&custom_active,FALSE,CDRF_DODEFAULT,&palette,custom_supported,custom_part)!=CDRF_DODEFAULT || draw_parts)return 28;
    if(win16_control_custom_draw_after(NULL,&custom,&custom_active,TRUE,CDRF_DODEFAULT,&palette,custom_supported,custom_part)!=CDRF_SKIPDEFAULT || draw_parts!=1)return 29;
    custom.dwDrawStage=CDDS_POSTPAINT;
    if(win16_control_custom_draw_after(NULL,&custom,&custom_active,TRUE,CDRF_DODEFAULT,&palette,custom_supported,custom_part)!=CDRF_DODEFAULT || custom_active)return 30;
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
    BOOL arrows=style_menu_bar_arrow(NULL,root,root,VK_LEFT,TRUE) &&
        style_menu_bar_arrow(NULL,root,root,VK_RIGHT,TRUE) &&
        !style_menu_bar_arrow(NULL,root,child,VK_LEFT,TRUE) &&
        !style_menu_bar_arrow(NULL,NULL,root,VK_RIGHT,TRUE) &&
        !style_menu_bar_arrow(NULL,root,root,VK_UP,TRUE);
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
    const DWORD separator_styles[]={SS_ETCHEDHORZ,SS_ETCHEDVERT,SS_ETCHEDFRAME,SS_LEFT};
    for(unsigned i=0;i<ARRAYSIZE(separator_styles);++i){
        HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD|separator_styles[i],0,0,10,10,window,NULL,GetModuleHandleW(NULL),NULL);
        if(!child || ntw16s_is_separator(child)!=(i<3)){DestroyWindow(window);return 107;}
        DestroyWindow(child);
    }
    HWND edit=CreateWindowW(L"EDIT",L"",WS_CHILD|SS_ETCHEDHORZ,0,0,10,10,window,NULL,GetModuleHandleW(NULL),NULL);
    if(!edit || ntw16s_is_separator(edit)){DestroyWindow(window);return 108;}
    DestroyWindow(edit);puts("CONTROL_SEPARATOR_OK etched=3 plain=1 nonstatic=1");
    HWND classic_combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,
        0,0,80,80,window,(HMENU)(INT_PTR)1136,GetModuleHandleW(NULL),NULL);
    if(!classic_combo || !win16_control_common_dialog_combo_valid(classic_combo,window) ||
       win16_control_common_dialog_combo_valid(classic_combo,NULL)){
        DestroyWindow(window);return 42;}
    SetWindowLongPtrW(classic_combo,GWLP_ID,1135);
    if(win16_control_common_dialog_combo_valid(classic_combo,window)){DestroyWindow(window);return 43;}
    DestroyWindow(classic_combo);
    control_style *font_state=win16_control_state_create();HFONT old_font=NULL,outbound_font=NULL;
    HFONT application_font=(HFONT)GetStockObject(SYSTEM_FONT);WPARAM font_parameter=(WPARAM)application_font;
    if(!font_state || !win16_control_state_prepare_font(window,font_state,WM_SETFONT,&font_parameter,TRUE,&old_font,&outbound_font) ||
       font_state->original_font!=application_font || font_parameter!=(WPARAM)application_font || !outbound_font){
        if(font_state)win16_control_state_dispose(font_state);
        DestroyWindow(window);return 40;}
    win16_style_system_release(outbound_font);win16_style_system_release(old_font);
    font_state->detached=TRUE;font_parameter=(WPARAM)application_font;
    if(win16_control_state_prepare_font(window,font_state,WM_SETFONT,&font_parameter,FALSE,&old_font,&outbound_font)){
        win16_control_state_dispose(font_state);DestroyWindow(window);return 41;}
    win16_control_state_dispose(font_state);
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
    int frame_paints=0;style_frame_ncpaint(window,1,&frame_paints,test_frame_paint,test_frame_paint);
    if(frame_paints!=1){DestroyWindow(window);return 18;}
    if(win16_control_frame_set_cursor(window,(WPARAM)window,MAKELPARAM(HTCLIENT,0)) ||
       win16_control_frame_set_cursor(window,0,MAKELPARAM(HTLEFT,0)) ||
       !win16_control_frame_set_cursor(window,(WPARAM)window,MAKELPARAM(HTLEFT,0))){DestroyWindow(window);return 17;}
    BOOL detached=FALSE,keyboard=TRUE;HWND focus=NULL;int selected=1;UINT bar_dpi=96;HFONT bar_font=(HFONT)GetStockObject(SYSTEM_FONT);
    BOOL frame_detached=TRUE,pending=TRUE,saved=TRUE;DWORD prior_policy=DWMNCRP_ENABLED;
    win16_control_frame_activate(window,&frame_detached,NULL,FALSE,test_frame_policy);
    if(frame_policy_calls!=1 || frame_policy_value!=DWMNCRP_DISABLED){DestroyWindow(window);return 31;}
    win16_control_frame_restore(window,&pending,&saved,&prior_policy,NULL,FALSE,test_frame_policy);
    if(frame_policy_calls!=1){DestroyWindow(window);return 32;}
    pending=FALSE;win16_control_frame_restore(window,&pending,&saved,&prior_policy,NULL,FALSE,test_frame_policy);
    if(frame_policy_calls!=2 || frame_policy_value!=prior_policy){DestroyWindow(window);return 33;}
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
