#define _WIN32_WINNT 0x0A00
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/menu/native_system_menu.h"
static HMENU system_menu;static HWND popup;static UINT before_type,seen_type;static ULONG_PTR before_data,seen_data;static BOOL seen,checked,selected,custom;static COLORREF sample=CLR_INVALID,live_sample=CLR_INVALID;static unsigned idles,custom_draws;static BOOL hover,hover_checked,hover_preserved,win2k_paint,win2k_live,win2k_glyph,win2k_default;static unsigned glyph_rows;static BOOL default_changed;
static BOOL cancel_test,cancel_sent,cancel_timed_out,native_string;
static BOOL win2k_switch,switch_done,win2k_keyboard,key_sent,win2k_context;static int sampled_index=-1,first_index=-1;static COLORREF first_live=CLR_INVALID;
static style_native_menu mixed_art;
static unsigned long long row_pixels(HDC dc,RECT row,RECT window)
{
    unsigned long long hash=14695981039346656037ULL;
    for(int y=row.top-window.top;y<row.bottom-window.top;++y)
        for(int x=row.left-window.left;x<row.right-window.left;++x){
            hash^=GetPixel(dc,x,y);
            hash*=1099511628211ULL;
        }
    return hash;
}
static BOOL mixed_marks(HDC dc,RECT window)
{
    RECT row;
    int count=GetMenuItemCount(system_menu);
    if(count<1 || !GetMenuItemRect(NULL,system_menu,(UINT)(count-1),&row))return FALSE;
    RECT check=row,arrow=row;
    UINT dpi=GetDpiForWindow(popup);
    check.right=check.left+MulDiv(16,(int)dpi,96);
    arrow.left=arrow.right-MulDiv(11,(int)dpi,96);
    if(!style_native_system_menu_paint(popup,system_menu,dc,TRUE,&mixed_art))return FALSE;
    GdiFlush();
    unsigned long long check_before=row_pixels(dc,check,window),arrow_before=row_pixels(dc,arrow,window);
    style_native_menu blank=mixed_art;
    memset(blank.art.check,'1',sizeof(blank.art.check));
    memset(blank.art.arrow,'1',sizeof(blank.art.arrow));
    if(!style_native_system_menu_paint(popup,system_menu,dc,TRUE,&blank))return FALSE;
    GdiFlush();
    BOOL checked=check_before!=row_pixels(dc,check,window),expanded=arrow_before!=row_pixels(dc,arrow,window);
    printf("SYSTEM MENU THEME ART check=%d arrow=%d\n",checked,expanded);
    return checked && expanded;
}
static BOOL default_weight(HDC dc,RECT window)
{
    RECT row;
    UINT pos=GetMenuDefaultItem(system_menu,TRUE,GMDI_USEDISABLED);
    if(pos==(UINT)-1 || !GetMenuItemRect(NULL,system_menu,pos,&row))return FALSE;
    if(!style_native_system_menu_paint(popup,system_menu,dc,TRUE,NULL))return FALSE;
    GdiFlush();
    unsigned long long bold=row_pixels(dc,row,window);
    const style_native_menu normal={.metrics={22,0,16,16,
        STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_SHORTCUT_RIGHT|STYLE_MENU_COMMAND_GLYPHS}};
    if(!style_native_system_menu_paint(popup,system_menu,dc,TRUE,&normal))return FALSE;
    GdiFlush();
    BOOL changed=bold!=row_pixels(dc,row,window);
    printf("SYSTEM MENU THEME WEIGHT changed=%d\n",changed);
    return changed;
}
static BOOL command_glyph(HDC dc,RECT window)
{
    UINT pos=GetMenuDefaultItem(system_menu,TRUE,GMDI_USEDISABLED);
    RECT row;
    if(pos==(UINT)-1 || !GetMenuItemRect(NULL,system_menu,pos,&row))return FALSE;
    row.right=row.left+MulDiv(16,(int)GetDpiForWindow(popup),96);
    if(!style_native_system_menu_paint(popup,system_menu,dc,TRUE,NULL))return FALSE;
    GdiFlush();
    unsigned long long with=row_pixels(dc,row,window);
    const style_native_menu without={.metrics={22,0,16,16,
        STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_SHORTCUT_RIGHT|STYLE_MENU_TEXT_BOLD_DEFAULT}};
    if(!style_native_system_menu_paint(popup,system_menu,dc,TRUE,&without))return FALSE;
    GdiFlush();
    BOOL changed=with!=row_pixels(dc,row,window);
    printf("SYSTEM MENU COMMAND ART changed=%d\n",changed);
    return changed;
}
static BOOL CALLBACK find_popup(HWND w,LPARAM unused){(void)unused;WCHAR cls[32];MENUBARINFO mbi={.cbSize=sizeof(mbi)};if(IsWindowVisible(w)&&GetClassNameW(w,cls,32)&&!lstrcmpW(cls,L"#32768")&&GetMenuBarInfo(w,OBJID_CLIENT,0,&mbi)&&mbi.hMenu==system_menu){popup=w;return FALSE;}return TRUE;}
static BOOL sample_menu(void){
    if(!EnumThreadWindows(GetCurrentThreadId(),find_popup,0)&&!popup)return FALSE;
    int count=GetMenuItemCount(system_menu),selected_index=-1;RECT row,window;
    for(int i=0;i<count;++i){MENUITEMINFOW m={.cbSize=sizeof(m),.fMask=MIIM_STATE};if(GetMenuItemInfoW(system_menu,(UINT)i,TRUE,&m)&&(m.fState&MFS_HILITE)){selected_index=i;break;}}
    sampled_index=selected_index;
    if(selected_index<0||!GetMenuItemRect(NULL,system_menu,(UINT)selected_index,&row)||!GetWindowRect(popup,&window))return FALSE;
    if((win2k_live&&!custom)||native_string){POINT point={row.left+4,(row.top+row.bottom)/2};ScreenToClient(popup,&point);
        if(native_string)RedrawWindow(popup,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        HDC live=GetDC(popup);if(live){live_sample=GetPixel(live,point.x,point.y);ReleaseDC(popup,live);}}

    int width=window.right-window.left,height=window.bottom-window.top;if(width<=0||height<=0||width>2048||height>2048)return FALSE;
    HDC screen=GetDC(NULL),dc=screen?CreateCompatibleDC(screen):NULL;HBITMAP bitmap=dc?CreateCompatibleBitmap(screen,width,height):NULL;HGDIOBJ prev=bitmap?SelectObject(dc,bitmap):NULL;
    if(screen)ReleaseDC(NULL,screen);
    if(!prev||prev==HGDI_ERROR){if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);return FALSE;}
    SendMessageW(popup,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);
    if(win2k_paint && !style_native_system_menu_paint(popup,system_menu,dc,TRUE,NULL)){SelectObject(dc,prev);DeleteObject(bitmap);DeleteDC(dc);return FALSE;}
    GdiFlush();
    sample=GetPixel(dc,row.left-window.left+4,(row.top+row.bottom)/2-window.top);
    RECT label=row;label.left+=MulDiv(24,(int)GetDpiForWindow(popup),96);
    label.right-=MulDiv(12,(int)GetDpiForWindow(popup),96);
    if(!IsRectEmpty(&label))printf("SYSTEM MENU LABEL HASH %016llx\n",row_pixels(dc,label,window));
    if(win2k_paint){
        const style_native_menu shifted={.metrics={42,0,16,16,0}};
        unsigned long long original=row_pixels(dc,row,window);
        BOOL painted=style_native_system_menu_paint(popup,system_menu,dc,TRUE,&shifted);
        GdiFlush();
        BOOL changed=painted && original!=row_pixels(dc,row,window);
        printf("SYSTEM MENU THEME COLUMNS changed=%d\n",changed);
        if(!changed || !default_weight(dc,window) || !command_glyph(dc,window) ||
            (mixed_art.art.available && !mixed_marks(dc,window)))
            sample=CLR_INVALID;
    }
    if(win2k_default){
        UINT prior=GetMenuDefaultItem(system_menu,TRUE,0);RECT close_row;
        if(prior!=(UINT)-1 && GetMenuItemRect(NULL,system_menu,prior,&close_row)){
            unsigned long long first=14695981039346656037ULL,second=first;
            for(int y=close_row.top-window.top+2;y<close_row.bottom-window.top-2;++y)
                for(int x=close_row.left-window.left+22;x<close_row.right-window.left-16;++x){first^=GetPixel(dc,x,y);first*=1099511628211ULL;}
            SetMenuDefaultItem(system_menu,(UINT)-1,TRUE);
            SendMessageW(popup,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
            for(int y=close_row.top-window.top+2;y<close_row.bottom-window.top-2;++y)
                for(int x=close_row.left-window.left+22;x<close_row.right-window.left-16;++x){second^=GetPixel(dc,x,y);second*=1099511628211ULL;}
            default_changed=first!=second;SetMenuDefaultItem(system_menu,prior,TRUE);
        }
    }
    if(win2k_glyph){for(int k=0;k<count;++k){MENUITEMINFOW q={.cbSize=sizeof(q),.fMask=MIIM_BITMAP};RECT rr;
        if(!GetMenuItemInfoW(system_menu,(UINT)k,TRUE,&q)||!q.hbmpItem||!GetMenuItemRect(NULL,system_menu,(UINT)k,&rr))continue;
        COLORREF bg=GetPixel(dc,rr.left-window.left+2,(rr.top+rr.bottom)/2-window.top);BOOL found=FALSE;
        for(int y=rr.top-window.top+3;y<rr.bottom-window.top-3&&!found;++y)
            for(int x=rr.left-window.left+3;x<rr.left-window.left+16;++x)if(GetPixel(dc,x,y)!=bg){found=TRUE;break;}
        if(found)++glyph_rows;
    }}
    SelectObject(dc,prev);DeleteObject(bitmap);DeleteDC(dc);selected=TRUE;
    MENUITEMINFOW m={.cbSize=sizeof(m),.fMask=MIIM_FTYPE|MIIM_DATA};BOOL read=GetMenuItemInfoW(system_menu,0,TRUE,&m);checked=read&&m.fType==before_type&&m.dwItemData==before_data;
    printf("SYSTEM_DURING read=%d type=%x expected=%x data=%llu expecteddata=%llu\n",read,m.fType,before_type,(unsigned long long)m.dwItemData,(unsigned long long)before_data);
    return sample!=CLR_INVALID;
}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b){
    if(m==WM_INITMENUPOPUP&&HIWORD(b)){MENUITEMINFOW i={.cbSize=sizeof(i),.fMask=MIIM_FTYPE|MIIM_DATA};if(GetMenuItemInfoW((HMENU)a,0,TRUE,&i)){seen=TRUE;seen_type=i.fType;seen_data=i.dwItemData;}}
    if(m==WM_ENTERIDLE&&a==MSGF_MENU){++idles;
        /* Model a host retaining native string presentation, as modern Notepad
         * does. Leave item data intact for the adapter's normal cleanup. */
        if((hover||native_string)&&!hover_checked){int count=GetMenuItemCount(system_menu);
            for(int i=0;i<count;++i){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_FTYPE};
                if(GetMenuItemInfoW(system_menu,(UINT)i,TRUE,&item)){item.fType&=~MFT_OWNERDRAW;SetMenuItemInfoW(system_menu,(UINT)i,TRUE,&item);}}
        }
        if(sample_menu()){
            if(win2k_keyboard && !key_sent){
                first_index=sampled_index;first_live=live_sample;key_sent=TRUE;
                PostMessageW(w,WM_KEYDOWN,VK_UP,1);return 0;
            }
            if(win2k_keyboard && sampled_index==first_index)return 0;
            if(win2k_keyboard)switch_done=TRUE;
            if(win2k_switch && !switch_done){
                first_index=sampled_index;first_live=live_sample;int count=GetMenuItemCount(system_menu);
                for(int step=1;step<count;++step){int next=(first_index+step)%count;MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE|MIIM_FTYPE};
                    if(GetMenuItemInfoW(system_menu,(UINT)next,TRUE,&item) && !(item.fState&MFS_DISABLED) && !(item.fType&MFT_SEPARATOR)){
                        HiliteMenuItem(w,system_menu,(UINT)first_index,MF_BYPOSITION|MF_UNHILITE);
                        HiliteMenuItem(w,system_menu,(UINT)next,MF_BYPOSITION|MF_HILITE);
                        switch_done=TRUE;PostMessageW(w,WM_ENTERIDLE,MSGF_MENU,(LPARAM)popup);return 0;
                    }}
            }
            if(hover){RECT row;int count=GetMenuItemCount(system_menu);
                if(GetMenuItemRect(NULL,system_menu,(UINT)(count-1),&row)){
                    MapWindowPoints(NULL,popup,(POINT *)&row,2);
                    SendMessageW(popup,WM_MOUSEMOVE,0,MAKELPARAM(15,35));
                    HDC dc=GetDC(popup);int x=row.right-5,y=(row.top+row.bottom)/2;
                    COLORREF marker=RGB(243,17,219);SetPixel(dc,x,y,marker);ReleaseDC(popup,dc);
                    for(int i=0;i<3;++i)SendMessageW(popup,WM_MOUSEMOVE,0,MAKELPARAM(16+i,35));
                    dc=GetDC(popup);hover_preserved=GetPixel(dc,x,y)==marker;ReleaseDC(popup,dc);
                    hover_checked=TRUE;
                }
                printf("SYSTEM_HOVER unchanged_row=%d checked=%d\n",hover_preserved,hover_checked);
            }
            if(cancel_test){cancel_sent=TRUE;SendMessageW(w,WM_CANCELMODE,0,0);return 0;}
            EndMenu();return 0;}if(idles==1)PostMessageW(w,WM_KEYDOWN,VK_DOWN,1);}
    if(m==WM_TIMER&&a==1){cancel_timed_out=TRUE;KillTimer(w,1);EndMenu();if(win2k_context && !seen)PostQuitMessage(0);return 0;}
    if(m==WM_EXITMENULOOP)PostQuitMessage(0);
    if(m==WM_MEASUREITEM&&custom){MEASUREITEMSTRUCT *x=(MEASUREITEMSTRUCT *)b;if(x&&x->CtlType==ODT_MENU&&x->itemID==12345){x->itemWidth=80;x->itemHeight=20;return TRUE;}}
    if(m==WM_DRAWITEM&&custom){DRAWITEMSTRUCT *x=(DRAWITEMSTRUCT *)b;if(x&&x->CtlType==ODT_MENU&&x->itemID==12345){++custom_draws;FillRect(x->hDC,&x->rcItem,GetSysColorBrush(COLOR_MENU));return TRUE;}}
    return DefWindowProcW(w,m,a,b);
}
static RECT system_anchor;
static BOOL system_hit,system_init,system_idle,system_timeout;
static unsigned system_init_count;

