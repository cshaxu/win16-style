#include <windows.h>
#include <oleacc.h>
#include <stdio.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
#define CHECK(expr) do {if(!(expr)){fprintf(stderr,"WIN2K_SCROLL_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#expr);return 1;}} while(0)

static unsigned notice_count,notices[32];
static LONG full_track;
static HBRUSH custom;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_CTLCOLORSCROLLBAR && custom)return (LRESULT)custom;
    if(m==WM_VSCROLL || m==WM_HSCROLL){
        if(notice_count<ARRAYSIZE(notices))notices[notice_count++]=LOWORD(a);
        if(LOWORD(a)==SB_THUMBTRACK && b){
            SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_TRACKPOS};
            if(GetScrollInfo((HWND)b,SB_CTL,&info))full_track=info.nTrackPos;
        }
    }
    return DefWindowProcW(w,m,a,b);
}

static void clear(HDC dc)
{
    RECT r={0,0,460,420};
    SetDCBrushColor(dc,RGB(211,31,167));
    FillRect(dc,&r,GetStockObject(DC_BRUSH));
}

static BOOL groove(HDC dc,int x,int y)
{
    COLORREF a=GetPixel(dc,x,y),b=GetPixel(dc,x+1,y);
    return (a==RGB(255,255,255) && b==RGB(212,208,200)) ||
        (b==RGB(255,255,255) && a==RGB(212,208,200));
}

static BOOL window_bar(HWND window,HDC dc,int object)
{
    SCROLLBARINFO bar={.cbSize=sizeof(bar)};
    RECT outer;
    if(!GetScrollBarInfo(window,object,&bar) || !GetWindowRect(window,&outer))return FALSE;
    int x=bar.rcScrollBar.left-outer.left,y=bar.rcScrollBar.top-outer.top;
    clear(dc);
    SendMessageW(window,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT);
    return object==OBJID_VSCROLL?groove(dc,x+4,y+30):groove(dc,x+30,y+4);
}

