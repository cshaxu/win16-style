/* Product-composition test: use the real published-compatible DLL ABI. */
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <stdio.h>
#include <wchar.h>
typedef BOOL (__cdecl *window_op)(HWND);
static window_op attach,attached,remove_style;
static unsigned clicks,action;static BOOL fired;
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_BUTTON_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_COMMAND && HIWORD(a)==BN_CLICKED)++clicks;return DefWindowProcW(w,m,a,b);}
static LRESULT CALLBACK query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_GETTEXT && action && !fired){fired=TRUE;
        if(action==1)remove_style(w);else if(action==2)DestroyWindow(w);}
    return DefSubclassProc(w,m,a,b);
}
static void capture(HWND window,HDC dc)
{int saved=SaveDC(dc);PatBlt(dc,0,0,100,32,BLACKNESS);SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);RestoreDC(dc,saved);}
static unsigned count_color(HDC dc,COLORREF color)
{unsigned count=0;for(int y=7;y<25;++y)for(int x=3;x<97;++x)count+=GetPixel(dc,x,y)==color;return count;}
static unsigned long fingerprint(HDC dc)
{unsigned long value=2166136261u;for(int y=3;y<29;++y)for(int x=3;x<97;++x)value=(value^GetPixel(dc,x,y))*16777619u;return value;}
static POINT ink_origin(HDC dc)
{POINT origin={100,32};for(int y=3;y<29;++y)for(int x=3;x<97;++x)if(GetPixel(dc,x,y)==RGB(0,0,0)){origin.x=min(origin.x,x);origin.y=min(origin.y,y);}return origin;}
int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2 || (argc==3 && (!wcscmp(argv[2],L"aware") || !wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3 && !wcscmp(argv[2],L"aware"),hooked=argc==3 && !wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);CHECK(dll);
    attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");remove_style=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");CHECK(attach&&attached&&remove_style);
    WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"OwnedWin2kButtonTest"};CHECK(RegisterClassW(&cls));
    HWND parent=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,40,40,300,180,NULL,NULL,cls.hInstance,NULL);CHECK(parent);
    HDC dc=CreateCompatibleDC(NULL);void *pixels;BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),100,-32,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(dc&&bitmap);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old&&old!=HGDI_ERROR);
    HFONT font=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(font);
    HWND button=CreateWindowW(L"BUTTON",L"&Original",WS_CHILD|BS_PUSHBUTTON,0,0,100,32,parent,(HMENU)1,cls.hInstance,NULL);CHECK(button);
    SendMessageW(button,WM_SETFONT,(WPARAM)font,FALSE);if(hooked)CHECK(attached(button));CHECK(attach(button)&&attached(button));
    capture(button,dc);
    CHECK(GetPixel(dc,0,10)==RGB(255,255,255) && GetPixel(dc,5,5)==RGB(212,208,200));
    CHECK((HFONT)SendMessageW(button,WM_GETFONT,0,0)==font);
    SendMessageW(button,BM_SETSTYLE,BS_DEFPUSHBUTTON,FALSE);capture(button,dc);
    CHECK(GetPixel(dc,0,10)==RGB(0,0,0) && GetPixel(dc,1,10)==RGB(255,255,255) && GetPixel(dc,2,10)==RGB(212,208,200));
    SendMessageW(button,BM_SETSTATE,TRUE,0);capture(button,dc);
    CHECK(GetPixel(dc,0,10)==RGB(0,0,0) && GetPixel(dc,1,10)==RGB(128,128,128));
    SendMessageW(button,BM_SETSTATE,FALSE,0);EnableWindow(button,FALSE);capture(button,dc);
    CHECK(GetPixel(dc,5,5)==RGB(212,208,200) && count_color(dc,RGB(128,128,128))>0 && count_color(dc,RGB(255,255,255))>0);
    EnableWindow(button,TRUE);
    /* Exercise BUTTON's own focus/UI-cue messages, not a fabricated paint flag.
     * This verifies message/render behavior, not physical keyboard focus. */
    SendMessageW(button,WM_UPDATEUISTATE,MAKEWPARAM(UIS_CLEAR,UISF_HIDEFOCUS),0);
    capture(button,dc);unsigned long unfocused=fingerprint(dc);
    SendMessageW(button,WM_SETFOCUS,0,0);CHECK(SendMessageW(button,BM_GETSTATE,0,0)&BST_FOCUS);
    capture(button,dc);CHECK(fingerprint(dc)!=unfocused);
    SendMessageW(button,WM_UPDATEUISTATE,MAKEWPARAM(UIS_SET,UISF_HIDEFOCUS),0);
    capture(button,dc);CHECK(fingerprint(dc)==unfocused);
    SendMessageW(button,WM_KILLFOCUS,0,0);
    SendMessageW(button,BM_CLICK,0,0);CHECK(clicks==1 && SendMessageW(button,BM_GETCHECK,0,0)==BST_UNCHECKED);
    SetWindowLongPtrW(button,GWL_STYLE,WS_CHILD|BS_PUSHBUTTON|BS_LEFT|BS_TOP);
    CHECK(SetWindowTextW(button,L"X"));capture(button,dc);POINT normal=ink_origin(dc);CHECK(normal.x<100);
    SendMessageW(button,BM_SETSTATE,TRUE,0);capture(button,dc);POINT pressed=ink_origin(dc);
    CHECK(pressed.x==normal.x+1 && pressed.y==normal.y+1);SendMessageW(button,BM_SETSTATE,FALSE,0);
    SetWindowLongPtrW(button,GWL_STYLE,WS_CHILD|BS_PUSHBUTTON|BS_MULTILINE);
    CHECK(SetWindowTextW(button,L"Original\nmultiline"));capture(button,dc);CHECK(count_color(dc,RGB(0,0,0))>0);
    /* Application-owned bitmap remains borrowed through rendering and removal. */
    HBITMAP image=CreateBitmap(4,4,1,32,NULL);CHECK(image);
    HDC image_dc=CreateCompatibleDC(dc);CHECK(image_dc);HGDIOBJ image_old=SelectObject(image_dc,image);CHECK(image_old);
    HBRUSH marker=CreateSolidBrush(RGB(23,79,131));RECT tile={0,0,4,4};CHECK(marker&&FillRect(image_dc,&tile,marker));
    CHECK(SelectObject(image_dc,image_old)&&DeleteDC(image_dc)&&DeleteObject(marker));
