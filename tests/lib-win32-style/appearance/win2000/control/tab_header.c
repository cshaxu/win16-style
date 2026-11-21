#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef BOOL (__cdecl *window_op)(HWND);
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_TAB_HEADER_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static const COLORREF marker=RGB(53,17,111),face=RGB(212,208,200),dark=RGB(64,64,64);
static HWND header_control;
static BOOL custom_header;
static unsigned header_items,header_clicks,tab_changes;
static LRESULT CALLBACK parent_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_NOTIFY && b){
        const NMHDR *notice=(const NMHDR *)b;
        if(notice->code==TCN_SELCHANGE)++tab_changes;
        if(notice->hwndFrom==header_control){
            if(notice->code==HDN_ITEMCLICKA || notice->code==HDN_ITEMCLICKW)++header_clicks;
            if(notice->code==NM_CUSTOMDRAW){
                const NMCUSTOMDRAW *draw=(const NMCUSTOMDRAW *)b;
                if(draw->dwDrawStage==CDDS_PREPAINT && custom_header)return CDRF_NOTIFYITEMDRAW;
                if(draw->dwDrawStage==CDDS_ITEMPREPAINT){
                    ++header_items;
                    if(custom_header){
                        SetDCBrushColor(draw->hdc,marker);
                        FillRect(draw->hdc,&draw->rc,GetStockObject(DC_BRUSH));
                        return CDRF_SKIPDEFAULT;
                    }
                }
            }
        }
    }
    return DefWindowProcW(window,message,a,b);
}
static void print_client(HWND window,HDC dc)
{
    RECT canvas={0,0,380,230};
    SetDCBrushColor(dc,RGB(199,55,141));
    FillRect(dc,&canvas,GetStockObject(DC_BRUSH));
    SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
}

