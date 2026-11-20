#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
typedef struct window_ops {window_op attach,attached,remove;BOOL hooked;} window_ops;
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_DATES_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return FALSE;}}while(0)
static const COLORREF white=RGB(255,255,255),clear=RGB(246,27,193);
static COLORREF title=RGB(10,36,106),face=RGB(212,208,200);
static const SYSTEMTIME first={.wYear=2000,.wMonth=2,.wDay=18,.wHour=9};
static HWND selection_source,picker_source;
static unsigned selection_notices,opened,closed,popup_pixels;
static COLORREF popup_color,popup_expected;
static HDC popup_dc;
static DWORD *surface_pixels;
static unsigned pixels(RECT,COLORREF);
static void print(HWND,HDC);

static LRESULT CALLBACK root_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_NOTIFY && b){
        const NMHDR *notice=(const NMHDR *)b;
        if(notice->hwndFrom==selection_source && notice->code==MCN_SELCHANGE)++selection_notices;
        if(notice->hwndFrom==picker_source && notice->code==DTN_DROPDOWN){
            ++opened;
            HWND popup=(HWND)SendMessageW(picker_source,DTM_GETMONTHCAL,0,0);
            RECT r;
            if(popup && GetClientRect(popup,&r) && r.right<=500 && r.bottom<=300){
                popup_color=(COLORREF)SendMessageW(popup,MCM_GETCOLOR,MCSC_TITLEBK,0);
                print(popup,popup_dc);
                popup_pixels=pixels(r,popup_expected);
            }
            PostMessageW(picker_source,WM_KEYDOWN,VK_ESCAPE,0);
            PostMessageW(picker_source,DTM_CLOSEMONTHCAL,0,0);
        }
        if(notice->hwndFrom==picker_source && notice->code==DTN_CLOSEUP)++closed;
    }
    return DefWindowProcW(window,message,a,b);
}

static unsigned pixels(RECT r,COLORREF color)
{
    unsigned count=0;
    DWORD pixel=((DWORD)GetRValue(color)<<16)|((DWORD)GetGValue(color)<<8)|GetBValue(color);
    GdiFlush();
    for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x)
        if((surface_pixels[y*500+x]&0xffffffu)==pixel)++count;
    return count;
}

static void clear_surface(HDC dc)
{
    RECT r={0,0,500,300};
    COLORREF old=SetDCBrushColor(dc,clear);
    FillRect(dc,&r,GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc,old);
}

static void print(HWND window,HDC dc)
{
    clear_surface(dc);
    SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
}

static BOOL same_date(const SYSTEMTIME *a,const SYSTEMTIME *b)
{return a->wYear==b->wYear && a->wMonth==b->wMonth && a->wDay==b->wDay;}

static BOOL calendar(HWND root,HDC dc,const window_ops *ops)
{
    HWND window=CreateWindowW(MONTHCAL_CLASSW,NULL,WS_CHILD|WS_VISIBLE|WS_TABSTOP,
        0,0,295,240,root,NULL,NULL,NULL);
    CHECK(window && (!ops->attach || ((!ops->hooked || ops->attached(window)) && ops->attach(window))));
    CHECK(SendMessageW(window,MCM_SETCURSEL,0,(LPARAM)&first));
    RECT bounds;
    CHECK(SendMessageW(window,MCM_GETMINREQRECT,0,(LPARAM)&bounds));
    CHECK(SetWindowPos(window,NULL,0,0,bounds.right+10,bounds.bottom+10,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER));
    CHECK((COLORREF)SendMessageW(window,MCM_GETCOLOR,MCSC_TITLEBK,0)==title);
    CHECK((COLORREF)SendMessageW(window,MCM_GETCOLOR,MCSC_TITLETEXT,0)==white);
    print(window,dc);
    CHECK(bounds.left>=0 && bounds.top>=0 && bounds.right<=500 && bounds.bottom<=300);
    CHECK(pixels(bounds,title)>200 && pixels(bounds,white)>1000);
    selection_source=window;
    SetFocus(window);
    SendMessageW(window,WM_KEYDOWN,VK_RIGHT,0);
    SYSTEMTIME selected={0};
    CHECK(SendMessageW(window,MCM_GETCURSEL,0,(LPARAM)&selected));
    CHECK(selected.wYear==2000 && selected.wMonth==2 && selected.wDay==19 && selection_notices);
    CHECK(!SendMessageW(window,MCM_SETCURSEL,0,(LPARAM)&(SYSTEMTIME){.wYear=2000,.wMonth=13,.wDay=1}));
    COLORREF user=RGB(180,26,121);
    SendMessageW(window,MCM_SETCOLOR,MCSC_TITLEBK,user);
    print(window,dc);
    CHECK(pixels(bounds,user)>200 && (COLORREF)SendMessageW(window,MCM_GETCOLOR,MCSC_TITLEBK,0)==user);
    SendMessageW(window,MCM_SETCOLOR,MCSC_TITLETEXT,white);
    CHECK(!ops->remove || (ops->remove(window) && !ops->attached(window)));
    CHECK((COLORREF)SendMessageW(window,MCM_GETCOLOR,MCSC_TITLEBK,0)==user &&
        (COLORREF)SendMessageW(window,MCM_GETCOLOR,MCSC_TITLETEXT,0)==white);
    SYSTEMTIME after;
    CHECK(SendMessageW(window,MCM_GETCURSEL,0,(LPARAM)&after) && same_date(&selected,&after));
    selection_source=NULL;
    CHECK(DestroyWindow(window));
    return TRUE;
}

