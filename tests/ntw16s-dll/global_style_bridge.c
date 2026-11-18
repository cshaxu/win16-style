#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/attachment.h"

static HWND footer;
static unsigned paints;
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_SIZE && footer){RECT client;if(GetClientRect(window,&client))
        MoveWindow(footer,0,max(0,client.bottom-20),client.right,20,TRUE);}
    if(message==WM_PAINT){PAINTSTRUCT ps;++paints;HDC dc=BeginPaint(window,&ps);HBRUSH brush=CreateSolidBrush(RGB(0,128,0));
        if(brush){FillRect(dc,&ps.rcPaint,brush);DeleteObject(brush);}EndPaint(window,&ps);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
static void pump(void)
{MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
static BOOL client_top_clean(HWND window)
{
    HDC dc=GetDC(window);if(!dc)return FALSE;BOOL clean=TRUE;
    for(int y=0;y<12 && clean;++y)for(int x=10;x<150;++x){COLORREF color=GetPixel(dc,x,y);if(color!=RGB(0,128,0)){printf("DIRTY x=%d y=%d color=%06lx\n",x,y,(unsigned long)color);clean=FALSE;break;}}
    ReleaseDC(window,dc);return clean;
}
static BOOL footer_aligned(HWND window)
{
    RECT client,child;POINT origin={0,0};
    if(!footer || !GetClientRect(window,&client) || !ClientToScreen(window,&origin) || !GetWindowRect(footer,&child))return FALSE;
    printf("FOOTER child_bottom=%ld client_bottom=%ld\n",child.bottom,origin.y+client.bottom);
    return child.bottom==origin.y+client.bottom;
}
static COLORREF button_face(HWND button)
{
    RECT client;
    if(!GetClientRect(button,&client))return CLR_INVALID;
    HDC screen=GetDC(button);
    if(!screen)return CLR_INVALID;
    HDC dc=CreateCompatibleDC(screen);
    HBITMAP bitmap=CreateCompatibleBitmap(screen,client.right,client.bottom);
    ReleaseDC(button,screen);
    HGDIOBJ previous=dc && bitmap?SelectObject(dc,bitmap):NULL;
    COLORREF color=CLR_INVALID;
    if(previous){
        PatBlt(dc,0,0,client.right,client.bottom,WHITENESS);
        SendMessageW(button,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
        color=GetPixel(dc,client.right/2,client.bottom/2);
        SelectObject(dc,previous);
    }
    if(bitmap)DeleteObject(bitmap);
    if(dc)DeleteDC(dc);
    return color;
}
static BOOL style_cycle(HWND window,HINSTANCE instance,BOOL (__cdecl *attached2k)(HWND))
{
    HWND button=CreateWindowW(L"BUTTON",L"",WS_CHILD|WS_VISIBLE,8,30,120,32,window,NULL,instance,NULL);
    if(!button || !attached2k)return FALSE;
    COLORREF native=button_face(button);
    if(native==CLR_INVALID)return FALSE;
    HWND later=NULL;
    const DWORD original=MAXDWORD;
    const DWORD path[]={original,NTW16S_GLOBAL_STYLE_16,NTW16S_GLOBAL_STYLE_2K,original,
        NTW16S_GLOBAL_STYLE_2K,NTW16S_GLOBAL_STYLE_16,original};
    for(unsigned i=1;i<ARRAYSIZE(path);++i){
        DWORD style=path[i],before=path[i-1];
        BOOL frame;
        if(style!=original){
            frame=ntw16s_global_window_apply_style(window,style);
            if(i==1){
                later=CreateWindowW(L"BUTTON",L"Option",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
                    140,30,90,32,window,NULL,instance,NULL);
                if(!later)return FALSE;
                SendMessageW(later,BM_SETCHECK,BST_CHECKED,0);
            }
            ntw16s_global_window_children(window,TRUE,style);
        }else{
            ntw16s_global_window_children(window,FALSE,before);
            frame=ntw16s_global_window_remove_style(window,before);
        }
        BOOL classic16=style==NTW16S_GLOBAL_STYLE_16,classic2k=style==NTW16S_GLOBAL_STYLE_2K;
        frame=frame && (GetPropW(window,L"Win2kStyle.Frame")!=NULL)==classic2k &&
            Ntw16sWindowAttached(window)==classic16;
        BOOL children=Ntw16sWindowAttached(button)==classic16 && Ntw16sWindowAttached(later)==classic16 &&
            attached2k(button)==classic2k && attached2k(later)==classic2k;
        BOOL data=SendMessageW(later,BM_GETCHECK,0,0)==BST_CHECKED;
        COLORREF color=button_face(button),expected=classic2k?RGB(212,208,200):classic16?RGB(192,192,192):native;
        printf("GLOBAL STYLE CYCLE step=%u from=%lu to=%lu frame=%d children=%d state=%d pixel=%06lx expected=%06lx\n",
               i,(unsigned long)before,(unsigned long)style,frame,children,data,(unsigned long)color,(unsigned long)expected);
        if(!frame || !children || !data || color!=expected)return FALSE;
    }
    puts("GLOBAL STYLE CYCLE PASS edges=6 frame=1 children=2 state=1 draw_restore=1");
    return TRUE;
}

static BOOL color_cycle_ok;
static BOOL (__cdecl *color_child_styled)(HWND);
static UINT_PTR CALLBACK color_cycle_hook(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    (void)lp;
    if(message==WM_INITDIALOG){
        if(!SetTimer(window,1,60,NULL))PostMessageW(window,WM_CLOSE,0,0);
        return 0;
    }
    if(message!=WM_TIMER || wp!=1)return 0;
    KillTimer(window,1);

    HWND ok=GetDlgItem(window,IDOK);
    RECT initial={0},active={0},restored={0};
    BOOL before=ok && color_child_styled && GetClientRect(window,&initial) &&
        !GetPropW(window,L"Win2kStyle.Frame") && !color_child_styled(ok);
    BOOL applied=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K);
    ntw16s_global_window_children(window,TRUE,NTW16S_GLOBAL_STYLE_2K);
    BOOL styled=applied && GetPropW(window,L"Win2kStyle.Frame") && color_child_styled && color_child_styled(ok) &&
        GetClientRect(window,&active) && EqualRect(&initial,&active);
    ntw16s_global_window_children(window,FALSE,NTW16S_GLOBAL_STYLE_2K);
    BOOL removed=ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K);
    BOOL native=removed && !GetPropW(window,L"Win2kStyle.Frame") && color_child_styled && !color_child_styled(ok) &&
        GetClientRect(window,&restored) && EqualRect(&initial,&restored);
    color_cycle_ok=before && styled && native;
    printf("GLOBAL COLOR local initial=%d styled=%d restored=%d\n",before,styled,native);
    PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)ok);
    return 1;
}

