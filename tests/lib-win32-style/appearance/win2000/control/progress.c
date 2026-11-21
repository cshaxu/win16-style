#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_PROGRESS_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
#ifdef WIN2K_PROGRESS_V6
static const COLORREF face=RGB(212,208,200),ink=RGB(10,36,106);
#endif
static const COLORREF app_ink=RGB(209,53,67),app_face=RGB(19,107,61);

static void print(HWND window,HDC dc,UINT message,UINT flags)
{
    RECT canvas={0,0,150,150};
    SetDCBrushColor(dc,RGB(189,45,153));
    FillRect(dc,&canvas,GetStockObject(DC_BRUSH));
    SendMessageW(window,message,(WPARAM)dc,flags);
}

#ifndef WIN2K_PROGRESS_V6
static BOOL has_color(HDC dc,const RECT *r,COLORREF desired)
{
    for(int y=r->top;y<r->bottom;++y)
        for(int x=r->left;x<r->right;++x)
            if(GetPixel(dc,x,y)==desired)return TRUE;
    return FALSE;
}
#endif

int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL production=argc==2 && !wcscmp(argv[1],L"--theme-launch");
#ifdef WIN2K_PROGRESS_V6
    BOOL outlined=argc==2 && !wcscmp(argv[1],L"--outlined-theme-launch");
    production=production || outlined;
