#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
typedef struct ops {window_op attach,attached,remove;BOOL started;} ops;
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_RUNTIME_ROLES_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return FALSE;}}while(0)

static BOOL alias(const WCHAR *base,const WCHAR *name)
{
    WNDCLASSEXW cls={.cbSize=sizeof(cls)};
    if(!GetClassInfoExW(NULL,base,&cls))return FALSE;
    cls.hInstance=GetModuleHandleW(NULL);
    cls.lpszClassName=name;
    cls.lpszMenuName=NULL;
    cls.hIcon=cls.hIconSm=NULL;
    return RegisterClassExW(&cls)!=0;
}

static HWND child(HWND parent,LPCWSTR name,DWORD style,DWORD ex,int width,int height)
{
    return CreateWindowExW(ex,name,L"Example",WS_CHILD|WS_VISIBLE|style,0,0,width,height,parent,NULL,
        GetModuleHandleW(NULL),NULL);
}

static void print(HWND window,HDC dc,UINT message,LPARAM flags)
{
    const RECT r={0,0,500,240};
    COLORREF old=SetDCBrushColor(dc,RGB(123,41,197));
    FillRect(dc,&r,GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc,old);
    SendMessageW(window,message,(WPARAM)dc,flags);
}

static BOOL attached(HWND window,const ops *api)
{return window && (!api->started || api->attached(window)) && api->attach(window) && api->attached(window);}

static BOOL buttons(HWND root,HDC dc,const ops *api)
{
    HWND button=child(root,L"TButton",BS_PUSHBUTTON,0,150,36);
    CHECK(attached(button,api));
    HFONT font=(HFONT)GetStockObject(ANSI_VAR_FONT);
    SendMessageW(button,WM_SETFONT,(WPARAM)font,FALSE);
    print(button,dc,WM_PRINTCLIENT,PRF_CLIENT);
    CHECK(GetPixel(dc,4,4)==RGB(212,208,200));
    SendMessageW(button,BM_SETSTATE,TRUE,0);
    CHECK(SendMessageW(button,BM_GETSTATE,0,0)&BST_PUSHED);
    CHECK(api->remove(button) && !api->attached(button));
    CHECK((HFONT)SendMessageW(button,WM_GETFONT,0,0)==font);
    CHECK(api->attach(button) && api->remove(button) && DestroyWindow(button));
    HWND owner=child(root,L"TButton",BS_OWNERDRAW,0,150,36);
    CHECK(owner && !api->attached(owner) && !api->attach(owner) && DestroyWindow(owner));
    return TRUE;
}

static BOOL edit(HWND root,HDC dc,const ops *api)
{
    HWND window=child(root,L"TEdit",ES_AUTOHSCROLL,WS_EX_CLIENTEDGE,190,35);
    CHECK(attached(window,api));
    CHECK(SetWindowTextW(window,L"editable"));
    SendMessageW(window,EM_SETSEL,1,4);
    CHECK(SendMessageW(window,EM_GETSEL,0,0)==MAKELONG(1,4));
    print(window,dc,WM_PRINT,PRF_NONCLIENT);
    CHECK(GetPixel(dc,0,3)==RGB(128,128,128) && GetPixel(dc,1,3)==RGB(64,64,64));
    CHECK(api->remove(window) && !api->attached(window) && SendMessageW(window,EM_GETSEL,0,0)==MAKELONG(1,4));
    CHECK(DestroyWindow(window));
    return TRUE;
}

static BOOL common_controls(HWND root,HDC dc,const ops *api)
{
    HWND tab=child(root,L"TPageControl",WS_TABSTOP,0,260,130);
    CHECK(attached(tab,api));
    TCITEMW page={.mask=TCIF_TEXT,.pszText=L"Page"};
    CHECK(SendMessageW(tab,TCM_INSERTITEMW,0,(LPARAM)&page)==0);
    RECT item;
    CHECK(SendMessageW(tab,TCM_GETITEMRECT,0,(LPARAM)&item));
    print(tab,dc,WM_PRINTCLIENT,PRF_CLIENT);
    CHECK(GetPixel(dc,(item.left+item.right)/2,item.top+3)==RGB(212,208,200));
    CHECK(api->remove(tab) && SendMessageW(tab,TCM_GETITEMCOUNT,0,0)==1 && DestroyWindow(tab));
    HWND tree=child(root,L"TTreeView",TVS_HASBUTTONS,WS_EX_CLIENTEDGE,220,115);
    CHECK(attached(tree,api));
    TVINSERTSTRUCTW insert={.hParent=TVI_ROOT,.hInsertAfter=TVI_LAST,
        .item={.mask=TVIF_TEXT,.pszText=L"Root node"}};
    HTREEITEM node=(HTREEITEM)SendMessageW(tree,TVM_INSERTITEMW,0,(LPARAM)&insert);
    CHECK(node && SendMessageW(tree,TVM_SELECTITEM,TVGN_CARET,(LPARAM)node));
    print(tree,dc,WM_PRINT,PRF_NONCLIENT);
    CHECK(GetPixel(dc,0,3)==RGB(128,128,128));
    CHECK(api->remove(tree) && SendMessageW(tree,TVM_GETNEXTITEM,TVGN_CARET,0)==(LRESULT)node);
    CHECK(DestroyWindow(tree));
    return TRUE;
}

static BOOL run(int argc,WCHAR **argv)
{
    CHECK(argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),started=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_TAB_CLASSES|ICC_TREEVIEW_CLASSES};
    CHECK(InitCommonControlsEx(&icc));
    HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    CHECK(dll);
    const ops api={(void *)GetProcAddress(dll,"Ntw2ksWindowAttach"),(void *)GetProcAddress(dll,"Ntw2ksWindowAttached"),
        (void *)GetProcAddress(dll,"Ntw2ksWindowRemove"),started};
    CHECK(api.attach && api.attached && api.remove);
    CHECK(alias(L"BUTTON",L"TButton") && alias(L"EDIT",L"TEdit"));
    CHECK(alias(WC_TABCONTROLW,L"TPageControl") && alias(WC_TREEVIEWW,L"TTreeView"));
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WNDCLASSW impostor={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"ThunderRT6CommandButton"};
    CHECK(RegisterClassW(&impostor));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",NULL,
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,-10000,-10000,550,275,NULL,NULL,instance,NULL);
    CHECK(root);
    HWND fake=child(root,impostor.lpszClassName,BS_PUSHBUTTON,0,150,36);
    CHECK(fake && !api.attached(fake) && !api.attach(fake) && GetLastError()==ERROR_NOT_SUPPORTED && DestroyWindow(fake));
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),500,-240,1,32,BI_RGB}};
    void *bits;
    HBITMAP dib=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    CHECK(dc && dib);
    HGDIOBJ old=SelectObject(dc,dib);
    CHECK(old && old!=HGDI_ERROR);
    CHECK(buttons(root,dc,&api));
    CHECK(edit(root,dc,&api));
    CHECK(common_controls(root,dc,&api));
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);
    DeleteObject(dib);
    DeleteDC(dc);
    FreeLibrary(dll);
    puts("WIN2K_RUNTIME_ROLES_OK models=4 rejected=2");
    return TRUE;
}

int wmain(int argc,WCHAR **argv)
{return run(argc,argv)?0:1;}
