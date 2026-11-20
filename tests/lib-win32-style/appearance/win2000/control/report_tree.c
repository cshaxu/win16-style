#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_REPORT_TREE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static HWND list,tree;
static BOOL custom;
static unsigned custom_items,list_items,tree_items,expand_notices;
static const COLORREF face=RGB(212,208,200),marker=RGB(62,25,148);

static LRESULT CALLBACK parent_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_NOTIFY && b){
        NMHDR *notice=(NMHDR *)b;
        if(notice->hwndFrom==tree && (notice->code==TVN_ITEMEXPANDEDW || notice->code==TVN_ITEMEXPANDEDA))
            ++expand_notices;
        if((notice->hwndFrom==list || notice->hwndFrom==tree) && notice->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *draw=(NMCUSTOMDRAW *)b;
            if(draw->dwDrawStage==CDDS_PREPAINT && custom)return CDRF_NOTIFYITEMDRAW;
            if(draw->dwDrawStage==CDDS_ITEMPREPAINT){
                ++custom_items;
                if(custom){
                    if(notice->hwndFrom==list){
                        ++list_items;
                        RECT r={.left=LVIR_BOUNDS};
                        if(!SendMessageW(list,LVM_GETITEMRECT,draw->dwItemSpec,(LPARAM)&r))return CDRF_DODEFAULT;
                        SetDCBrushColor(draw->hdc,marker);
                        FillRect(draw->hdc,&r,GetStockObject(DC_BRUSH));
                        return CDRF_SKIPDEFAULT;
                    }else{
                        NMTVCUSTOMDRAW *item=(NMTVCUSTOMDRAW *)b;
                        item->clrTextBk=marker;item->clrText=RGB(255,255,255);++tree_items;
                    }
                }
            }
        }
    }
    return DefWindowProcW(window,message,a,b);
}

static unsigned client_pixels(HWND window,HDC dc,const RECT *area,COLORREF color)
{
    RECT client;
    if(!GetClientRect(window,&client))return 0;
    int saved=SaveDC(dc);
    if(!saved)return 0;
    SetDCBrushColor(dc,RGB(232,51,113));
    FillRect(dc,&client,GetStockObject(DC_BRUSH));
    SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
    unsigned count=0;
    RECT region;
    if(IntersectRect(&region,&client,area)){
        for(LONG y=region.top;y<region.bottom;++y)
            for(LONG x=region.left;x<region.right;++x)
                if(GetPixel(dc,x,y)==color)++count;
    }
    RestoreDC(dc,saved);
    return count;
}

