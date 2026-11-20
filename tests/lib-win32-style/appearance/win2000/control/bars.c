#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_BARS_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return FALSE;}}while(0)
static const COLORREF highlight=RGB(255,255,255),shadow=RGB(128,128,128),dark=RGB(64,64,64);
static const COLORREF ink=RGB(195,29,83),clear=RGB(243,10,210);
static COLORREF toolbar_highlight=RGB(255,255,255),rebar_highlight=RGB(255,255,255),status_shadow=RGB(128,128,128);
static HWND custom_source,owner_source;
static unsigned custom_visits,owner_visits,commands;
static BOOL custom_paint;

static void fill(HDC dc,const RECT *r,COLORREF color)
{
    COLORREF previous=SetDCBrushColor(dc,color);
    FillRect(dc,r,GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc,previous);
}

static LRESULT CALLBACK parent_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_NOTIFY && b){
        const NMCUSTOMDRAW *draw=(void *)b;
        if(draw->hdr.hwndFrom==custom_source && draw->hdr.code==NM_CUSTOMDRAW){
            ++custom_visits;
            if(custom_paint && draw->dwDrawStage==CDDS_PREPAINT)return CDRF_NOTIFYITEMDRAW;
            if(custom_paint && draw->dwDrawStage==CDDS_ITEMPREPAINT){
                fill(draw->hdc,&draw->rc,ink);
                return CDRF_SKIPDEFAULT;
            }
        }
    }
    if(message==WM_DRAWITEM && b){
        const DRAWITEMSTRUCT *item=(void *)b;
        if(item->hwndItem==owner_source && item->itemData==451){
            ++owner_visits;
            fill(item->hDC,&item->rcItem,ink);
            return TRUE;
        }
    }
    if(message==WM_COMMAND && LOWORD(a)==101)++commands;
    return DefWindowProcW(window,message,a,b);
}

static void paint(HWND window,HDC dc)
{
    const RECT full={0,0,420,140};
    fill(dc,&full,clear);
    SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
}

static unsigned color_count(HDC dc,RECT r,COLORREF color)
{
    unsigned count=0;
    for(LONG y=r.top;y<r.bottom;++y)
        for(LONG x=r.left;x<r.right;++x)
            if(GetPixel(dc,x,y)==color)++count;
    return count;
}

