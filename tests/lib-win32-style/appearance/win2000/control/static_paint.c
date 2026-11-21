/* Real product ABI/STATIC/GROUPBOX consumer; source/original-image contracts. */
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
typedef BOOL (__cdecl *window_op)(HWND);
static window_op attach,attached,remove_style;
static HBRUSH custom_brush;static unsigned action,dialogs,dialog_children;static BOOL fired,hooked,modal;
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_STATIC_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_CTLCOLORSTATIC){
        if(action&&!fired){fired=TRUE;if(action==1)remove_style((HWND)b);else DestroyWindow((HWND)b);}
        SetTextColor((HDC)a,RGB(21,43,65));SetBkColor((HDC)a,GetSysColor(COLOR_BTNFACE));
        return (LRESULT)(custom_brush?custom_brush:GetSysColorBrush(COLOR_BTNFACE));
    }
    return DefWindowProcW(w,m,a,b);
}
static void capture(HWND window,HDC dc)
{int saved=SaveDC(dc);SetDCBrushColor(dc,RGB(201,17,199));RECT r={0,0,100,32};FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);RestoreDC(dc,saved);}
static unsigned long hash(HDC dc)
{unsigned long v=2166136261u;for(int y=0;y<32;++y)for(int x=0;x<100;++x)v=(v^GetPixel(dc,x,y))*16777619u;return v;}
static unsigned count(HDC dc,COLORREF color)
{unsigned n=0;for(int y=0;y<32;++y)for(int x=0;x<100;++x)n+=GetPixel(dc,x,y)==color;return n;}
static int left_ink(HDC dc)
{for(int x=0;x<100;++x)for(int y=0;y<32;++y)if(GetPixel(dc,x,y)==RGB(21,43,65))return x;return 100;}
static BOOL CALLBACK initialized_child(HWND child,LPARAM context)
{(void)context;if(!attached(child))dialog_children+=1000;else ++dialog_children;return TRUE;}
static INT_PTR CALLBACK dialog_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    (void)a;(void)b;
    if(message==WM_INITDIALOG){PostMessageW(window,WM_APP+7,0,0);return TRUE;}
    if(message==WM_APP+7){EnumChildWindows(window,initialized_child,0);++dialogs;
        if(modal)EndDialog(window,73);else DestroyWindow(window);return TRUE;}
    return FALSE;
}
static int separator_fingerprints(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=instance,.lpszClassName=L"OwnedSeparatorThemeTest"};
    CHECK(RegisterClassW(&cls));
    HWND parent=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,cls.lpszClassName,L"",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,-10000,-10000,300,180,NULL,NULL,instance,NULL);
    CHECK(parent);
    HDC dc=CreateCompatibleDC(NULL);
    void *pixels;
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),100,-32,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);
    CHECK(old && old!=HGDI_ERROR);
    const DWORD types[]={SS_ETCHEDHORZ,SS_ETCHEDVERT,SS_ETCHEDFRAME};
    for(unsigned i=0;i<ARRAYSIZE(types);++i){
        HWND window=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE|types[i],
            0,0,100,32,parent,(HMENU)(UINT_PTR)(10+i),instance,NULL);
        RECT client,outer;
        CHECK(window && GetClientRect(window,&client) && GetWindowRect(window,&outer));
        PatBlt(dc,0,0,100,32,BLACKNESS);
        SendMessageW(window,WM_PRINT,(WPARAM)dc,PRF_CLIENT|PRF_NONCLIENT);
        printf("SEPARATOR_THEME type=%lu client=%ldx%ld outer=%ldx%ld pixels=%08lx\n",
            types[i],client.right,client.bottom,outer.right-outer.left,outer.bottom-outer.top,hash(dc));
        CHECK(DestroyWindow(window));
    }
    CHECK(DestroyWindow(parent) && SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc));
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"--separator-fingerprints"))return separator_fingerprints();
    CHECK(argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware")||!wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3&&!wcscmp(argv[2],L"aware");hooked=argc==3&&!wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);CHECK(dll);
    attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");remove_style=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");CHECK(attach&&attached&&remove_style);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=instance,.lpszClassName=L"OwnedWin2kStaticTest"};CHECK(RegisterClassW(&cls));
    HWND parent=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,40,40,300,180,NULL,NULL,instance,NULL);CHECK(parent);
    HDC dc=CreateCompatibleDC(NULL);void *pixels;BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),100,-32,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(dc&&bitmap);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old&&old!=HGDI_ERROR);
    HFONT font=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(font);
    const DWORD types[]={SS_LEFT,SS_CENTER,SS_RIGHT,SS_SIMPLE,SS_LEFTNOWORDWRAP,SS_BLACKRECT,SS_GRAYRECT,SS_WHITERECT,SS_BLACKFRAME,SS_GRAYFRAME,SS_WHITEFRAME,SS_ETCHEDHORZ,SS_ETCHEDVERT,SS_ETCHEDFRAME};
    int ink[3]={0};unsigned renders=0;
    for(unsigned i=0;i<ARRAYSIZE(types);++i){
        HWND w=CreateWindowW(L"STATIC",L"&Label",WS_CHILD|types[i],0,0,100,32,parent,(HMENU)1,instance,NULL);CHECK(w);
        if(hooked)CHECK(attached(w));
        SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);
        HFONT native_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        RECT native;CHECK(GetClientRect(w,&native));if(attached(w))CHECK(remove_style(w));CHECK(attach(w)&&attached(w));capture(w,dc);
        CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==native_font);
        if(i<5){CHECK(count(dc,RGB(21,43,65))>0);if(i<3)ink[i]=left_ink(dc);}
        else if(types[i]>=SS_BLACKRECT&&types[i]<=SS_WHITEFRAME){
            COLORREF color=(types[i]-SS_BLACKRECT)%3==0?RGB(64,64,64):(types[i]-SS_BLACKRECT)%3==1?RGB(128,128,128):RGB(255,255,255);
            CHECK(GetPixel(dc,0,0)==color);CHECK(GetPixel(dc,50,15)==(types[i]<=SS_WHITERECT?color:RGB(201,17,199)));
        }else if(types[i]==SS_ETCHEDHORZ){RECT r;CHECK(GetClientRect(w,&r)&&r.bottom==2);CHECK(GetPixel(dc,5,0)==RGB(128,128,128)&&GetPixel(dc,5,1)==RGB(255,255,255));}
        else if(types[i]==SS_ETCHEDVERT){RECT r;CHECK(GetClientRect(w,&r)&&r.right==2);CHECK(GetPixel(dc,0,5)==RGB(128,128,128)&&GetPixel(dc,1,5)==RGB(255,255,255));}
        else CHECK(GetPixel(dc,0,5)==RGB(128,128,128)&&GetPixel(dc,1,5)==RGB(255,255,255));
        EnableWindow(w,FALSE);capture(w,dc);++renders;
        if(i<5){if(types[i]==SS_SIMPLE)CHECK(count(dc,RGB(21,43,65))>0);else CHECK(count(dc,RGB(128,128,128))>0);}
        EnableWindow(w,TRUE);CHECK(remove_style(w)&&!attached(w));
        if(types[i]==SS_ETCHEDHORZ||types[i]==SS_ETCHEDVERT){RECT r;CHECK(GetClientRect(w,&r));if(types[i]==SS_ETCHEDHORZ)CHECK(r.bottom==0);else CHECK(r.right==0);}
        CHECK(DestroyWindow(w));++renders;
    }
    CHECK(ink[0]<ink[1]&&ink[1]<ink[2]);
    HWND group=CreateWindowW(L"BUTTON",L"Group",WS_CHILD|BS_GROUPBOX,0,0,100,32,parent,(HMENU)2,instance,NULL);CHECK(group&&attach(group));SendMessageW(group,WM_SETFONT,(WPARAM)font,FALSE);
    HGDIOBJ old_font=SelectObject(dc,font);SIZE zero;CHECK(old_font&&GetTextExtentPoint32W(dc,L"0",1,&zero)&&SelectObject(dc,old_font));
    capture(group,dc);CHECK(GetPixel(dc,0,zero.cy/2)==RGB(128,128,128)&&GetPixel(dc,1,zero.cy/2+1)==RGB(255,255,255));
    CHECK(GetPixel(dc,50,25)==RGB(201,17,199)&&count(dc,RGB(21,43,65))>0);
    EnableWindow(group,FALSE);capture(group,dc);CHECK(count(dc,RGB(128,128,128))>10);EnableWindow(group,TRUE);
    capture(group,dc);int left=left_ink(dc);
    SetWindowLongPtrW(group,GWL_STYLE,WS_CHILD|BS_GROUPBOX|BS_CENTER);capture(group,dc);int center=left_ink(dc);CHECK(center>left&&center<100);
    SetWindowLongPtrW(group,GWL_STYLE,WS_CHILD|BS_GROUPBOX|BS_RIGHT);capture(group,dc);CHECK(left_ink(dc)>center);
    SetWindowLongPtrW(group,GWL_STYLE,WS_CHILD|BS_GROUPBOX|BS_FLAT);capture(group,dc);CHECK(GetPixel(dc,0,zero.cy/2)==RGB(0,0,0));
    SetWindowLongPtrW(group,GWL_STYLE,WS_CHILD|BS_GROUPBOX);
    CHECK(SetWindowTextW(group,L""));capture(group,dc);CHECK(GetPixel(dc,50,zero.cy/2)==RGB(128,128,128));
    CHECK(remove_style(group)&&DestroyWindow(group));
    for(unsigned image_type=0;image_type<2;++image_type){
        HWND w=CreateWindowW(L"STATIC",NULL,WS_CHILD|SS_CENTERIMAGE|(image_type?SS_ICON:SS_BITMAP),0,0,100,32,parent,(HMENU)3,instance,NULL);CHECK(w);
        if(attached(w))CHECK(remove_style(w));
        HBITMAP picture=CreateBitmap(4,4,1,32,NULL);CHECK(picture);
        HDC tile_dc=CreateCompatibleDC(dc);CHECK(tile_dc);HGDIOBJ tile_old=SelectObject(tile_dc,picture);CHECK(tile_old);
        SetDCBrushColor(tile_dc,RGB(23,79,131));RECT tile={0,0,4,4};CHECK(FillRect(tile_dc,&tile,(HBRUSH)GetStockObject(DC_BRUSH)));
        CHECK(SelectObject(tile_dc,tile_old)&&DeleteDC(tile_dc));
        HANDLE image=image_type?(HANDLE)LoadIconW(NULL,IDI_APPLICATION):(HANDLE)picture;
        SendMessageW(w,STM_SETIMAGE,image_type?IMAGE_ICON:IMAGE_BITMAP,(LPARAM)image);
        LRESULT original=SendMessageW(w,STM_GETIMAGE,image_type?IMAGE_ICON:IMAGE_BITMAP,0);capture(w,dc);unsigned long native=hash(dc);CHECK(original);
        if(!image_type)CHECK(count(dc,RGB(23,79,131))>0);
        CHECK(attach(w));capture(w,dc);CHECK(hash(dc)==native&&SendMessageW(w,STM_GETIMAGE,image_type?IMAGE_ICON:IMAGE_BITMAP,0)==original);
        CHECK(remove_style(w)&&DestroyWindow(w));BITMAP borrowed;CHECK(GetObjectW(picture,sizeof(borrowed),&borrowed)==sizeof(borrowed)&&DeleteObject(picture));
    }
    HWND label=CreateWindowW(L"STATIC",L"Custom",WS_CHILD|SS_LEFT,0,0,100,32,parent,(HMENU)4,instance,NULL);CHECK(label&&attach(label));
    SendMessageW(label,WM_SETFONT,(WPARAM)font,FALSE);custom_brush=CreateSolidBrush(RGB(12,34,56));CHECK(custom_brush);capture(label,dc);CHECK(GetPixel(dc,99,31)==RGB(12,34,56));CHECK(DeleteObject(custom_brush));custom_brush=NULL;
    HDC pattern_dc=CreateCompatibleDC(dc);HBITMAP pattern=CreateCompatibleBitmap(dc,2,2);CHECK(pattern_dc&&pattern);
    HGDIOBJ pattern_old=SelectObject(pattern_dc,pattern);CHECK(pattern_old);
    CHECK(SetPixelV(pattern_dc,0,0,RGB(1,2,3))&&SetPixelV(pattern_dc,1,1,RGB(1,2,3))&&
        SetPixelV(pattern_dc,1,0,RGB(4,5,6))&&SetPixelV(pattern_dc,0,1,RGB(4,5,6)));
    custom_brush=CreatePatternBrush(pattern);CHECK(custom_brush&&SetWindowPos(label,NULL,3,5,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE));
    capture(label,dc);CHECK(GetPixel(dc,99,31)==RGB(1,2,3)&&GetPixel(dc,98,31)==RGB(4,5,6));
    HDC crop=CreateCompatibleDC(dc);void *crop_pixels;BITMAPINFO crop_info={.bmiHeader={sizeof(BITMAPINFOHEADER),23,-17,1,32,BI_RGB}};
    HBITMAP crop_bitmap=CreateDIBSection(crop,&crop_info,DIB_RGB_COLORS,&crop_pixels,NULL,0);CHECK(crop&&crop_bitmap);
    HGDIOBJ crop_old=SelectObject(crop,crop_bitmap);CHECK(crop_old&&SetWindowOrgEx(crop,3,4,NULL));
    SendMessageW(label,WM_PRINTCLIENT,(WPARAM)crop,PRF_CLIENT);CHECK(SetWindowOrgEx(crop,0,0,NULL));
    for(int y=0;y<17;++y)for(int x=0;x<23;++x)CHECK(GetPixel(crop,x,y)==GetPixel(dc,x+3,y+4));
    CHECK(SelectObject(crop,crop_old)&&DeleteObject(crop_bitmap)&&DeleteDC(crop));
    CHECK(DeleteObject(custom_brush));custom_brush=NULL;
    CHECK(SelectObject(pattern_dc,pattern_old)&&DeleteObject(pattern)&&DeleteDC(pattern_dc)&&DestroyWindow(label));
    puts("WIN2K_STATIC_CONTRACTS_OK group_alignment=3 flat=1 pattern_phase=1 partial_print=1 interior_preserved=1 native_image_witness=1");
    for(action=1;action<=2;++action){fired=FALSE;label=CreateWindowW(L"STATIC",L"Retire",WS_CHILD|SS_LEFT,0,0,100,32,parent,(HMENU)5,instance,NULL);CHECK(label&&attach(label));capture(label,dc);CHECK(fired&&!attached(label));if(IsWindow(label))CHECK(DestroyWindow(label));}action=0;
    DWORD resources=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(resources);
    for(unsigned cycle=0;cycle<32;++cycle){label=CreateWindowA("STATIC","Borrowed",WS_CHILD|SS_CENTER,0,0,100,32,parent,(HMENU)6,instance,NULL);CHECK(label&&attach(label));SendMessageW(label,WM_SETFONT,(WPARAM)font,FALSE);capture(label,dc);CHECK(remove_style(label)&&DestroyWindow(label));}
    CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==resources);
    if(hooked){
        modal=FALSE;HWND dialog=CreateDialogParamW(instance,MAKEINTRESOURCEW(200),NULL,dialog_proc,0);CHECK(dialog);
        MSG message;while(IsWindow(dialog)&&GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
        modal=TRUE;CHECK(DialogBoxParamW(instance,MAKEINTRESOURCEW(200),NULL,dialog_proc,0)==73);
        CHECK(dialogs==2&&dialog_children==8);
    }
    CHECK(DestroyWindow(parent)&&UnregisterClassW(cls.lpszClassName,instance));
    CHECK(SelectObject(dc,old)&&DeleteObject(bitmap)&&DeleteDC(dc)&&DeleteObject(font)&&FreeLibrary(dll));
    printf("WIN2K_STATIC_OK bits=%u aware=%d hooked=%d static_roles=16 group=1 rendered_states=%u alignment=1 native_images=2 parent_brush=1 parent_retire=2 gdi_cycles=32 dialogs=%u initialized_children=%u\n",(unsigned)sizeof(void*)*8,aware,hooked,renders,dialogs,dialog_children);return 0;
}