int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    CHECK(dll);
    window_op attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
    window_op attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
    window_op remove=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
    CHECK(attach && attached && remove);
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW wc={.lpfnWndProc=parent_proc,.hInstance=module,.lpszClassName=L"OwnedWin2kScroll"};
    CHECK(RegisterClassW(&wc));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,wc.lpszClassName,L"Scroll",
        WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL|WS_VISIBLE,-10000,-10000,395,350,NULL,NULL,module,NULL);
    CHECK(root && (!hooked || attached(root)));
    if(!attached(root))CHECK(attach(root));
    HDC dc=CreateCompatibleDC(NULL);
    void *pixels;
    BITMAPINFO dib={.bmiHeader={sizeof(BITMAPINFOHEADER),460,-420,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&dib,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);
    CHECK(old && old!=HGDI_ERROR);
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMax=99999,.nPage=22000,.nPos=35000};
    SetScrollInfo(root,SB_HORZ,&info,TRUE);
    SetScrollInfo(root,SB_VERT,&info,TRUE);
    CHECK(window_bar(root,dc,OBJID_HSCROLL) && window_bar(root,dc,OBJID_VSCROLL));

    UINT dpi=GetDpiForWindow(root);
    int metric=GetSystemMetricsForDpi(SM_CXHSCROLL,dpi);
    for(int v=0;v<2;++v){
        HWND bar=CreateWindowW(L"SCROLLBAR",L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|(v?SBS_VERT:SBS_HORZ),
            10,18,v?metric:180,v?180:metric,root,NULL,module,NULL);
        CHECK(bar && (!hooked || attached(bar)));
        CHECK(attach(bar));
        SetScrollInfo(bar,SB_CTL,&info,TRUE);
        clear(dc);
        SetTextColor(dc,RGB(5,9,13));
        SendMessageW(bar,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
        CHECK(GetTextColor(dc)==RGB(5,9,13));
        CHECK(v?groove(dc,4,metric+5):groove(dc,metric+5,4));
        custom=CreateSolidBrush(RGB(11,33,66));
        CHECK(custom);
        clear(dc);
        SendMessageW(bar,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
        CHECK(GetPixel(dc,v?4:metric+5,v?metric+5:4)==RGB(11,33,66));
        CHECK(DeleteObject(custom));
        custom=NULL;
        notice_count=0;
        SendMessageW(bar,WM_KEYDOWN,VK_RIGHT,0);
        SendMessageW(bar,WM_KEYUP,VK_RIGHT,0);
        CHECK(notice_count==2 && notices[0]==SB_LINEDOWN && notices[1]==SB_ENDSCROLL);
        notice_count=0;
        SendMessageW(bar,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));
        SendMessageW(bar,WM_LBUTTONUP,0,MAKELPARAM(5,5));
        CHECK(notice_count==2 && notices[0]==SB_LINEUP && notices[1]==SB_ENDSCROLL && GetCapture()!=bar);

        int track=180-2*metric,thumb=MulDiv(track,(int)info.nPage,100000);
        int start=metric+MulDiv(track-thumb,info.nPos,100000-(int)info.nPage)+thumb/2;
        notice_count=0;full_track=0;
        SendMessageW(bar,WM_LBUTTONDOWN,MK_LBUTTON,v?MAKELPARAM(5,start):MAKELPARAM(start,5));
        SendMessageW(bar,WM_MOUSEMOVE,MK_LBUTTON,v?MAKELPARAM(5,start+80):MAKELPARAM(start+80,5));
        SendMessageW(bar,WM_LBUTTONUP,0,v?MAKELPARAM(5,start+80):MAKELPARAM(start+80,5));
        CHECK(notice_count>=3 && notices[notice_count-2]==SB_THUMBPOSITION &&
            notices[notice_count-1]==SB_ENDSCROLL && full_track>65535 && GetCapture()!=bar);
        EnableWindow(bar,FALSE);
        clear(dc);
        SendMessageW(bar,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
        CHECK(v?groove(dc,4,90):groove(dc,90,4));
        CHECK(remove(bar) && !attached(bar) && DestroyWindow(bar));
    }
    HWND list=CreateWindowExW(WS_EX_CLIENTEDGE,L"LISTBOX",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_DISABLENOSCROLL,
        8,8,170,130,root,NULL,module,NULL);
    CHECK(list && attach(list));
    for(int row=0;row<120;++row)CHECK(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"row")!=LB_ERR);
    CHECK(SendMessageW(list,LB_SETTOPINDEX,60,0)==0);
    CHECK(window_bar(list,dc,OBJID_VSCROLL));
    CHECK(SendMessageW(list,LB_SETTOPINDEX,0,0)==0);
    SendMessageW(list,WM_MOUSEWHEEL,MAKEWPARAM(0,-WHEEL_DELTA),0);
    CHECK(SendMessageW(list,LB_GETTOPINDEX,0,0)>0);
    HWND edit=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE,
        8,8,170,130,root,NULL,module,NULL);
    CHECK(edit && (!hooked || attached(edit)) && attach(edit));
    WCHAR lines[321];
    for(int i=0;i<80;++i){
        lines[4*i]=L'x';lines[4*i+1]=L'x';lines[4*i+2]=L'\r';lines[4*i+3]=L'\n';
    }
    lines[320]=0;
    CHECK(SetWindowTextW(edit,lines));
    CHECK(SendMessageW(edit,EM_LINESCROLL,0,35));
    CHECK(window_bar(edit,dc,OBJID_VSCROLL));
    CHECK(remove(edit) && !attached(edit));
    HWND grip=CreateWindowW(L"SCROLLBAR",L"",WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP,0,0,17,17,root,NULL,module,NULL);
    CHECK(grip && attach(grip));
    clear(dc);
    SendMessageW(grip,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    unsigned shadow=0;
    for(int y=5;y<17;++y)for(int x=5;x<17;++x)shadow+=GetPixel(dc,x,y)==RGB(128,128,128);
    CHECK(shadow>0 && GetPixel(dc,0,0)==RGB(212,208,200));
    CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc) && DestroyWindow(root));
    printf("WIN2K_SCROLL_OK bits=%u aware=%d hooked=%d axes=2 native_frame=1 native_list=1 native_edit=1 custom_track=1 thumb_32bit=1 sizegrip=1\n",
        (unsigned)sizeof(void *)*8,aware,hooked);
    return 0;
}