#endif
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_PROGRESS_CLASS};
    CHECK(InitCommonControlsEx(&icc));
    HMODULE dll=NULL;
    window_op attach=NULL,attached=NULL,remove=NULL;
    if(!production){
        dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        CHECK(dll);
        attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
        attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
        remove=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
        CHECK(attach && attached && remove);
    }
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",L"Owned Win2000 progress",
        WS_POPUP|WS_VISIBLE,-10000,-10000,260,260,NULL,NULL,NULL,NULL);
    CHECK(root);
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),150,-150,1,32,BI_RGB}};
    void *pixels;
    HBITMAP bmp=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bmp);
    HGDIOBJ old=SelectObject(dc,bmp);
    CHECK(old && old!=HGDI_ERROR);
    const DWORD variants[]={0,PBS_SMOOTH,PBS_VERTICAL,PBS_VERTICAL|PBS_SMOOTH};

    for(unsigned i=0;i<ARRAYSIZE(variants);++i){
        BOOL vertical=(variants[i]&PBS_VERTICAL)!=0;
        HWND bar=CreateWindowW(PROGRESS_CLASSW,NULL,WS_CHILD|WS_VISIBLE|variants[i],
            0,0,vertical?24:130,vertical?130:24,root,NULL,NULL,NULL);
        CHECK(bar && (!hooked || attached(bar)));
#ifdef WIN2K_PROGRESS_V6
        CHECK(SendMessageW(bar,PBM_GETSTATE,0,0)==PBST_NORMAL);
#else
        CHECK(!SendMessageW(bar,PBM_GETSTATE,0,0));
        SendMessageW(bar,PBM_SETBARCOLOR,0,(LPARAM)app_ink);
        SendMessageW(bar,PBM_SETBKCOLOR,0,(LPARAM)app_face);
#endif
        CHECK(production || attach(bar));
        SendMessageW(bar,PBM_SETRANGE32,(WPARAM)(INT_PTR)-200,600);
        CHECK(SendMessageW(bar,PBM_SETPOS,0,0)==0);
        SendMessageW(bar,PBM_SETSTEP,100,0);
        CHECK(SendMessageW(bar,PBM_STEPIT,0,0)==0);
        CHECK(SendMessageW(bar,PBM_GETPOS,0,0)==100);
        RECT client;
        CHECK(GetClientRect(bar,&client) && client.right>2 && client.bottom>2);
        SetTextColor(dc,RGB(24,63,16));
        print(bar,dc,WM_PRINTCLIENT,PRF_CLIENT|PRF_ERASEBKGND);
#ifdef WIN2K_PROGRESS_V6
        if(outlined){
            int unit=max(1,MulDiv(1,(int)GetDpiForWindow(bar),96));
            int x=vertical?client.right/2:unit,y=vertical?client.bottom-unit-1:client.bottom/2;
            CHECK(GetPixel(dc,0,0)==RGB(0,0,0));
            SendMessageW(bar,PBM_SETBARCOLOR,0,(LPARAM)app_ink);
            const COLORREF state_colors[]={app_ink,RGB(192,0,0),RGB(192,192,0)};
            for(unsigned state=PBST_NORMAL;state<=PBST_PAUSED;++state){
                SendMessageW(bar,PBM_SETSTATE,state,0);
                print(bar,dc,WM_PRINTCLIENT,PRF_CLIENT|PRF_ERASEBKGND);
                CHECK(SendMessageW(bar,PBM_GETSTATE,0,0)==(LRESULT)state);
                CHECK(GetPixel(dc,x,y)==state_colors[state-PBST_NORMAL]);
            }
            CHECK(DestroyWindow(bar));
            continue;
        }
        int start_x=vertical?client.right/2:1,start_y=vertical?client.bottom-2:client.bottom/2;
        CHECK(GetPixel(dc,0,0)==face && GetPixel(dc,start_x,start_y)==ink);
        CHECK(GetTextColor(dc)==RGB(24,63,16));
        EnableWindow(bar,FALSE);
        print(bar,dc,WM_PRINTCLIENT,PRF_CLIENT|PRF_ERASEBKGND);
        CHECK(GetPixel(dc,start_x,start_y)==ink);
        EnableWindow(bar,TRUE);
        SendMessageW(bar,PBM_SETBARCOLOR,0,(LPARAM)app_ink);
        SendMessageW(bar,PBM_SETBKCOLOR,0,(LPARAM)app_face);
        print(bar,dc,WM_PRINTCLIENT,PRF_CLIENT|PRF_ERASEBKGND);
        CHECK(GetPixel(dc,0,0)==app_face && GetPixel(dc,start_x,start_y)==app_ink);
        CHECK(SendMessageW(bar,PBM_SETSTATE,PBST_PAUSED,0)==PBST_NORMAL);
        CHECK(SendMessageW(bar,PBM_GETSTATE,0,0)==PBST_PAUSED);
        CHECK(SendMessageW(bar,PBM_SETSTATE,PBST_NORMAL,0)==PBST_PAUSED);
#else
        CHECK(has_color(dc,&client,app_ink));
#endif
        print(bar,dc,WM_PRINT,PRF_NONCLIENT);
        CHECK(GetPixel(dc,0,6)==RGB(128,128,128) && GetPixel(dc,6,0)==RGB(128,128,128));
        CHECK(production || (remove(bar) && !attached(bar)));
        CHECK(SendMessageW(bar,PBM_GETPOS,0,0)==100);
        print(bar,dc,WM_PRINTCLIENT,PRF_CLIENT|PRF_ERASEBKGND);
#ifdef WIN2K_PROGRESS_V6
        CHECK(SendMessageW(bar,PBM_GETBARCOLOR,0,0)==app_ink &&
            SendMessageW(bar,PBM_GETBKCOLOR,0,0)==app_face);
#else
        CHECK(has_color(dc,&client,app_ink));
#endif
        CHECK(DestroyWindow(bar));
    }
    HWND special=CreateWindowW(PROGRESS_CLASSW,NULL,WS_CHILD|PBS_MARQUEE,0,0,100,20,root,NULL,NULL,NULL);
    CHECK(special);
    if(!production)CHECK(!attached(special) && !attach(special) && GetLastError()==ERROR_NOT_SUPPORTED);
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);
    if(dll)FreeLibrary(dll);
#ifdef WIN2K_PROGRESS_V6
    if(outlined){
        puts("OUTLINED_PROGRESS_THEME_LAUNCH_OK variants=4 native_values=1 states=3 edge=1");
        return 0;
    }
#endif
    if(production)puts("WIN2K_PROGRESS_THEME_LAUNCH_OK variants=4 native_values=1 caller_colors=1 edge=1");
    else puts("WIN2K_PROGRESS_OK variants=4 native_values=1 caller_colors=1 edge=1 detach=4 marquee_native=1");
    return 0;
}
