/* Product-composition test: use the real published-compatible DLL ABI. */
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <stdio.h>
#include <wchar.h>
typedef BOOL (__cdecl *window_op)(HWND);
static window_op attach,attached,remove_style;
static unsigned clicks,action,background_action;static BOOL fired;
static HBRUSH background_brush;
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_BUTTON_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_COMMAND && HIWORD(a)==BN_CLICKED)++clicks;
    if(m==WM_CTLCOLORSTATIC){
        if(background_action){unsigned selected=background_action;background_action=0;fired=TRUE;
            if(selected==1)remove_style((HWND)b);else DestroyWindow((HWND)b);}
        if(background_brush){SetTextColor((HDC)a,RGB(21,43,65));SetBkColor((HDC)a,RGB(12,34,56));return (LRESULT)background_brush;}
        SetTextColor((HDC)a,RGB(0,0,0));SetBkColor((HDC)a,GetSysColor(COLOR_BTNFACE));return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);}
    return DefWindowProcW(w,m,a,b);
}
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
static unsigned long long patch_hash(HDC dc,int left,int top)
{
    unsigned long long hash=14695981039346656037ull;
    for(int y=0;y<13;++y)for(int x=0;x<13;++x){
        COLORREF color=GetPixel(dc,left+x,top+y);BYTE rgb[3]={GetRValue(color),GetGValue(color),GetBValue(color)};
        for(unsigned i=0;i<3;++i)hash=(hash^rgb[i])*1099511628211ull;
    }
    return hash;
}
static int choices(HWND parent,HINSTANCE instance,HFONT font,HDC dc,BOOL hooked)
{
    const DWORD types[]={BS_CHECKBOX,BS_AUTOCHECKBOX,BS_RADIOBUTTON,BS_AUTORADIOBUTTON,BS_3STATE,BS_AUTO3STATE};
    const unsigned long long golden[]={0xc68eca65c0304287ull,0x2f492bf0b4662f9aull,0x1bfea9cb15233217ull,0xf36f079a80b8802bull};
    unsigned renders=0;
    for(unsigned kind=0;kind<6;++kind)for(unsigned push=0;push<2;++push){
        HWND button=CreateWindowW(L"BUTTON",L"Choice",WS_CHILD|types[kind]|(push?BS_PUSHLIKE:0),0,0,100,32,parent,(HMENU)9,instance,NULL);CHECK(button);
        if(hooked)CHECK(attached(button));
        SendMessageW(button,WM_SETFONT,(WPARAM)font,FALSE);CHECK(attach(button));
        unsigned count=kind>=4?3:2;
        for(unsigned value=0;value<count;++value)for(unsigned overlay=0;overlay<3;++overlay){
            EnableWindow(button,TRUE);SendMessageW(button,BM_SETSTATE,FALSE,0);SendMessageW(button,BM_SETCHECK,value,0);
            if(overlay==1)SendMessageW(button,BM_SETSTATE,TRUE,0);
            if(overlay==2)EnableWindow(button,FALSE);
            CHECK(SendMessageW(button,BM_GETCHECK,0,0)==(LRESULT)value);
            capture(button,dc);
            CHECK((HFONT)SendMessageW(button,WM_GETFONT,0,0)==font && SendMessageW(button,BM_GETCHECK,0,0)==(LRESULT)value);
            if(!push){
                if(!overlay && value<2)CHECK(patch_hash(dc,0,9)==golden[(kind==2||kind==3?2:0)+value]);
                COLORREF center=GetPixel(dc,6,15);
                if(value)CHECK(center==((overlay==2||value==2)?RGB(128,128,128):RGB(0,0,0)));
                else CHECK(center==(overlay?RGB(212,208,200):RGB(255,255,255)));
            }else CHECK(GetPixel(dc,0,10)==((value||overlay==1)?RGB(64,64,64):RGB(255,255,255)));
            ++renders;
        }
        CHECK(remove_style(button)&&DestroyWindow(button));
    }
    CHECK(renders==84);
    /* Native AUTO3STATE still owns its cycle and notifications. */
    HWND automatic=CreateWindowW(L"BUTTON",L"",WS_CHILD|BS_AUTO3STATE,0,0,100,32,parent,(HMENU)10,instance,NULL);CHECK(automatic&&attach(automatic));
    unsigned before=clicks;
    for(unsigned step=1;step<=3;++step){SendMessageW(automatic,BM_CLICK,0,0);CHECK(SendMessageW(automatic,BM_GETCHECK,0,0)==(LRESULT)(step%3));}
    CHECK(clicks==before+3 && DestroyWindow(automatic));
    HWND first=CreateWindowW(L"BUTTON",L"First",WS_CHILD|WS_VISIBLE|WS_GROUP|BS_AUTORADIOBUTTON,0,0,100,32,parent,(HMENU)11,instance,NULL);
    HWND second=CreateWindowW(L"BUTTON",L"Second",WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,0,0,100,32,parent,(HMENU)12,instance,NULL);CHECK(first&&second&&attach(first)&&attach(second));
    SendMessageW(first,BM_CLICK,0,0);CHECK(SendMessageW(first,BM_GETCHECK,0,0)==BST_CHECKED);
    SendMessageW(second,BM_CLICK,0,0);CHECK(SendMessageW(first,BM_GETCHECK,0,0)==BST_UNCHECKED&&SendMessageW(second,BM_GETCHECK,0,0)==BST_CHECKED);
    CHECK(DestroyWindow(first)&&DestroyWindow(second));
    /* Odd offset detects checker phase changes in buffered/printed DCs. */
    HWND mixed=CreateWindowW(L"BUTTON",L"Phase",WS_CHILD|BS_AUTO3STATE|BS_PUSHLIKE,0,0,100,32,parent,(HMENU)13,instance,NULL);CHECK(mixed&&attach(mixed));
    SendMessageW(mixed,WM_SETFONT,(WPARAM)font,FALSE);SendMessageW(mixed,BM_SETCHECK,BST_INDETERMINATE,0);capture(mixed,dc);
    HDC crop=CreateCompatibleDC(dc);void *pixels;BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),23,-17,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(crop,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(crop&&bitmap);
    HGDIOBJ old=SelectObject(crop,bitmap);CHECK(old&&old!=HGDI_ERROR&&SetWindowOrgEx(crop,3,4,NULL));
    SendMessageW(mixed,WM_PRINTCLIENT,(WPARAM)crop,PRF_CLIENT);
    CHECK(SetWindowOrgEx(crop,0,0,NULL));
    for(int y=0;y<17;++y)for(int x=0;x<23;++x)CHECK(GetPixel(crop,x,y)==GetPixel(dc,x+3,y+4));
    CHECK(SelectObject(crop,old)&&DeleteObject(bitmap)&&DeleteDC(crop)&&DestroyWindow(mixed));
    /* Original CBR_CHECKBOX uses a fixed13-cell, half a "0" text gap and
     * font-height TOP/BOTTOM placement; verify the real native role changes. */
    HWND layout=CreateWindowW(L"BUTTON",L"&Layout",WS_CHILD|BS_AUTOCHECKBOX,0,0,100,32,parent,(HMENU)14,instance,NULL);CHECK(layout&&attach(layout));
    SendMessageW(layout,WM_SETFONT,(WPARAM)font,FALSE);
    HGDIOBJ selected_font=SelectObject(dc,font);SIZE zero;CHECK(selected_font&&GetTextExtentPoint32W(dc,L"0",1,&zero));CHECK(SelectObject(dc,selected_font));
    const DWORD positions[]={BS_TOP,BS_BOTTOM,BS_VCENTER};
    for(unsigned place=0;place<3;++place){
        SetWindowLongPtrW(layout,GWL_STYLE,WS_CHILD|BS_AUTOCHECKBOX|BS_LEFTTEXT|positions[place]);
        capture(layout,dc);int row=zero.cy+zero.cy/4;
        int top=place==0?(row-13)/2:place==1?32-row+(row-13)/2:9;
        CHECK(patch_hash(dc,87,top)==golden[0]);
    }
    SetWindowLongPtrW(layout,GWL_STYLE,WS_CHILD|BS_AUTOCHECKBOX);
    SendMessageW(layout,WM_UPDATEUISTATE,MAKEWPARAM(UIS_CLEAR,UISF_HIDEFOCUS|UISF_HIDEACCEL),0);
    capture(layout,dc);unsigned long base=fingerprint(dc);
    SendMessageW(layout,WM_SETFOCUS,0,0);capture(layout,dc);CHECK(fingerprint(dc)!=base);
    SendMessageW(layout,WM_UPDATEUISTATE,MAKEWPARAM(UIS_SET,UISF_HIDEFOCUS),0);capture(layout,dc);CHECK(fingerprint(dc)==base);
    SendMessageW(layout,WM_KILLFOCUS,0,0);
    before=clicks;SendMessageW(layout,WM_KEYDOWN,VK_SPACE,1);CHECK(SendMessageW(layout,BM_GETSTATE,0,0)&BST_PUSHED);
    SendMessageW(layout,WM_KEYUP,VK_SPACE,1);CHECK(SendMessageW(layout,BM_GETCHECK,0,0)==BST_CHECKED&&clicks==before+1);
    SetWindowLongPtrW(layout,GWL_STYLE,WS_CHILD|BS_AUTOCHECKBOX|BS_FLAT);capture(layout,dc);CHECK(GetPixel(dc,1,10)==RGB(0,0,0));
    SetWindowLongPtrW(layout,GWL_STYLE,WS_CHILD|BS_AUTOCHECKBOX|BS_MULTILINE);CHECK(SetWindowTextW(layout,L"Two\nlines"));capture(layout,dc);CHECK(count_color(dc,RGB(0,0,0))>0);
    background_brush=CreateSolidBrush(RGB(12,34,56));CHECK(background_brush);capture(layout,dc);
    CHECK(GetPixel(dc,99,31)==RGB(12,34,56)&&count_color(dc,RGB(21,43,65))>0);
    CHECK(DeleteObject(background_brush));background_brush=NULL;
    CHECK(remove_style(layout)&&!attached(layout)&&DestroyWindow(layout));
    for(unsigned retire=1;retire<=2;++retire){
        HWND reentrant=CreateWindowW(L"BUTTON",L"Parent callback",WS_CHILD|BS_CHECKBOX,0,0,100,32,parent,(HMENU)15,instance,NULL);CHECK(reentrant&&attach(reentrant));
        fired=FALSE;background_action=retire;capture(reentrant,dc);CHECK(fired&&!attached(reentrant));
        if(IsWindow(reentrant))CHECK(DestroyWindow(reentrant));
    }
    puts("WIN2K_CHOICE_CONTRACTS_OK right_top_bottom=3 role_change=1 focus_cue_messages=1 space_messages=1 flat=1 multiline=1 custom_parent_brush=1 parent_remove=1 parent_destroy=1");
    printf("WIN2K_CHOICES_OK types=6 check_values=14 renders=%u source_glyphs=1 native_auto3=1 native_radio_group=1 dirty_phase=1 borrowed_font=1\n",renders);return 0;
}
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
    CHECK(choices(parent,cls.hInstance,font,dc,hooked)==0);
    CHECK(DestroyWindow(parent) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc) && DeleteObject(font) && FreeLibrary(dll));
    printf("WIN2K_BUTTON_OK bits=%u aware=%d hooked=%d original_edge=1 face=1 default=1 pressed=1 disabled=1 font_borrowed=1 native_click=1 bitmap_borrowed=1 pressed_text_offset=1 multiline=1 disabled_emboss=1 focus_cue_messages=1 ansi_cycles=64 gdi_balanced=1 reentrant_remove=1 reentrant_destroy=1\n",(unsigned)sizeof(void*)*8,aware,hooked);return 0;
}
