#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_TRACKBAR_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static unsigned scrolls,custom_parts;
static BOOL custom;
static HWND track;
static const COLORREF face=RGB(212,208,200),white=RGB(255,255,255),marker=RGB(198,12,125);

static LRESULT CALLBACK parent_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if((message==WM_HSCROLL || message==WM_VSCROLL) && (HWND)b==track)++scrolls;
    if(message==WM_NOTIFY && b){
        NMCUSTOMDRAW *draw=(NMCUSTOMDRAW *)b;
        if(draw->hdr.hwndFrom==track && draw->hdr.code==NM_CUSTOMDRAW){
            if(draw->dwDrawStage==CDDS_PREPAINT && custom)return CDRF_NOTIFYITEMDRAW;
            if(draw->dwDrawStage==CDDS_ITEMPREPAINT && draw->dwItemSpec==TBCD_THUMB){
                ++custom_parts;
                SetDCBrushColor(draw->hdc,marker);
                FillRect(draw->hdc,&draw->rc,GetStockObject(DC_BRUSH));
                return CDRF_SKIPDEFAULT;
            }
        }
    }
    return DefWindowProcW(window,message,a,b);
}

int wmain(int argc,WCHAR **argv)
{
    BOOL themed=argc==3 && !wcscmp(argv[2],L"theme-edge");
    CHECK(argc==2 || (argc==3 && (themed || !wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_BAR_CLASSES};
    CHECK(InitCommonControlsEx(&icc));
    HMODULE dll=NULL;
    window_op attach=NULL,attached=NULL,remove=NULL;
    COLORREF edge=0;
    if(themed){
        WCHAR *end;
        unsigned long value=wcstoul(argv[1],&end,16);
        CHECK(end!=argv[1] && !*end && value<=0xffffff);
        edge=(COLORREF)value;
    }else{
        dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        CHECK(dll);
        attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
        attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
        remove=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
        CHECK(attach && attached && remove);
    }
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=instance,.lpszClassName=L"OwnedWin2kTrack"};
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,cls.lpszClassName,L"",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,-10000,-10000,700,700,NULL,NULL,instance,NULL);
    CHECK(root);
    UINT dpi=GetDpiForWindow(root);
    int length=MulDiv(220,dpi,96),across=MulDiv(40,dpi,96);
    CHECK(length<640 && across<640);
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),640,-640,1,32,BI_RGB}};
    void *pixels;
    HBITMAP bmp=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bmp);
    HGDIOBJ old=SelectObject(dc,bmp);
    CHECK(old && old!=HGDI_ERROR);
    for(int vertical=0;vertical<2;++vertical){
        track=CreateWindowW(TRACKBAR_CLASSW,NULL,WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_BOTH|(vertical?TBS_VERT:0),
            0,0,vertical?across:length,vertical?length:across,root,NULL,instance,NULL);
        CHECK(track && (themed || ((!hooked || attached(track)) && attach(track))));
        SendMessageW(track,TBM_SETRANGE,TRUE,MAKELPARAM(0,10));
        SendMessageW(track,TBM_SETPOS,TRUE,5);
        RECT thumb,channel;
        SendMessageW(track,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);
        SendMessageW(track,TBM_GETCHANNELRECT,0,(LPARAM)&channel);
        int x=(thumb.left+thumb.right)/2,y=(thumb.top+thumb.bottom)/2;
        SendMessageW(track,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
        if(themed){
            COLORREF actual=GetPixel(dc,thumb.left,thumb.top);
            printf("THEME_TRACKBAR_EDGE axis=%d pixel=%06lx\n",vertical,actual);
            CHECK(actual==edge && SendMessageW(track,TBM_GETPOS,0,0)==5);
            CHECK(DestroyWindow(track));
            continue;
        }
        if(GetPixel(dc,x,y)!=face)
            fprintf(stderr,"square axis=%d dpi=%u thumb=%ld,%ld,%ld,%ld pixel=%06lx\n",
                vertical,dpi,thumb.left,thumb.top,thumb.right,thumb.bottom,GetPixel(dc,x,y));
        CHECK(GetPixel(dc,x,y)==face);
        CHECK(GetPixel(dc,thumb.left,thumb.top)==white);
        if(!vertical)CHECK(GetPixel(dc,channel.left+4,channel.top)==RGB(128,128,128));
        CHECK(!custom_parts);
        EnableWindow(track,FALSE);
        SendMessageW(track,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
        COLORREF a=GetPixel(dc,x,y),b=GetPixel(dc,x+1,y);
        CHECK(a!=b && (a==white || a==face) && (b==white || b==face));
        EnableWindow(track,TRUE);
        unsigned before=scrolls;
        SendMessageW(track,WM_KEYDOWN,vertical?VK_DOWN:VK_RIGHT,0);
        SendMessageW(track,WM_KEYUP,vertical?VK_DOWN:VK_RIGHT,0);
        CHECK(SendMessageW(track,TBM_GETPOS,0,0)==6 && scrolls>before);
        custom=TRUE;
        SendMessageW(track,TBM_SETPOS,TRUE,5);
        SendMessageW(track,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
        if(GetPixel(dc,x,y)!=marker || !custom_parts)
            fprintf(stderr,"custom axis=%d parts=%u pixel=%06lx center=%d,%d\n",vertical,custom_parts,GetPixel(dc,x,y),x,y);
        CHECK(GetPixel(dc,x,y)==marker && custom_parts);
        CHECK(remove(track) && !attached(track) && SendMessageW(track,TBM_GETPOS,0,0)==5);
        CHECK(DestroyWindow(track));
        custom=FALSE;
        custom_parts=0;
    }
    const DWORD points[]={0,TBS_TOP,TBS_VERT,TBS_VERT|TBS_TOP};
    for(unsigned i=0;!themed && i<ARRAYSIZE(points);++i){
        BOOL vertical=(points[i]&TBS_VERT)!=0,start=(points[i]&TBS_TOP)!=0;
        track=CreateWindowW(TRACKBAR_CLASSW,NULL,WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|points[i],
            0,0,vertical?across:length,vertical?length:across,root,NULL,instance,NULL);
        CHECK(track && (!hooked || attached(track)) && attach(track));
        SendMessageW(track,TBM_SETPOS,TRUE,50);
        RECT thumb;
        SendMessageW(track,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);
        SendMessageW(track,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
        int center=vertical?(thumb.top+thumb.bottom)/2:(thumb.left+thumb.right)/2;
        int tip=vertical?(start?thumb.left:thumb.right-1):(start?thumb.top:thumb.bottom-1);
        int adjacent=tip+(start?1:-1);
        CHECK(GetPixel(dc,vertical?tip:center,vertical?center:tip)==RGB(64,64,64));
        /* Keyboard Properties, first thumb, y134 has white, shadow, dark
         * immediately before the one-pixel dark tip at y135. */
        CHECK(GetPixel(dc,vertical?adjacent:center-1,vertical?center-1:adjacent)==white);
        CHECK(GetPixel(dc,vertical?adjacent:center,vertical?center:adjacent)==RGB(128,128,128));
        CHECK(GetPixel(dc,vertical?adjacent:center+1,vertical?center+1:adjacent)==RGB(64,64,64));
        CHECK(remove(track) && DestroyWindow(track));
    }
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);
    if(dll)FreeLibrary(dll);
    if(themed)puts("THEME_TRACKBAR_OK axes=2 position_preserved=1");
    else puts("WIN2K_TRACKBAR_OK rectangular=2 pointed=4 disabled=2 native_notifications=2 custom_draw=2 detach=6");
    return 0;
}