static BOOL toolbar(HWND root,HDC dc,BOOL hooked,window_op attach,window_op attached,window_op remove)
{
    HWND bar=CreateWindowW(TOOLBARCLASSNAMEW,NULL,WS_CHILD|WS_VISIBLE|CCS_NOPARENTALIGN|CCS_NORESIZE,
        0,0,380,48,root,NULL,NULL,NULL);
    CHECK(bar && (!attach || ((!hooked || attached(bar)) && attach(bar))));
    SendMessageW(bar,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);
    HBITMAP image=CreateCompatibleBitmap(dc,12,12);
    HDC image_dc=CreateCompatibleDC(dc);
    CHECK(image && image_dc);
    HGDIOBJ previous=SelectObject(image_dc,image);
    const RECT image_rect={0,0,12,12};
    fill(image_dc,&image_rect,ink);
    SelectObject(image_dc,previous);
    DeleteDC(image_dc);
    HIMAGELIST images=ImageList_Create(12,12,ILC_COLOR32,1,1);
    CHECK(images && ImageList_Add(images,image,NULL)==0);
    DeleteObject(image);
    SendMessageW(bar,TB_SETIMAGELIST,0,(LPARAM)images);
    TBBUTTON items[]={
        {.iBitmap=0,.idCommand=101,.fsState=TBSTATE_ENABLED,.iString=(INT_PTR)L"Open"},
        {.iBitmap=8,.fsStyle=BTNS_SEP},
        {.iBitmap=I_IMAGENONE,.idCommand=102,.fsState=TBSTATE_ENABLED|TBSTATE_CHECKED,.fsStyle=BTNS_CHECK,.iString=(INT_PTR)L"Checked"},
        {.iBitmap=I_IMAGENONE,.idCommand=103,.iString=(INT_PTR)L"Disabled"}
    };
    CHECK(SendMessageW(bar,TB_ADDBUTTONS,ARRAYSIZE(items),(LPARAM)items));
    SendMessageW(bar,TB_AUTOSIZE,0,0);
    RECT open,pressed,sep,disabled;
    CHECK(SendMessageW(bar,TB_GETITEMRECT,0,(LPARAM)&open) && SendMessageW(bar,TB_GETITEMRECT,1,(LPARAM)&sep) &&
        SendMessageW(bar,TB_GETITEMRECT,2,(LPARAM)&pressed) && SendMessageW(bar,TB_GETITEMRECT,3,(LPARAM)&disabled));
    custom_source=bar;
    paint(bar,dc);
    CHECK(GetPixel(dc,open.left+3,open.top)==toolbar_highlight && GetPixel(dc,open.left+3,open.bottom-1)==dark);
    CHECK(GetPixel(dc,pressed.left+3,pressed.top)==dark && GetPixel(dc,disabled.left+3,disabled.top)==toolbar_highlight);
    CHECK(color_count(dc,open,ink)>=60 && sep.right>sep.left);
    CHECK((HIMAGELIST)SendMessageW(bar,TB_GETIMAGELIST,0,0)==images);
    custom_paint=TRUE;
    paint(bar,dc);
    CHECK(GetPixel(dc,open.left+3,open.top)==ink && custom_visits>=3);
    custom_paint=FALSE;
    LPARAM click=MAKELPARAM((open.left+open.right)/2,(open.top+open.bottom)/2);
    SendMessageW(bar,WM_LBUTTONDOWN,MK_LBUTTON,click);
    SendMessageW(bar,WM_LBUTTONUP,0,click);
    CHECK(commands==1);
    SetWindowLongPtrW(bar,GWL_STYLE,GetWindowLongPtrW(bar,GWL_STYLE)|TBSTYLE_FLAT);
    CHECK(SendMessageW(bar,TB_GETITEMRECT,1,(LPARAM)&sep) &&
        SendMessageW(bar,TB_GETITEMRECT,2,(LPARAM)&pressed));
    paint(bar,dc);
    int x=sep.left+(sep.right-sep.left-1)/2,y=(sep.top+sep.bottom)/2;
    CHECK(GetPixel(dc,x,y)==shadow && GetPixel(dc,x+1,y)==toolbar_highlight);
    CHECK(GetPixel(dc,pressed.left+3,pressed.top)==shadow);
    custom_paint=TRUE;
    paint(bar,dc);
    CHECK(GetPixel(dc,x,y)!=shadow || GetPixel(dc,x+1,y)!=toolbar_highlight);
    custom_paint=FALSE;
    CHECK(!remove || (remove(bar) && !attached(bar)));
    CHECK((HIMAGELIST)SendMessageW(bar,TB_GETIMAGELIST,0,0)==images);
    custom_source=NULL;
    CHECK(DestroyWindow(bar));
    ImageList_Destroy(images);
    return TRUE;
}

static BOOL rebar(HWND root,HDC dc,BOOL hooked,window_op attach,window_op attached,window_op remove)
{
    HWND bar=CreateWindowW(REBARCLASSNAMEW,NULL,WS_CHILD|WS_VISIBLE|CCS_NOPARENTALIGN|CCS_NORESIZE|RBS_BANDBORDERS,
        0,0,380,45,root,NULL,NULL,NULL);
    CHECK(bar && (!attach || ((!hooked || attached(bar)) && attach(bar))));
    HWND child=CreateWindowW(L"STATIC",L"Band client",WS_CHILD|WS_VISIBLE,0,0,70,25,bar,NULL,NULL,NULL);
    CHECK(child);
    REBARBANDINFOW band={
        .cbSize=REBARBANDINFOW_V3_SIZE,.fMask=RBBIM_STYLE|RBBIM_CHILD|RBBIM_CHILDSIZE|RBBIM_SIZE|RBBIM_ID,
        .fStyle=RBBS_GRIPPERALWAYS,.hwndChild=child,.cxMinChild=60,.cyMinChild=24,.cx=180,.wID=201
    };
    CHECK(SendMessageW(bar,RB_INSERTBANDW,(WPARAM)-1,(LPARAM)&band));
    RECT rect,child_rect;
    CHECK(SendMessageW(bar,RB_GETRECT,0,(LPARAM)&rect) && GetWindowRect(child,&child_rect));
    UINT dpi=GetDpiForWindow(bar);
    int unit=GetSystemMetricsForDpi(SM_CXBORDER,dpi),edge=GetSystemMetricsForDpi(SM_CXEDGE,dpi);
    int x=rect.left+2*unit,y=rect.top+edge;
    paint(bar,dc);
    CHECK(GetPixel(dc,x,y)==rebar_highlight && GetPixel(dc,x+3*unit-1,y+1)==shadow);
    CHECK(SendMessageW(bar,RB_IDTOINDEX,201,0)==0 && GetParent(child)==bar);
    band.fMask=RBBIM_COLORS;
    band.clrFore=RGB(0,0,0);
    band.clrBack=RGB(86,31,177);
    CHECK(SendMessageW(bar,RB_SETBANDINFOW,0,(LPARAM)&band));
    REBARBANDINFOW colors={.cbSize=REBARBANDINFOW_V3_SIZE,.fMask=RBBIM_COLORS};
    CHECK(SendMessageW(bar,RB_GETBANDINFOW,0,(LPARAM)&colors) && colors.clrBack==band.clrBack);
    paint(bar,dc);
    CHECK(GetPixel(dc,x+3*unit-1,y+1)!=shadow);
    band.fMask=RBBIM_STYLE;
    band.fStyle=RBBS_NOGRIPPER;
    CHECK(SendMessageW(bar,RB_SETBANDINFOW,0,(LPARAM)&band));
    paint(bar,dc);
    CHECK(GetPixel(dc,x+3*unit-1,y+1)!=shadow);
    CHECK((!remove || (remove(bar) && !attached(bar))) && GetParent(child)==bar);
    CHECK(DestroyWindow(bar));
    return TRUE;
}