static LRESULT CALLBACK slot_proc(HWND w,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_INITMENU && (HMENU)a==GetSystemMenu(w,FALSE))++system_init_count;
    if(message==WM_INITMENUPOPUP && (HMENU)a==GetSystemMenu(w,FALSE))system_init=TRUE;
    if(message==WM_ENTERIDLE && a==MSGF_MENU){
        RECT shown;
        system_idle=GetWindowRect((HWND)b,&shown) && abs(shown.left-system_anchor.left)<=3 &&
            abs(shown.top-system_anchor.bottom)<=3;
        EndMenu();
        return 0;
    }
    if(message==WM_TIMER && a==1){system_timeout=TRUE;EndMenu();return 0;}
    return DefWindowProcW(w,message,a,b);
}

static int check_system_slot(void)
{
    WNDCLASSW cls={.lpfnWndProc=slot_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"ThemeSystemSlot"};
    if(!RegisterClassW(&cls))return 1;
    HWND w=CreateWindowW(cls.lpszClassName,L"Theme system slot",WS_OVERLAPPEDWINDOW,80,80,400,220,
        NULL,NULL,cls.hInstance,NULL);
    if(!w)return 1;
    ShowWindow(w,SW_SHOWNORMAL);UpdateWindow(w);
    TITLEBARINFOEX info={.cbSize=sizeof(info)};
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    system_anchor=info.rgrect[1];
    POINT point={(system_anchor.left+system_anchor.right)/2,(system_anchor.top+system_anchor.bottom)/2};
    LPARAM screen=MAKELPARAM(point.x,point.y);
    system_hit=GetPropW(w,L"Win2kStyle.Frame") && !IsRectEmpty(&system_anchor) &&
        system_anchor.left>info.rcTitleBar.left+30 && SendMessageW(w,WM_NCHITTEST,0,screen)==HTSYSMENU;
    BOOL captured=FALSE;
    if(system_hit && SetTimer(w,1,2500,NULL)){
        SendMessageW(w,WM_NCLBUTTONDOWN,HTSYSMENU,screen);
        captured=GetCapture()==w;
        ScreenToClient(w,&point);
        if(captured)SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(point.x,point.y));
        KillTimer(w,1);
    }
    DestroyWindow(w);
    printf("WIN32 SYSTEM SLOT hit=%d captured=%d menu=%d anchor=%d timeout=%d init=%u\n",
        system_hit,captured,system_init,system_idle,system_timeout,system_init_count);
    return system_hit && captured && system_init && system_idle && !system_timeout && system_init_count==1?0:1;
}