static BOOL popup_calendar(HWND window,HDC dc,COLORREF background)
{
    picker_source=window;
    popup_dc=dc;
    popup_expected=background;
    opened=closed=popup_pixels=0;
    popup_color=CLR_INVALID;
    SetFocus(window);
    SendMessageW(window,WM_KEYDOWN,VK_F4,0);
    ULONGLONG end=GetTickCount64()+1500;
    while(opened && !closed && GetTickCount64()<end){
        MSG message;
        if(PeekMessageW(&message,NULL,0,0,PM_REMOVE))DispatchMessageW(&message);
        else MsgWaitForMultipleObjects(0,NULL,FALSE,25,QS_ALLINPUT);
    }
    picker_source=NULL;
    fprintf(stderr,"popup opens=%u closes=%u title_color=%06lx visible_pixels=%u\n",opened,closed,popup_color,popup_pixels);
    CHECK(opened==1 && closed==1 && popup_color==background && popup_pixels>200);
    return TRUE;
}

static BOOL picker(HWND root,HDC dc,const window_ops *ops)
{
    HWND window=CreateWindowW(DATETIMEPICK_CLASSW,NULL,WS_CHILD|WS_VISIBLE|WS_TABSTOP,
        0,0,230,36,root,NULL,NULL,NULL);
    CHECK(window && (!ops->attach || ((!ops->hooked || ops->attached(window)) && ops->attach(window))));
    CHECK(SendMessageW(window,DTM_SETSYSTEMTIME,GDT_VALID,(LPARAM)&first));
    CHECK(SendMessageW(window,DTM_SETFORMATW,0,(LPARAM)L"yyyy'-'MM'-'dd"));
    SYSTEMTIME value;
    CHECK(SendMessageW(window,DTM_GETSYSTEMTIME,0,(LPARAM)&value)==GDT_VALID && same_date(&value,&first));
    CHECK((COLORREF)SendMessageW(window,DTM_GETMCCOLOR,MCSC_TITLEBK,0)==title);
    RECT client;
    CHECK(GetClientRect(window,&client));
    DATETIMEPICKERINFO info={.cbSize=sizeof(info)};
    RECT button;
    if(SendMessageW(window,DTM_GETDATETIMEPICKERINFO,0,(LPARAM)&info))button=info.rcButton;
    else{
        button=client;
        button.left=button.right-GetSystemMetricsForDpi(SM_CXVSCROLL,GetDpiForWindow(window));
    }
    CHECK(!IsRectEmpty(&button) && button.left>=0 && button.top>=0 && button.right<=500 && button.bottom<=300);
    SetTextColor(dc,RGB(28,61,89));
    print(window,dc);
    CHECK(pixels(button,face)>20 && pixels(button,RGB(0,0,0))>3);
    CHECK(GetTextColor(dc)==RGB(28,61,89));
    RECT clipped={button.left,button.top,button.right-4,button.bottom};
    clear_surface(dc);
    int saved=SaveDC(dc);
    CHECK(saved && IntersectClipRect(dc,clipped.left,clipped.top,clipped.right,clipped.bottom)>NULLREGION);
    SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    CHECK(RestoreDC(dc,saved));
    CHECK(pixels(clipped,face)>20 && GetPixel(dc,button.right-2,button.top+3)==clear);
    CHECK(popup_calendar(window,dc,title));
    COLORREF background=RGB(86,21,137);
    SendMessageW(window,DTM_SETMCCOLOR,MCSC_TITLEBK,background);
    CHECK(popup_calendar(window,dc,background));
    EnableWindow(window,FALSE);
    print(window,dc);
    CHECK(pixels(button,face)>20 && pixels(button,RGB(0,0,0))==0);
    CHECK(!ops->remove || (ops->remove(window) && !ops->attached(window)));
    CHECK(!ops->remove || (COLORREF)SendMessageW(window,DTM_GETMCCOLOR,MCSC_TITLETEXT,0)==GetSysColor(COLOR_CAPTIONTEXT));
    CHECK((COLORREF)SendMessageW(window,DTM_GETMCCOLOR,MCSC_TITLEBK,0)==background);
    CHECK(SendMessageW(window,DTM_GETSYSTEMTIME,0,(LPARAM)&value)==GDT_VALID && same_date(&value,&first));
    CHECK(DestroyWindow(window));
    HWND spin=CreateWindowW(DATETIMEPICK_CLASSW,NULL,WS_CHILD|WS_VISIBLE|DTS_TIMEFORMAT|DTS_UPDOWN,
        0,0,180,29,root,NULL,NULL,NULL);
    CHECK(spin && (!ops->attach || (!ops->attached(spin) && !ops->attach(spin))));
    HWND child=FindWindowExW(spin,NULL,UPDOWN_CLASSW,NULL);
    CHECK(child && DestroyWindow(spin));
    return TRUE;
}