static BOOL status(HWND root,HDC dc,BOOL hooked,window_op attach,window_op attached,window_op remove)
{
    HWND bar=CreateWindowW(STATUSCLASSNAMEW,NULL,WS_CHILD|WS_VISIBLE|CCS_NOPARENTALIGN|CCS_NORESIZE|SBARS_SIZEGRIP,
        0,0,380,32,root,NULL,NULL,NULL);
    CHECK(bar && (!attach || ((!hooked || attached(bar)) && attach(bar))));
    int parts[]={95,180,260,-1};
    RECT r[4];
    CHECK(SendMessageW(bar,SB_SETPARTS,ARRAYSIZE(parts),(LPARAM)parts));
    CHECK(SendMessageW(bar,SB_SETTEXTW,0,(LPARAM)L"Ready") && SendMessageW(bar,SB_SETTEXTW,1|SBT_POPOUT,(LPARAM)L"Pressed") &&
        SendMessageW(bar,SB_SETTEXTW,2|SBT_NOBORDERS,(LPARAM)L"Open") &&
        SendMessageW(bar,SB_SETTEXTW,3|SBT_OWNERDRAW,451));
    SendMessageW(bar,SB_SETBKCOLOR,0,RGB(20,117,182));
    for(unsigned i=0;i<ARRAYSIZE(parts);++i)CHECK(SendMessageW(bar,SB_GETRECT,i,(LPARAM)&r[i]));
    owner_source=bar;
    SetTextColor(dc,RGB(7,13,19));
    paint(bar,dc);
    CHECK(GetPixel(dc,r[0].left+3,r[0].top)==status_shadow && GetPixel(dc,r[0].left+3,r[0].bottom-1)==highlight);
    CHECK(GetPixel(dc,r[1].left+3,r[1].top)==highlight && GetPixel(dc,r[1].left+3,r[1].bottom-1)==status_shadow);
    CHECK(GetPixel(dc,r[2].left+3,r[2].top)!=status_shadow);
    CHECK(owner_visits && color_count(dc,r[3],ink)>30 && GetTextColor(dc)==RGB(7,13,19));
    CHECK(GetPixel(dc,r[2].right-8,(r[2].top+r[2].bottom)/2)==RGB(20,117,182));
    const RECT surface={0,0,420,140};
    fill(dc,&surface,clear);
    int saved=SaveDC(dc);
    CHECK(saved && IntersectClipRect(dc,r[0].left,r[0].top,r[0].right,r[0].bottom)!=ERROR);
    SendMessageW(bar,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    CHECK(RestoreDC(dc,saved));
    CHECK(GetPixel(dc,r[0].left+3,r[0].top)==status_shadow && GetPixel(dc,r[1].left+3,r[1].top)==clear);
    RECT client;
    CHECK(GetClientRect(bar,&client));
    POINT grip={client.right-3,client.bottom-3};
    CHECK(ClientToScreen(bar,&grip));
    CHECK(SendMessageW(bar,WM_NCHITTEST,0,MAKELPARAM(grip.x,grip.y))==HTBOTTOMRIGHT);
    CHECK(SendMessageW(bar,SB_SETTEXTW,3,(LPARAM)L"Last"));
    paint(bar,dc);
    RECT grip_area={client.right-16,client.bottom-14,client.right,client.bottom};
    CHECK(color_count(dc,grip_area,highlight)>3 && color_count(dc,grip_area,shadow)>3);
    SendMessageW(bar,SB_SIMPLE,TRUE,0);
    CHECK(SendMessageW(bar,SB_SETTEXTW,SB_SIMPLEID,(LPARAM)L"Simple"));
    paint(bar,dc);
    CHECK(SendMessageW(bar,SB_ISSIMPLE,0,0));
    WCHAR label[32];
    CHECK(GetWindowTextW(bar,label,ARRAYSIZE(label))==6 && !wcscmp(label,L"Simple"));
    SendMessageW(bar,SB_SIMPLE,FALSE,0);
    CHECK(!remove || (remove(bar) && !attached(bar)));
    CHECK(LOWORD(SendMessageW(bar,SB_GETTEXTW,0,(LPARAM)label))==5 && !wcscmp(label,L"Ready"));
    owner_source=NULL;
    CHECK(DestroyWindow(bar));
    return TRUE;
}

static BOOL run(int argc,WCHAR **argv)
{
    BOOL themed=argc==5 && !wcscmp(argv[1],L"--theme-colors");
    CHECK(themed || argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_BAR_CLASSES|ICC_COOL_CLASSES};
    CHECK(InitCommonControlsEx(&icc));
    HMODULE dll=NULL;
    window_op attach=NULL,attached=NULL,remove=NULL;
    if(themed){
        COLORREF *roles[]={&toolbar_highlight,&rebar_highlight,&status_shadow};
        for(size_t i=0;i<ARRAYSIZE(roles);++i){
            WCHAR *end;
            unsigned long value=wcstoul(argv[i+2],&end,16);
            CHECK(end!=argv[i+2] && !*end && value<=0xffffff);
            *roles[i]=(COLORREF)value;
        }
    }else{
        dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        CHECK(dll);
        attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
        attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
        remove=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
        CHECK(attach && attached && remove);
    }
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=instance,.lpszClassName=L"OwnedWin2kBars"};
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,cls.lpszClassName,L"",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,-10000,-10000,450,240,NULL,NULL,instance,NULL);
    CHECK(root);
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),420,-140,1,32,BI_RGB}};
    void *bits;
    HBITMAP dib=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    CHECK(dc && dib);
    HGDIOBJ old=SelectObject(dc,dib);
    CHECK(old && old!=HGDI_ERROR);
    CHECK(toolbar(root,dc,hooked,attach,attached,remove));
    CHECK(rebar(root,dc,hooked,attach,attached,remove));
    CHECK(status(root,dc,hooked,attach,attached,remove));
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);
    DeleteObject(dib);
    DeleteDC(dc);
    if(dll)FreeLibrary(dll);
    if(themed)printf("THEME_BARS_OK toolbar=%06lx rebar=%06lx status=%06lx\n",
        toolbar_highlight,rebar_highlight,status_shadow);
    else puts("WIN2K_BARS_OK toolbar=1 rebar=1 status=1");
    return TRUE;
}