int wmain(int argc,WCHAR **argv){
    if(argc==2 && !lstrcmpW(argv[1],L"system-slot"))return check_system_slot();
    if(argc!=2 && (argc!=3 || lstrcmpW(argv[1],L"win2k-paint")))return 2;
    cancel_test=!lstrcmpW(argv[1],L"native-cancel")||!lstrcmpW(argv[1],L"win2k-cancel");
    native_string=!lstrcmpW(argv[1],L"native-string");
    BOOL bar_height=!lstrcmpW(argv[1],L"menu-bar-height");
    custom=!lstrcmpW(argv[1],L"custom")||!lstrcmpW(argv[1],L"win2k-custom");hover=!lstrcmpW(argv[1],L"hover")||!lstrcmpW(argv[1],L"win2k-hover");win2k_paint=!lstrcmpW(argv[1],L"win2k-paint");win2k_glyph=!lstrcmpW(argv[1],L"win2k-glyph");win2k_default=!lstrcmpW(argv[1],L"win2k-default");win2k_switch=!lstrcmpW(argv[1],L"win2k-switch");win2k_keyboard=!lstrcmpW(argv[1],L"win2k-keyboard");win2k_context=!lstrcmpW(argv[1],L"win2k-context");win2k_live=!lstrcmpW(argv[1],L"win2k-cancel")||win2k_switch||win2k_keyboard||win2k_context||win2k_glyph||win2k_default||!lstrcmpW(argv[1],L"win2k-live")||!lstrcmpW(argv[1],L"win2k-hover")||!lstrcmpW(argv[1],L"win2k-custom");if(!bar_height&&!cancel_test&&!custom&&!hover&&!native_string&&!win2k_paint&&!win2k_live&&lstrcmpW(argv[1],L"normal"))return 2;
    WNDCLASSW cls={.lpfnWndProc=proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"Win16SysMenuWitness"};if(!RegisterClassW(&cls))return 4;
    HMENU bar=CreateMenu(),sub=CreatePopupMenu();if(!bar||!sub||!AppendMenuW(sub,MF_STRING,42,L"Item")||!AppendMenuW(bar,MF_POPUP,(UINT_PTR)sub,L"&File"))return 5;
    HWND w=CreateWindowW(cls.lpszClassName,L"System menu witness",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,30,30,360,200,NULL,bar,cls.hInstance,NULL);if(!w)return 5;
    ShowWindow(w,SW_SHOWNORMAL);UpdateWindow(w);
    if(bar_height){
        POINT client={0};
        RECT outer={0};
        BOOL valid=ClientToScreen(w,&client) && GetWindowRect(w,&outer);
        if(valid)printf("MENU_BAR_HEIGHT dpi=%u offset=%ld\n",(unsigned)GetDpiForWindow(w),(long)(client.y-outer.top));
        DestroyWindow(w);
        return valid?0:6;
    }
    system_menu=GetSystemMenu(w,FALSE);if(!system_menu)return 6;if(custom&&!AppendMenuW(system_menu,MF_OWNERDRAW,12345,0))return 7;
    if(argc==3){
        classic_theme_mapping source={0};
        if(FAILED(classic_theme_open(argv[2],&source)))return 10;
        classic_theme_native_menu(&mixed_art,&source);
        classic_theme_close(&source);
        HMENU nested=CreatePopupMenu();
        if(!mixed_art.art.available || !nested || !AppendMenuW(nested,MF_STRING,12345,L"Child") ||
            !AppendMenuW(system_menu,MF_POPUP|MF_CHECKED,(UINT_PTR)nested,L"Theme artwork"))return 11;
    }
    MENUITEMINFOW i={.cbSize=sizeof(i),.fMask=MIIM_FTYPE|MIIM_DATA};if(!GetMenuItemInfoW(system_menu,0,TRUE,&i))return 8;before_type=i.fType;before_data=i.dwItemData;
    SetActiveWindow(w);SetFocus(w);RECT window={0};GetWindowRect(w,&window);
    if(!SetTimer(w,1,3000,NULL) || !(win2k_context?
        PostMessageW(w,WM_CONTEXTMENU,(WPARAM)w,MAKELPARAM(window.left+80,window.top+10)):
        PostMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ')))return 9;
    MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}KillTimer(w,1);
    BOOL intact=GetMenuItemInfoW(system_menu,0,TRUE,&i)&&i.fType==before_type&&i.dwItemData==before_data;
    BOOL style=sample==((win2k_paint||win2k_live)?RGB(0,0,128):RGB(164,200,240));printf("SYSTEM_MENU bits=%u custom=%d seen=%d unchanged=%d checked=%d selected=%d blue=%d sample=%06lx idles=%u custom_draws=%u\n",(unsigned)(sizeof(void*)*8),custom,seen,seen_type==before_type&&seen_data==before_data&&intact,checked,selected,style,(DWORD)sample,idles,custom_draws);
    if(win2k_live&&!custom)printf("WIN2K SYSTEM LIVE sample=%06lx\n",(unsigned long)live_sample);
    if(native_string)printf("WIN16 SYSTEM STRING LIVE sample=%06lx\n",(unsigned long)live_sample);
    if(win2k_glyph)printf("WIN2K SYSTEM GLYPHS rows=%u\n",glyph_rows);
    if(win2k_default)printf("WIN2K SYSTEM DEFAULT changed=%d\n",default_changed);
    if(win2k_switch)printf("WIN2K SYSTEM SWITCH changed=%d first=%06lx second=%06lx\n",switch_done && sampled_index!=first_index,(unsigned long)first_live,(unsigned long)live_sample);
    if(win2k_keyboard)printf("WIN2K SYSTEM KEYBOARD changed=%d first=%06lx second=%06lx\n",switch_done && sampled_index!=first_index,(unsigned long)first_live,(unsigned long)live_sample);
    if(cancel_test)printf("SYSTEM CANCEL sent=%d timed_out=%d\n",cancel_sent,cancel_timed_out);
    DestroyWindow(w);return (!cancel_test || (cancel_sent && !cancel_timed_out))&&(!win2k_live||custom||live_sample==RGB(0,0,128))&&(!native_string||live_sample==RGB(164,200,240))&&(!win2k_glyph||glyph_rows==4)&&(!win2k_default||default_changed)&&(!(win2k_switch||win2k_keyboard)||(switch_done && sampled_index!=first_index && first_live==RGB(0,0,128)))&&seen&&seen_type==before_type&&seen_data==before_data&&intact&&selected&&(custom?custom_draws>0:(!lstrcmpW(argv[1],L"native-cancel") || style))&&(!hover||(hover_checked&&hover_preserved))?0:1;
}