static BOOL run(int argc,WCHAR **argv)
{
    BOOL themed=argc==4 && !wcscmp(argv[1],L"--theme-colors");
    CHECK(themed || argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_DATE_CLASSES};
    CHECK(InitCommonControlsEx(&icc));
    HMODULE dll=NULL;
    window_ops ops={0};
    if(themed){
        COLORREF *roles[]={&title,&face};
        for(size_t i=0;i<ARRAYSIZE(roles);++i){
            WCHAR *end;
            unsigned long value=wcstoul(argv[i+2],&end,16);
            CHECK(end!=argv[i+2] && !*end && value<=0xffffff);
            *roles[i]=(COLORREF)value;
        }
    }else{
        dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        CHECK(dll);
        ops=(window_ops){(void *)GetProcAddress(dll,"Ntw2ksWindowAttach"),(void *)GetProcAddress(dll,"Ntw2ksWindowAttached"),
            (void *)GetProcAddress(dll,"Ntw2ksWindowRemove"),hooked};
        CHECK(ops.attach && ops.attached && ops.remove);
    }
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WNDCLASSW cls={.lpfnWndProc=root_proc,.hInstance=instance,.lpszClassName=L"OwnedWin2kDates"};
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,cls.lpszClassName,NULL,
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,-10000,-10000,520,360,NULL,NULL,instance,NULL);
    CHECK(root);
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),500,-300,1,32,BI_RGB}};
    void *bits;
    HBITMAP dib=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    CHECK(dc && dib);
    surface_pixels=bits;
    HGDIOBJ old=SelectObject(dc,dib);
    CHECK(old && old!=HGDI_ERROR);
    CHECK(calendar(root,dc,&ops));
    CHECK(picker(root,dc,&ops));
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);
    DeleteObject(dib);
    DeleteDC(dc);
    if(dll)FreeLibrary(dll);
    if(themed)printf("THEME_DATES_OK calendar=%06lx button=%06lx popup=2 clip=1\n",title,face);
    else puts("WIN2K_DATES_OK month=1 picker=1 native_state=1 popup=2 clip=1");
    return TRUE;
}

int wmain(int argc,WCHAR **argv)
{return run(argc,argv)?0:1;}