#ifdef WIN2K_BUTTON_V6
    HWND list_button=CreateWindowW(L"BUTTON",L"Image + text",WS_CHILD,0,0,100,32,parent,(HMENU)4,cls.hInstance,NULL);CHECK(list_button);
    /* Startup mode already owns newly created controls; establish a native baseline. */
    if(attached(list_button))CHECK(remove_style(list_button));
    CHECK(SUCCEEDED(SetWindowTheme(list_button,L" ",L" ")));
    HIMAGELIST images=ImageList_Create(4,4,ILC_COLOR32|ILC_MASK,1,0);CHECK(images);
    CHECK(ImageList_AddMasked(images,image,RGB(255,0,255))==0);
    BUTTON_IMAGELIST settings={.himl=images,.uAlign=BUTTON_IMAGELIST_ALIGN_LEFT};
    CHECK(SendMessageW(list_button,BCM_SETIMAGELIST,0,(LPARAM)&settings));
    capture(list_button,dc);unsigned long native_list=fingerprint(dc);
    fprintf(stderr,"V6_LIST native_marker=%u left=%08lx middle=%08lx dpi=%u visible=%d\n",count_color(dc,RGB(23,79,131)),GetPixel(dc,4,15),GetPixel(dc,50,15),GetDpiForWindow(list_button),IsWindowVisible(list_button));
    CHECK(count_color(dc,RGB(23,79,131))>0);
    CHECK(attach(list_button));capture(list_button,dc);CHECK(fingerprint(dc)==native_list);
    RECT before,after;CHECK(GetClipBox(dc,&before)!=ERROR);
    SendMessageW(list_button,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    CHECK(GetClipBox(dc,&after)!=ERROR && EqualRect(&before,&after));
    CHECK(remove_style(list_button)&&DestroyWindow(list_button)&&ImageList_Destroy(images));
    puts("WIN2K_BUTTON_V6 image_list_native_preserved=1 dc_restored=1");
#endif
    SetWindowLongPtrW(button,GWL_STYLE,WS_CHILD|BS_PUSHBUTTON|BS_BITMAP);SendMessageW(button,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)image);
    capture(button,dc);CHECK(GetPixel(dc,49,15)==RGB(23,79,131));
    CHECK((HBITMAP)SendMessageW(button,BM_GETIMAGE,IMAGE_BITMAP,0)==image);
    SetWindowLongPtrW(button,GWL_STYLE,WS_CHILD|BS_PUSHBUTTON);
    CHECK(remove_style(button)&&!attached(button) && !remove_style(button));CHECK(attach(button)&&DestroyWindow(button));
    for(action=1;action<=2;++action){
        fired=FALSE;button=CreateWindowW(L"BUTTON",L"Retire",WS_CHILD,0,0,100,32,parent,(HMENU)2,cls.hInstance,NULL);CHECK(button&&attach(button));
        CHECK(SetWindowSubclass(button,query,7,0));capture(button,dc);CHECK(fired && !attached(button));
        if(IsWindow(button)){CHECK(RemoveWindowSubclass(button,query,7) && DestroyWindow(button));}
    }
    action=0;
    DWORD objects=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(objects);
    for(unsigned cycle=0;cycle<64;++cycle){
        button=CreateWindowA("BUTTON","Borrowed",WS_CHILD,0,0,100,32,parent,(HMENU)3,cls.hInstance,NULL);CHECK(button);
        SendMessageW(button,WM_SETFONT,(WPARAM)font,FALSE);CHECK(attach(button));capture(button,dc);
        CHECK(GetPixel(dc,5,5)==RGB(212,208,200) && remove_style(button) && DestroyWindow(button));
    }
    CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==objects);
    BITMAP borrowed;CHECK(GetObjectW(image,sizeof(borrowed),&borrowed)==sizeof(borrowed)&&DeleteObject(image));
    CHECK(DestroyWindow(parent) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc) && DeleteObject(font) && FreeLibrary(dll));
    printf("WIN2K_BUTTON_OK bits=%u aware=%d hooked=%d original_edge=1 face=1 default=1 pressed=1 disabled=1 font_borrowed=1 native_click=1 bitmap_borrowed=1 pressed_text_offset=1 multiline=1 disabled_emboss=1 focus_cue_messages=1 ansi_cycles=64 gdi_balanced=1 reentrant_remove=1 reentrant_destroy=1\n",(unsigned)sizeof(void*)*8,aware,hooked);return 0;
}
