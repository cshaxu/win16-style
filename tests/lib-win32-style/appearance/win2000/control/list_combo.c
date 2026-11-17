#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
static const COLORREF sentinel=RGB(211,31,167), selection=RGB(10,36,106);
#define CHECK(expr) do { if(!(expr)){fprintf(stderr,"WIN2K_LIST_COMBO_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#expr);return 1;} } while(0)

static HBRUSH background;
static unsigned owner_draws;
static LRESULT CALLBACK parent_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_CTLCOLORLISTBOX && background)return (LRESULT)background;
    if(message==WM_DRAWITEM){
        const DRAWITEMSTRUCT *draw=(const DRAWITEMSTRUCT *)lp;
        ++owner_draws;
        FillRect(draw->hDC,&draw->rcItem,GetStockObject(BLACK_BRUSH));
        return TRUE;
    }
    return DefWindowProcW(window,message,wp,lp);
}

static void capture(HWND window,HDC dc,UINT message,LPARAM flags)
{
    RECT bounds={0,0,240,180};
    SetDCBrushColor(dc,sentinel);
    FillRect(dc,&bounds,GetStockObject(DC_BRUSH));
    SendMessageW(window,message,(WPARAM)dc,flags);
}

int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    INITCOMMONCONTROLSEX common={sizeof(common),ICC_WIN95_CLASSES};
    CHECK(InitCommonControlsEx(&common));
    HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    CHECK(dll);
    window_op attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
    window_op attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
    window_op remove_style=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
    CHECK(attach && attached && remove_style);
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=module,.lpszClassName=L"OwnedWin2kListCombo"};
    CHECK(RegisterClassW(&cls));
    HWND parent=CreateWindowExW(WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,cls.lpszClassName,L"",
        WS_POPUP|WS_VISIBLE,-10000,-10000,300,220,NULL,NULL,module,NULL);
    CHECK(parent);
    HDC dc=CreateCompatibleDC(NULL);
    void *pixels;
    BITMAPINFO bitmap_info={.bmiHeader={sizeof(BITMAPINFOHEADER),240,-180,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&bitmap_info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);
    CHECK(old && old!=HGDI_ERROR);
    const DWORD list_modes[]={0,LBS_MULTIPLESEL,LBS_EXTENDEDSEL,LBS_MULTICOLUMN,LBS_USETABSTOPS};
    for(unsigned i=0;i<ARRAYSIZE(list_modes);++i){
        HWND list=CreateWindowExW(WS_EX_CLIENTEDGE,L"LISTBOX",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|list_modes[i],
            0,0,170,85,parent,(HMENU)101,module,NULL);
        CHECK(list && (!hooked || attached(list)));
        if(i==4 && attached(list))CHECK(remove_style(list));
        CHECK(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"Row\tTail")==0);
        CHECK(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"Second")==1);
        if(i==4){
            int tab=31;
            CHECK(SendMessageW(list,LB_SETTABSTOPS,1,(LPARAM)&tab));
        }
        if(i==1 || i==2)CHECK(SendMessageW(list,LB_SETSEL,TRUE,0)!=LB_ERR);
        else CHECK(SendMessageW(list,LB_SETCURSEL,0,0)==0);
        RECT before,after,row;
        CHECK(GetClientRect(list,&before));
        COLORREF native_tab[145];
        if(i==4){
            capture(list,dc,WM_PRINTCLIENT,PRF_CLIENT);
            for(int x=0;x<145;++x)native_tab[x]=GetPixel(dc,x,5);
        }
        CHECK(attach(list));
        CHECK(SendMessageW(list,LB_GETITEMRECT,0,(LPARAM)&row)!=LB_ERR);
        capture(list,dc,WM_PRINTCLIENT,PRF_CLIENT);
        if(i<4)CHECK(GetPixel(dc,row.right-3,row.top+3)==selection);
        else for(int x=0;x<145;++x)CHECK(GetPixel(dc,x,5)==native_tab[x]);
        if(i==0){
            background=CreateSolidBrush(RGB(12,34,56));
            CHECK(background);
            capture(list,dc,WM_PRINTCLIENT,PRF_CLIENT);
            CHECK(GetPixel(dc,130,65)==RGB(12,34,56));
            CHECK(DeleteObject(background));
            background=NULL;
        }
        capture(list,dc,WM_PRINT,PRF_CLIENT|PRF_NONCLIENT);
        CHECK(GetPixel(dc,0,9)==RGB(128,128,128));
        CHECK(remove_style(list) && !attached(list) && GetClientRect(list,&after) && EqualRect(&before,&after));
        CHECK(SendMessageW(list,LB_GETCOUNT,0,0)==2 && DestroyWindow(list));
    }
    const DWORD combo_modes[]={CBS_SIMPLE,CBS_DROPDOWN,CBS_DROPDOWNLIST};
    for(unsigned i=0;i<ARRAYSIZE(combo_modes);++i){
        HWND combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|combo_modes[i],
            0,0,170,105,parent,(HMENU)102,module,NULL);
        CHECK(combo && (!hooked || attached(combo)));
        CHECK(attach(combo));
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        CHECK(GetComboBoxInfo(combo,&info) && info.hwndList && attached(info.hwndList));
        CHECK(SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)L"Alpha")==0);
        CHECK(SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)L"Beta")==1);
        CHECK(SendMessageW(combo,CB_SETCURSEL,1,0)==1);
        RECT selected;
        CHECK(SendMessageW(info.hwndList,LB_GETITEMRECT,1,(LPARAM)&selected)!=LB_ERR);
        capture(info.hwndList,dc,WM_PRINTCLIENT,PRF_CLIENT);
        CHECK(GetPixel(dc,100,selected.top+3)==selection);
        if(i!=2){
            WCHAR text[16];
            CHECK(info.hwndItem && GetWindowTextW(info.hwndItem,text,ARRAYSIZE(text))==4 && !wcscmp(text,L"Beta"));
        }
        capture(combo,dc,WM_PRINTCLIENT,PRF_CLIENT);
        CHECK(GetPixel(dc,0,8)==RGB(128,128,128) && GetPixel(dc,1,8)==RGB(64,64,64));
        if(i!=2)CHECK(GetPixel(dc,info.rcItem.left+3,info.rcItem.top+3)==sentinel);
        if(i){
            unsigned black=0;
            for(int y=info.rcButton.top;y<info.rcButton.bottom;++y)
                for(int x=info.rcButton.left;x<info.rcButton.right;++x)black+=GetPixel(dc,x,y)==RGB(0,0,0);
            CHECK(black>0);
            SendMessageW(combo,CB_SHOWDROPDOWN,TRUE,0);
            CHECK(SendMessageW(combo,CB_GETDROPPEDSTATE,0,0));
            SendMessageW(combo,CB_SHOWDROPDOWN,FALSE,0);
            CHECK(!SendMessageW(combo,CB_GETDROPPEDSTATE,0,0));
            EnableWindow(combo,FALSE);
            capture(combo,dc,WM_PRINTCLIENT,PRF_CLIENT);
            CHECK(GetPixel(dc,info.rcButton.left+2,info.rcButton.top+2)==RGB(212,208,200));
        }
        CHECK(remove_style(combo) && !attached(combo) && IsWindow(info.hwndList) && !attached(info.hwndList));
        CHECK(SendMessageW(combo,CB_GETCURSEL,0,0)==1 && DestroyWindow(combo));
    }
    HWND owner=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|WS_VISIBLE|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS,
        0,0,170,80,parent,(HMENU)103,module,NULL);
    CHECK(owner && SendMessageW(owner,LB_ADDSTRING,0,(LPARAM)L"Owner")==0 && attach(owner));
    owner_draws=0;
    capture(owner,dc,WM_PRINTCLIENT,PRF_CLIENT);
    CHECK(owner_draws && GetPixel(dc,100,4)==RGB(0,0,0) && DestroyWindow(owner));
    CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc) && DestroyWindow(parent));
    printf("WIN2K_LIST_COMBO_OK bits=%u aware=%d hooked=%d list_modes=5 combo_modes=3 child_retire=3 native_tabs=1 owner_draw=1\n",
        (unsigned)sizeof(void *)*8,aware,hooked);
    return 0;
}