static BOOL theme_flat_status(PCWSTR color)
{
    WCHAR *end;
    unsigned long status_edge=wcstoul(color,&end,16);
    CHECK(end!=color && !*end && status_edge<=0xffffff);
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_BAR_CLASSES};
    CHECK(InitCommonControlsEx(&icc));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",L"",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        -10000,-10000,450,180,NULL,NULL,NULL,NULL);
    CHECK(root);
    HWND status=CreateWindowW(STATUSCLASSNAMEW,NULL,WS_CHILD|WS_VISIBLE|CCS_NOPARENTALIGN|CCS_NORESIZE,
        0,60,380,32,root,NULL,NULL,NULL);
    CHECK(status);
    const int parts[]={95,-1};
    RECT rect;
    CHECK(SendMessageW(status,SB_SETPARTS,ARRAYSIZE(parts),(LPARAM)parts));
    CHECK(SendMessageW(status,SB_SETTEXTW,0,(LPARAM)L"Ready") && SendMessageW(status,SB_GETRECT,0,(LPARAM)&rect));
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),420,-140,1,32,BI_RGB}};
    void *bits;
    HBITMAP dib=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    CHECK(dc && dib);
    HGDIOBJ old=SelectObject(dc,dib);
    CHECK(old && old!=HGDI_ERROR);
    paint(status,dc);
    COLORREF edge=GetPixel(dc,rect.left+3,rect.top);
    printf("THEME_FLAT_STATUS_EDGE pixel=%06lx\n",edge);
    CHECK(edge==(COLORREF)status_edge && DestroyWindow(root));
    SelectObject(dc,old);DeleteObject(dib);DeleteDC(dc);
    puts("THEME_FLAT_STATUS_OK");
    return TRUE;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"--theme-flat-status"))return theme_flat_status(argv[2])?0:1;
    return run(argc,argv)?0:1;
}
