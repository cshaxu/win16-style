#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_UPDOWN_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static HWND spin;
static unsigned notices;
static BOOL cancel_step;
static const COLORREF face=RGB(212,208,200),dark=RGB(64,64,64),untouched=RGB(147,21,161);

static LRESULT CALLBACK parent_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_NOTIFY && b){
        const NMHDR *notice=(const NMHDR *)b;
        if(notice->hwndFrom==spin && notice->code==UDN_DELTAPOS){
            ++notices;
            return cancel_step?TRUE:FALSE;
        }
    }
    return DefWindowProcW(window,message,a,b);
}

static unsigned count(HDC dc,RECT r,COLORREF color)
{
    unsigned found=0;
    for(int y=r.top;y<r.bottom;++y)
        for(int x=r.left;x<r.right;++x)
            if(GetPixel(dc,x,y)==color)++found;
    return found;
}

static void print(HWND window,HDC dc,RECT r)
{
    SetDCBrushColor(dc,untouched);
    FillRect(dc,&r,GetStockObject(DC_BRUSH));
    SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
}

int wmain(int argc,WCHAR **argv)
{
    BOOL theme_symbol=argc==2 && !wcscmp(argv[1],L"--theme-symbol");
    BOOL native_metrics=argc==2 && !wcscmp(argv[1],L"--native-metrics");
    BOOL theme_metrics=native_metrics || (argc==2 && !wcscmp(argv[1],L"--theme-metrics"));
    BOOL theme_art=theme_metrics || theme_symbol || (argc==2 && !wcscmp(argv[1],L"--theme-art"));
    CHECK(argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_UPDOWN_CLASS};
    CHECK(InitCommonControlsEx(&icc));
    HMODULE dll=NULL;
    window_op attach=NULL,attached=NULL,observed=NULL,remove=NULL;
    if(theme_metrics){
        HMODULE runtime=GetModuleHandleW(sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll");
        attached=runtime?(void *)GetProcAddress(runtime,"Ntw16sWindowAttached"):NULL;
        observed=runtime?(void *)GetProcAddress(runtime,"Ntw2ksWindowAttached"):NULL;
        CHECK(attached && observed);
    }else if(!theme_art){
        dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        CHECK(dll);
        attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
        attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
        remove=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
        CHECK(attach && attached && remove);
    }
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=instance,.lpszClassName=L"OwnedWin2kUpDown"};
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,cls.lpszClassName,L"",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,-10000,-10000,320,300,NULL,NULL,instance,NULL);
    CHECK(root);
    HWND buddy=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE,0,80,100,25,root,NULL,instance,NULL);
    CHECK(buddy);
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),110,-110,1,32,BI_RGB}};
    void *bits;
    HBITMAP dib=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    CHECK(dc && dib);
    HGDIOBJ old=SelectObject(dc,dib);
    CHECK(old && old!=HGDI_ERROR);
    for(int axis=0;axis<2;++axis){
        spin=CreateWindowW(UPDOWN_CLASSW,NULL,WS_CHILD|WS_VISIBLE|(axis?UDS_HORZ:0),
            0,0,axis?39:19,axis?19:39,root,NULL,instance,NULL);
        CHECK(spin && (theme_art || ((!hooked || attached(spin)) && attach(spin))));
        SendMessageW(spin,UDM_SETRANGE32,0,40);
        SendMessageW(spin,UDM_SETPOS32,0,20);
        RECT r;
        CHECK(GetClientRect(spin,&r));
        SetTextColor(dc,RGB(5,21,69));
        print(spin,dc,r);
        if(theme_metrics){
            CHECK((!native_metrics==(attached(spin) || observed(spin))) && SendMessageW(spin,UDM_GETPOS32,0,0)==20);
            DWORD hash=2166136261u;
            for(int y=r.top;y<r.bottom;++y)
                for(int x=r.left;x<r.right;++x)hash=(hash^GetPixel(dc,x,y))*16777619u;
            printf("THEME_UPDOWN_METRICS axis=%d pixels=%08lx\n",axis,hash);
            CHECK(DestroyWindow(spin));
            continue;
        }
        if(theme_art){
            RECT interior={4,4,15,15};
            COLORREF edge=theme_symbol?RGB(255,255,255):RGB(0,0,0);
            CHECK(GetPixel(dc,0,0)==edge && SendMessageW(spin,UDM_GETPOS32,0,0)==20);
            printf("THEME_UPDOWN_ART axis=%d dark=%u shadow=%u\n",axis,
                count(dc,interior,RGB(0,0,0)),count(dc,interior,RGB(128,128,128)));
            CHECK(DestroyWindow(spin));
            continue;
        }
        CHECK(count(dc,r,face)>20 && count(dc,r,dark)>10 && count(dc,r,RGB(0,0,0))>10);
        CHECK(count(dc,r,untouched)==(unsigned)(axis?r.bottom:r.right));
        CHECK(GetTextColor(dc)==RGB(5,21,69));
        COLORREF raised=GetPixel(dc,0,3);
        EnableWindow(spin,FALSE);
        print(spin,dc,r);
        CHECK(count(dc,r,face)>20 && count(dc,r,RGB(0,0,0))==0);
        EnableWindow(spin,TRUE);
        POINT click={axis?r.right/4:r.right/2,axis?r.bottom/2:r.bottom/4};
        LPARAM location=MAKELPARAM(click.x,click.y);
        notices=0;cancel_step=TRUE;
        SendMessageW(spin,WM_LBUTTONDOWN,MK_LBUTTON,location);
        SendMessageW(spin,WM_LBUTTONUP,0,location);
        CHECK(notices && SendMessageW(spin,UDM_GETPOS32,0,0)==20);
        cancel_step=FALSE;
        SendMessageW(spin,WM_LBUTTONDOWN,MK_LBUTTON,location);
        print(spin,dc,r);
        CHECK(GetPixel(dc,0,3)!=raised);
        SendMessageW(spin,WM_LBUTTONUP,0,location);
        LRESULT value=SendMessageW(spin,UDM_GETPOS32,0,0);
        CHECK(notices>=2 && value!=20);
        CHECK(remove(spin) && !attached(spin) && SendMessageW(spin,UDM_GETPOS32,0,0)==value);
        CHECK(DestroyWindow(spin));
    }
    if(theme_art){
        CHECK(DestroyWindow(root));
        SelectObject(dc,old);DeleteObject(dib);DeleteDC(dc);
        puts("THEME_UPDOWN_ART_OK axes=2 position_preserved=1");
        return 0;
    }
    spin=CreateWindowW(UPDOWN_CLASSW,NULL,WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ARROWKEYS,
        110,80,16,26,root,NULL,instance,NULL);
    CHECK(spin && (!hooked || attached(spin)) && attach(spin));
    SendMessageW(spin,UDM_SETBUDDY,(WPARAM)buddy,0);
    CHECK((HWND)SendMessageW(spin,UDM_GETBUDDY,0,0)==buddy);
    SendMessageW(spin,UDM_SETRANGE32,0,50);
    SendMessageW(spin,UDM_SETPOS32,0,27);
    WCHAR value[12];
    CHECK(GetWindowTextW(buddy,value,ARRAYSIZE(value))==2 && !lstrcmpW(value,L"27"));
    CHECK(remove(spin) && (HWND)SendMessageW(spin,UDM_GETBUDDY,0,0)==buddy);
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);DeleteObject(dib);DeleteDC(dc);FreeLibrary(dll);
    puts("WIN2K_UPDOWN_OK axes=2 native_seam=2 disabled=2 cancel=2 buddy=1 detach=3");
    return 0;
}