int wmain(int argc,WCHAR **argv)
{
    BOOL fingerprints=argc==2 && !wcscmp(argv[1],L"--control-fingerprints");
    BOOL themed_header=argc==3 && !wcscmp(argv[1],L"--header-edge");
    BOOL themed_tab=argc==4 && !wcscmp(argv[1],L"--tab-edge");
    BOOL themed=themed_header || themed_tab || fingerprints;
    CHECK(themed || argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    const INITCOMMONCONTROLSEX classes={sizeof(classes),ICC_TAB_CLASSES|ICC_LISTVIEW_CLASSES|ICC_BAR_CLASSES};
    CHECK(InitCommonControlsEx(&classes));
    HMODULE dll=NULL;
    window_op attach=NULL,attached=NULL,remove=NULL;
    if(!themed){
        dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        CHECK(dll);
        attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");
        attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");
        remove=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");
        CHECK(attach && attached && remove);
    }
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=module,.lpszClassName=L"OwnedWin2kTabHeader"};
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,cls.lpszClassName,L"Test",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        -10000,-10000,420,350,NULL,NULL,module,NULL);
    CHECK(root);
    HDC dc=CreateCompatibleDC(NULL);
    void *pixels;
    BITMAPINFO dib={.bmiHeader={sizeof(BITMAPINFOHEADER),380,-230,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&dib,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);
    CHECK(old && old!=HGDI_ERROR);

    if(fingerprints){
        const WCHAR *types[]={WC_TABCONTROLW,WC_HEADERW,TRACKBAR_CLASSW};
        const DWORD styles[]={0,HDS_BUTTONS,TBS_AUTOTICKS};
        const int heights[]={120,24,36};
        for(unsigned i=0;i<ARRAYSIZE(types);++i){
            HWND control=CreateWindowW(types[i],NULL,WS_CHILD|WS_VISIBLE|styles[i],
                0,0,250,heights[i],root,NULL,module,NULL);
            CHECK(control);
            if(i==0){
                TCITEMW item={.mask=TCIF_TEXT,.pszText=L"Tab"};
                CHECK(SendMessageW(control,TCM_INSERTITEMW,0,(LPARAM)&item)==0);
            }else if(i==1){
                HDITEMW item={.mask=HDI_WIDTH|HDI_FORMAT|HDI_TEXT,.cxy=90,.fmt=HDF_STRING,.pszText=L"Name"};
                CHECK(SendMessageW(control,HDM_INSERTITEMW,0,(LPARAM)&item)==0);
            }else{
                SendMessageW(control,TBM_SETRANGE,FALSE,MAKELPARAM(0,100));
                SendMessageW(control,TBM_SETPOS,FALSE,40);
                CHECK(SendMessageW(control,TBM_GETPOS,0,0)==40);
            }
            print_client(control,dc);
            unsigned long hash=2166136261u;
            for(int y=0;y<heights[i];++y)
                for(int x=0;x<250;++x)hash=(hash^GetPixel(dc,x,y))*16777619u;
            printf("CONTROL_THEME type=%u pixels=%08lx\n",i,hash);
            CHECK(DestroyWindow(control));
        }
        CHECK(DestroyWindow(root) && SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc));
        return 0;
    }

    if(themed_header){
        WCHAR *end;
        unsigned long value=wcstoul(argv[2],&end,16);
        CHECK(end!=argv[2] && !*end && value<=0xffffff);
        header_control=CreateWindowW(WC_HEADERW,NULL,WS_CHILD|WS_VISIBLE|HDS_BUTTONS,
            0,0,250,24,root,NULL,module,NULL);
        CHECK(header_control);
        HDITEMW column={.mask=HDI_WIDTH|HDI_FORMAT|HDI_TEXT,.cxy=90,.fmt=HDF_STRING,.pszText=L"Name"};
        CHECK(SendMessageW(header_control,HDM_INSERTITEMW,0,(LPARAM)&column)==0);
        print_client(header_control,dc);
        COLORREF edge=GetPixel(dc,4,0);
        printf("THEME_HEADER_EDGE pixel=%06lx\n",edge);
        CHECK(edge==(COLORREF)value);
        SendMessageW(header_control,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(25,9));
        SendMessageW(header_control,WM_LBUTTONUP,0,MAKELPARAM(25,9));
        CHECK(header_clicks==1 && DestroyWindow(root));
        SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
        puts("THEME_HEADER_OK click=1");
        return 0;
    }

    HWND tab=CreateWindowW(WC_TABCONTROLW,NULL,WS_CHILD|WS_VISIBLE|TCS_FIXEDWIDTH|TCS_MULTILINE,
        0,30,230,190,root,NULL,module,NULL);
    if(themed_tab){
        WCHAR *end;
        unsigned long value=wcstoul(argv[2],&end,16);
        CHECK(tab && end!=argv[2] && !*end && value<=0xffffff);
        unsigned long offset=wcstoul(argv[3],&end,10);
        CHECK(end!=argv[3] && !*end && (offset==1 || offset==2));
        SendMessageW(tab,TCM_SETITEMSIZE,0,MAKELPARAM(70,24));
        TCITEMW first_item={.mask=TCIF_TEXT,.pszText=L"General"};
        TCITEMW second_item={.mask=TCIF_TEXT,.pszText=L"Details"};
        CHECK(SendMessageW(tab,TCM_INSERTITEMW,0,(LPARAM)&first_item)==0 &&
            SendMessageW(tab,TCM_INSERTITEMW,1,(LPARAM)&second_item)==1);
        RECT first,second;
        CHECK(SendMessageW(tab,TCM_GETITEMRECT,0,(LPARAM)&first) &&
            SendMessageW(tab,TCM_GETITEMRECT,1,(LPARAM)&second));
        print_client(tab,dc);
        COLORREF edge=GetPixel(dc,(first.left+first.right)/2,first.top-(int)offset);
        printf("THEME_TAB_EDGE pixel=%06lx offset=%lu\n",edge,offset);
        CHECK(edge==(COLORREF)value && GetPixel(dc,120,120)==RGB(199,55,141));
        LPARAM point=MAKELPARAM((second.left+second.right)/2,(second.top+second.bottom)/2);
        SendMessageW(tab,WM_LBUTTONDOWN,MK_LBUTTON,point);
        SendMessageW(tab,WM_LBUTTONUP,0,point);
        CHECK(SendMessageW(tab,TCM_GETCURSEL,0,0)==1 && tab_changes==1 && DestroyWindow(root));
        SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
        puts("THEME_TAB_OK selection=1 client_clip=1");
        return 0;
    }
    CHECK(tab && (!hooked || attached(tab)) && attach(tab));
    HFONT font=GetStockObject(ANSI_VAR_FONT),borrowed=GetStockObject(ANSI_FIXED_FONT);
    SendMessageW(tab,WM_SETFONT,(WPARAM)font,FALSE);
    CHECK(SelectObject(dc,borrowed));
    SendMessageW(tab,TCM_SETITEMSIZE,0,MAKELPARAM(70,24));
    TCITEMW item={.mask=TCIF_TEXT,.pszText=L"&General"};
    CHECK(SendMessageW(tab,TCM_INSERTITEMW,0,(LPARAM)&item)==0);
    RECT icon={0,0,8,8};
    SetDCBrushColor(dc,marker);
    CHECK(FillRect(dc,&icon,GetStockObject(DC_BRUSH)));
    HBITMAP glyph=CreateCompatibleBitmap(dc,8,8);
    HDC icon_dc=CreateCompatibleDC(dc);
    CHECK(glyph && icon_dc);
    HGDIOBJ old_icon=SelectObject(icon_dc,glyph);
    CHECK(old_icon && BitBlt(icon_dc,0,0,8,8,dc,0,0,SRCCOPY));
    CHECK(SelectObject(icon_dc,old_icon) && DeleteDC(icon_dc));
    HIMAGELIST images=ImageList_Create(8,8,ILC_COLOR32,1,1);
    CHECK(images && ImageList_Add(images,glyph,NULL)==0 && DeleteObject(glyph));
    SendMessageW(tab,TCM_SETIMAGELIST,0,(LPARAM)images);
    item.mask=TCIF_TEXT|TCIF_IMAGE;item.iImage=0;item.pszText=L"Details";
    CHECK(SendMessageW(tab,TCM_INSERTITEMW,1,(LPARAM)&item)==1);
    RECT first,second;
    CHECK(SendMessageW(tab,TCM_GETITEMRECT,0,(LPARAM)&first) && SendMessageW(tab,TCM_GETITEMRECT,1,(LPARAM)&second));
    int x=(first.left+first.right)/2;
    print_client(tab,dc);
    CHECK(GetPixel(dc,x,first.top-2)==RGB(255,255,255) && GetPixel(dc,x,first.top-1)==face);
    /* Original Display Properties selected top corners at x156..158
     * and x228..229, y29..31 (same roles in original Task Manager). */
    CHECK(GetPixel(dc,first.left-2,first.top-2)==face);
    CHECK(GetPixel(dc,first.left-1,first.top-1)==RGB(255,255,255));
    CHECK(GetPixel(dc,first.right,first.top-1)==dark && GetPixel(dc,first.right+1,first.top)==dark);
    CHECK(GetPixel(dc,second.right-1,second.top+4)==dark);
    BOOL icon_drawn=FALSE;
    for(int y=second.top+3;y<second.bottom-2;++y)for(int column=second.left+3;column<second.right-2;++column)
        if(GetPixel(dc,column,y)==marker)icon_drawn=TRUE;
    CHECK(icon_drawn);
    CHECK(GetPixel(dc,190,10)==face && GetPixel(dc,120,120)==RGB(199,55,141));
    CHECK(GetCurrentObject(dc,OBJ_FONT)==borrowed && (HFONT)SendMessageW(tab,WM_GETFONT,0,0)==font);
    LPARAM pt=MAKELPARAM((second.left+second.right)/2,(second.top+second.bottom)/2);
    SendMessageW(tab,WM_LBUTTONDOWN,MK_LBUTTON,pt);
    SendMessageW(tab,WM_LBUTTONUP,0,pt);
    CHECK(SendMessageW(tab,TCM_GETCURSEL,0,0)==1 && tab_changes==1);
    EnableWindow(tab,FALSE);
    print_client(tab,dc);
    CHECK(GetPixel(dc,190,10)==face && GetCurrentObject(dc,OBJ_FONT)==borrowed);
    CHECK(remove(tab) && !attached(tab));
    CHECK(SendMessageW(tab,TCM_GETCURSEL,0,0)==1);
    CHECK((HIMAGELIST)SendMessageW(tab,TCM_SETIMAGELIST,0,0)==images && ImageList_Destroy(images));

    header_control=CreateWindowW(WC_HEADERW,NULL,WS_CHILD|WS_VISIBLE|HDS_BUTTONS,
        0,0,250,24,root,NULL,module,NULL);
    CHECK(header_control && (!hooked || attached(header_control)) && attach(header_control));
    HDITEMW column={.mask=HDI_WIDTH|HDI_FORMAT|HDI_TEXT,.cxy=90,.fmt=HDF_STRING|HDF_LEFT,.pszText=L"Name"};
    CHECK(SendMessageW(header_control,HDM_INSERTITEMW,0,(LPARAM)&column)==0);
    column.pszText=L"CPU";column.fmt=HDF_STRING|HDF_RIGHT;
    CHECK(SendMessageW(header_control,HDM_INSERTITEMW,1,(LPARAM)&column)==1);
    print_client(header_control,dc);
    CHECK(header_items==0 && GetPixel(dc,4,0)==RGB(255,255,255) && GetPixel(dc,89,5)==dark);
    CHECK(GetPixel(dc,75,8)==face && GetCurrentObject(dc,OBJ_FONT)==borrowed);
    SendMessageW(header_control,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(25,9));
    print_client(header_control,dc);
    CHECK(GetPixel(dc,4,0)==RGB(128,128,128) && GetPixel(dc,4,1)==face);
    SendMessageW(header_control,WM_LBUTTONUP,0,MAKELPARAM(25,9));
    CHECK(header_clicks==1);
    const int order[2]={1,0};
    CHECK(SendMessageW(header_control,HDM_SETORDERARRAY,2,(LPARAM)order));
    CHECK(SendMessageW(header_control,HDM_GETITEMRECT,0,(LPARAM)&first) && first.left==90);
    custom_header=TRUE;
    print_client(header_control,dc);
    CHECK(header_items==2 && GetPixel(dc,75,8)==marker && GetPixel(dc,164,8)==marker);
    CHECK(remove(header_control) && !attached(header_control));
    print_client(header_control,dc);
    CHECK(header_items==4 && GetPixel(dc,75,8)==marker);
    CHECK(DestroyWindow(root));
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);FreeLibrary(dll);
    puts("WIN2K_TAB_HEADER_CONTRACTS_OK tab=selection+frame+disabled header=raised+pressed+order+custom-draw remove=2");
    return 0;
}