static BOOL color_cycle(HWND owner,BOOL (__cdecl *attached)(HWND))
{
    COLORREF custom[16]={0},initial=RGB(10,20,30);
    CHOOSECOLORW color={.lStructSize=sizeof(color),.hwndOwner=owner,.rgbResult=initial,
        .lpCustColors=custom,.Flags=CC_RGBINIT|CC_ENABLEHOOK,.lpfnHook=color_cycle_hook};
    color_child_styled=attached;
    BOOL selected=ChooseColorW(&color);
    DWORD error=CommDlgExtendedError();
    BOOL pass=color_cycle_ok && selected && !error && color.rgbResult==initial;
    printf("GLOBAL COLOR bits=%u selected=%d rgb=%d error=%lu pass=%d\n",
        (unsigned)(sizeof(void *)*8),selected,color.rgbResult==initial,(unsigned long)error,pass);
    return pass;
}

int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"GlobalStyleBridgeTest"};
    if(!RegisterClassW(&cls))return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"Bridge",WS_OVERLAPPEDWINDOW,80,80,300,200,NULL,NULL,instance,NULL);
    if(!window)return 2;
    HMENU menu=CreateMenu(),file=CreatePopupMenu(),help=CreatePopupMenu();
    if(!menu || !file || !help || !AppendMenuW(file,MF_STRING,1,L"Item") || !AppendMenuW(help,MF_STRING,2,L"About") ||
       !AppendMenuW(menu,MF_POPUP,(UINT_PTR)file,L"Game") || !AppendMenuW(menu,MF_POPUP,(UINT_PTR)help,L"Help") || !SetMenu(window,menu))return 2;
    footer=CreateWindowW(L"STATIC",L"Status",WS_CHILD|WS_VISIBLE,0,0,10,20,window,NULL,instance,NULL);if(!footer)return 2;
    ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);pump();
    BOOL apply2k=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                 GetPropW(window,L"Win2kStyle.Frame")!=NULL && !Ntw16sWindowAttached(window);
    BOOL switched16=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) &&
                    Ntw16sWindowAttached(window) && !GetPropW(window,L"Win2kStyle.Frame");
    BOOL switched2k=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                    !Ntw16sWindowAttached(window) && GetPropW(window,L"Win2kStyle.Frame")!=NULL;
    pump();UpdateWindow(window);BOOL client_clean=client_top_clean(window),footer_ok=footer_aligned(window);
    MENUBARINFO native_menu={.cbSize=sizeof(native_menu)};POINT client_origin={0,0};RECT client_rect,overlap,update;
    BOOL overlap_exists=GetMenuBarInfo(window,OBJID_MENU,0,&native_menu) && GetClientRect(window,&client_rect) &&
        ClientToScreen(window,&client_origin);
    if(overlap_exists){OffsetRect(&client_rect,client_origin.x,client_origin.y);
        overlap_exists=IntersectRect(&overlap,&client_rect,&native_menu.rcBar);}
    ValidateRect(window,NULL);unsigned before_paint=paints;SendMessageW(window,WM_NCPAINT,1,0);
    BOOL refreshed=GetUpdateRect(window,&update,FALSE),clean_immediate=client_top_clean(window);
    BOOL repainted=paints>before_paint;
    if(overlap_exists)printf("OVERLAP screen=%ld,%ld,%ld,%ld origin=%ld,%ld update=%d repainted=%d\n",overlap.left,overlap.top,overlap.right,overlap.bottom,client_origin.x,client_origin.y,refreshed,repainted);
    UpdateWindow(window);BOOL clean_after=client_top_clean(window);
    BOOL click_clean=TRUE;
    for(unsigned i=0;i<6 && click_clean;++i){
        SendMessageW(window,WM_NCACTIVATE,(WPARAM)(i&1),0);
        SendMessageW(window,WM_NCPAINT,1,0);
        SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(180,30));
        SendMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(180,30));
        click_clean=client_top_clean(window);
    }
    BOOL remove2k=ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K) && GetPropW(window,L"Win2kStyle.Frame")==NULL;
    BOOL apply16=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) && Ntw16sWindowAttached(window);
    BOOL remove16=ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_16) && !Ntw16sWindowAttached(window);
    BOOL reverse=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) &&
                 ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_16) &&
                 ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                 ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K);
    BOOL existing=Ntw16sWindowAttach(window) && Ntw16sWindowAttached(window);
    BOOL takeover=existing && ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                  !Ntw16sWindowAttached(window) && GetPropW(window,L"Win2kStyle.Frame")!=NULL;
    if(takeover)(void)ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K);
    if(Ntw16sWindowAttached(window))(void)Ntw16sWindowRemove(window);
    HMODULE module=GetModuleHandleW(sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll");
    BOOL (__cdecl *attach2k)(HWND)=module?(void *)GetProcAddress(module,"Ntw2ksWindowAttach"):NULL;
    BOOL existing2k=attach2k && attach2k(window);
    BOOL takeover16=existing2k && ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) &&
                    Ntw16sWindowAttached(window) && !GetPropW(window,L"Win2kStyle.Frame");
    if(takeover16)(void)ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_16);
    BOOL (__cdecl *attached2k)(HWND)=module?(void *)GetProcAddress(module,"Ntw2ksWindowAttached"):NULL;
    BOOL cycle=style_cycle(window,instance,attached2k);
    BOOL color=color_cycle(window,attached2k);
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    printf("GLOBAL STYLE BRIDGE bits=%u apply2k=%d switch16=%d switch2k=%d client_clean=%d immediate=%d clean_after=%d click_clean=%d footer=%d overlap=%d refreshed=%d remove2k=%d apply16=%d remove16=%d reverse=%d existing=%d takeover=%d existing2k=%d takeover16=%d\n",
           (unsigned)(sizeof(void *)*8),apply2k,switched16,switched2k,client_clean,clean_immediate,clean_after,click_clean,footer_ok,overlap_exists,refreshed,remove2k,apply16,remove16,reverse,existing,takeover,existing2k,takeover16);
    return apply2k && switched16 && switched2k && client_clean && clean_immediate && clean_after && click_clean && footer_ok &&
        (!overlap_exists || refreshed || repainted) && remove2k && apply16 && remove16 && reverse && takeover && takeover16 && cycle && color?0:3;
}