int wmain(int argc,WCHAR **argv)
{
    BOOL themed=argc==4 && !wcscmp(argv[1],L"--theme-colors");
    CHECK(themed || argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_LISTVIEW_CLASSES|ICC_TREEVIEW_CLASSES};
    CHECK(InitCommonControlsEx(&icc));
    HMODULE dll=NULL;
    window_op attach=NULL,attached=NULL,remove=NULL;
    COLORREF list_face=face,tree_face=face;
    if(themed){
        WCHAR *end;
        unsigned long value=wcstoul(argv[2],&end,16);
        CHECK(end!=argv[2] && !*end && value<=0xffffff);
        list_face=(COLORREF)value;
        value=wcstoul(argv[3],&end,16);
        CHECK(end!=argv[3] && !*end && value<=0xffffff);
        tree_face=(COLORREF)value;
    }else{
        dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        CHECK(dll);
        attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
        attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
        remove=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
        CHECK(attach && attached && remove);
    }
    HINSTANCE module=GetModuleHandleW(NULL);
    const WNDCLASSW wc={.lpszClassName=L"OwnedWin2kReportTree",.lpfnWndProc=parent_proc,.hInstance=module};
    CHECK(RegisterClassW(&wc));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,wc.lpszClassName,L"",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        -10000,-10000,390,360,NULL,NULL,module,NULL);
    CHECK(root);
    list=CreateWindowW(WC_LISTVIEWW,NULL,WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS,0,0,280,125,root,NULL,module,NULL);
    tree=CreateWindowW(WC_TREEVIEWW,NULL,WS_CHILD|WS_VISIBLE|TVS_HASLINES|TVS_HASBUTTONS|TVS_LINESATROOT|TVS_SHOWSELALWAYS,
        0,130,280,130,root,NULL,module,NULL);
    CHECK(list && tree && (!hooked || (attached(list) && attached(tree))));
    CHECK(themed || (attach(list) && attach(tree)));
    LVCOLUMNW column={.mask=LVCF_TEXT|LVCF_WIDTH,.cx=170,.pszText=L"Name"};
    LVITEMW row={.mask=LVIF_TEXT,.pszText=L"Report item"};
    LVITEMW chosen={.stateMask=LVIS_SELECTED,.state=LVIS_SELECTED};
    CHECK(SendMessageW(list,LVM_INSERTCOLUMNW,0,(LPARAM)&column)==0);
    HWND header=ListView_GetHeader(list);
    CHECK(header && (themed || attach(header)));
    CHECK(SendMessageW(list,LVM_SETCOLUMNWIDTH,0,150));
    HDITEMW header_item={.mask=HDI_WIDTH};
    CHECK(SendMessageW(header,HDM_GETITEMW,0,(LPARAM)&header_item) && header_item.cxy==150);
    CHECK(SendMessageW(list,LVM_INSERTITEMW,0,(LPARAM)&row)==0);
    CHECK(SendMessageW(list,LVM_SETITEMSTATE,0,(LPARAM)&chosen));
    TVINSERTSTRUCTW branch={.hParent=TVI_ROOT,.hInsertAfter=TVI_LAST,.item={.mask=TVIF_TEXT,.pszText=L"Tree item"}};
    HTREEITEM node=(HTREEITEM)SendMessageW(tree,TVM_INSERTITEMW,0,(LPARAM)&branch);
    branch.hParent=node;branch.item.pszText=L"Child";
    HTREEITEM child=(HTREEITEM)SendMessageW(tree,TVM_INSERTITEMW,0,(LPARAM)&branch);
    CHECK(node && child && SendMessageW(tree,TVM_SELECTITEM,TVGN_CARET,(LPARAM)node));
    CHECK(SendMessageW(tree,TVM_EXPAND,TVE_EXPAND,(LPARAM)node) && expand_notices==1);
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),300,-160,1,32,BI_RGB}};
    void *pixels;
    HBITMAP bmp=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bmp);
    HGDIOBJ old=SelectObject(dc,bmp);
    CHECK(old && old!=HGDI_ERROR);
    RECT row_area={.left=LVIR_LABEL},tree_area;
    CHECK(SendMessageW(list,LVM_GETITEMRECT,0,(LPARAM)&row_area));
    CHECK(TreeView_GetItemRect(tree,node,&tree_area,TRUE));
    unsigned baseline=custom_items;
    SetTextColor(dc,RGB(16,40,81));
    unsigned report_face=client_pixels(list,dc,&row_area,list_face);
    CHECK(report_face>10);
    CHECK(client_pixels(tree,dc,&tree_area,tree_face)>10);
    CHECK(custom_items==baseline && GetTextColor(dc)==RGB(16,40,81));
    RECT indent={0,tree_area.top,tree_area.left-1,tree_area.bottom};
    COLORREF stroke=GetSysColor(COLOR_WINDOWTEXT);
    unsigned minus=client_pixels(tree,dc,&indent,stroke);
    CHECK(SendMessageW(tree,TVM_EXPAND,TVE_COLLAPSE,(LPARAM)node));
    CHECK(!(TreeView_GetItemState(tree,node,TVIS_EXPANDED)&TVIS_EXPANDED) && expand_notices==1);
    CHECK(minus>0 && client_pixels(tree,dc,&indent,stroke)>minus);
    CHECK(SendMessageW(tree,TVM_EXPAND,TVE_EXPAND,(LPARAM)node));
    CHECK((TreeView_GetItemState(tree,node,TVIS_EXPANDED)&TVIS_EXPANDED) && expand_notices==1);
    SetRect(&row_area,0,0,280,125);
    const UINT view_styles[]={LVS_ICON,LVS_SMALLICON,LVS_LIST,LVS_REPORT};
    for(unsigned i=0;i<ARRAYSIZE(view_styles);++i){
        LONG_PTR style=GetWindowLongPtrW(list,GWL_STYLE);
        SetWindowLongPtrW(list,GWL_STYLE,(style&~LVS_TYPEMASK)|view_styles[i]);
        CHECK((GetWindowLongPtrW(list,GWL_STYLE)&LVS_TYPEMASK)==view_styles[i]);
        CHECK(SendMessageW(list,LVM_GETITEMSTATE,0,LVIS_SELECTED)==LVIS_SELECTED);
        RECT position={.left=LVIR_BOUNDS};
        CHECK(SendMessageW(list,LVM_GETITEMRECT,0,(LPARAM)&position) && !IsRectEmpty(&position));
        CHECK(client_pixels(list,dc,&position,list_face)>10);
        CHECK(SendMessageW(list,LVM_GETITEMSTATE,0,LVIS_SELECTED)==LVIS_SELECTED);
    }
    custom=TRUE;
    CHECK(client_pixels(list,dc,&row_area,marker)>10);
    CHECK(client_pixels(tree,dc,&tree_area,marker)>10);
    CHECK(list_items && tree_items);
    if(themed){
        CHECK(DestroyWindow(root));
        SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);
        printf("THEME_REPORT_TREE_OK list=%06lx tree=%06lx views=4 app_custom=2\n",list_face,tree_face);
        return 0;
    }
    HIMAGELIST images=ImageList_Create(16,16,ILC_COLOR32|ILC_MASK,1,1);
    CHECK(images && ImageList_AddIcon(images,LoadIconW(NULL,IDI_APPLICATION))==0);
    CHECK(!SendMessageW(list,LVM_SETIMAGELIST,LVSIL_SMALL,(LPARAM)images));
    LVITEMW own_image={.mask=LVIF_IMAGE,.iImage=0};
    CHECK(SendMessageW(list,LVM_SETITEMW,0,(LPARAM)&own_image));
    CHECK(remove(header) && remove(list) && remove(tree) && !attached(list) && !attached(tree));
    CHECK(client_pixels(list,dc,&row_area,marker)>10 && client_pixels(tree,dc,&tree_area,marker)>10);
    CHECK((HIMAGELIST)SendMessageW(list,LVM_GETIMAGELIST,LVSIL_SMALL,0)==images);
    CHECK((HIMAGELIST)SendMessageW(list,LVM_SETIMAGELIST,LVSIL_SMALL,0)==images && ImageList_Destroy(images));
    CHECK((HTREEITEM)SendMessageW(tree,TVM_GETNEXTITEM,TVGN_CARET,0)==node);
    HWND virtual_list=CreateWindowW(WC_LISTVIEWW,NULL,WS_CHILD|LVS_REPORT|LVS_OWNERDATA,0,0,100,70,root,NULL,module,NULL);
    CHECK(virtual_list && !attached(virtual_list) && !attach(virtual_list) && GetLastError()==ERROR_NOT_SUPPORTED);
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);FreeLibrary(dll);
    printf("WIN2K_REPORT_TREE_OK views=4 tree_glyphs=2 report_face=%u app_custom=2 detach=3\n",report_face);
    return 0;
}
