/* Actual DLL + native edit consumers, not a replacement text implementation. */
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <ole2.h>
#include <stdio.h>
#include <wchar.h>
typedef BOOL (__cdecl *window_op)(HWND);
static window_op attach,attached,remove_style;
static HBRUSH custom_brush;static unsigned action,dialog_children;static BOOL fired,hooked;
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_EDIT_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_CTLCOLOREDIT||m==WM_CTLCOLORSTATIC){
        if(action&&!fired){fired=TRUE;if(action==1)remove_style((HWND)b);else DestroyWindow((HWND)b);}
        COLORREF color=custom_brush?RGB(12,34,56):GetSysColor(m==WM_CTLCOLOREDIT?COLOR_WINDOW:COLOR_BTNFACE);
        SetTextColor((HDC)a,RGB(0,0,0));SetBkColor((HDC)a,color);
        return (LRESULT)(custom_brush?custom_brush:GetSysColorBrush(m==WM_CTLCOLOREDIT?COLOR_WINDOW:COLOR_BTNFACE));
    }
    return DefWindowProcW(w,m,a,b);
}
static void capture(HWND window,HDC dc,BOOL frame)
{
    int saved=SaveDC(dc);SetDCBrushColor(dc,RGB(201,17,199));RECT r={0,0,200,80};FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    SendMessageW(window,frame?WM_PRINT:WM_PRINTCLIENT,(WPARAM)dc,frame?PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND:PRF_CLIENT);RestoreDC(dc,saved);
}
static unsigned count_color(HDC dc,COLORREF color)
{unsigned n=0;for(int y=3;y<45;++y)for(int x=3;x<157;++x)n+=GetPixel(dc,x,y)==color;return n;}
static BOOL edge_pixels(HWND window,HDC dc)
{
    RECT r;if(!GetWindowRect(window,&r))return FALSE;int width=r.right-r.left,height=r.bottom-r.top;
    return width>=4&&height>=4&&GetPixel(dc,0,height/2)==RGB(128,128,128)&&GetPixel(dc,1,height/2)==RGB(64,64,64)&&
        GetPixel(dc,width-2,height/2)==RGB(212,208,200)&&GetPixel(dc,width-1,height/2)==RGB(255,255,255)&&
        GetPixel(dc,width/2,0)==RGB(128,128,128)&&GetPixel(dc,width/2,1)==RGB(64,64,64)&&
        GetPixel(dc,width/2,height-2)==RGB(212,208,200)&&GetPixel(dc,width/2,height-1)==RGB(255,255,255);
}
static DWORD rich_format(HWND window,BOOL set,BOOL wide)
{
    DWORD effects;
    if(wide){
        CHARFORMATW f={.cbSize=sizeof(f),.dwMask=CFM_BOLD|CFM_COLOR,.dwEffects=CFE_BOLD,.crTextColor=RGB(23,79,131)};
        if(!set)f.dwEffects=0;
        SendMessageW(window,set?EM_SETCHARFORMAT:EM_GETCHARFORMAT,SCF_SELECTION,(LPARAM)&f);effects=f.dwEffects;
    }else{
        CHARFORMATA f={.cbSize=sizeof(f),.dwMask=CFM_BOLD|CFM_COLOR,.dwEffects=CFE_BOLD,.crTextColor=RGB(23,79,131)};
        if(!set)f.dwEffects=0;
        SendMessageA(window,set?EM_SETCHARFORMAT:EM_GETCHARFORMAT,SCF_SELECTION,(LPARAM)&f);effects=f.dwEffects;
    }
    return effects;
}
static BOOL CALLBACK initialized_child(HWND child,LPARAM value)
{(void)value;dialog_children+=attached(child)?1:1000;return TRUE;}
static INT_PTR CALLBACK dialog_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    (void)a;(void)b;
    if(message==WM_INITDIALOG){PostMessageW(window,WM_APP+9,0,0);return TRUE;}
    if(message==WM_APP+9){EnumChildWindows(window,initialized_child,0);EndDialog(window,73);return TRUE;}
    return FALSE;
}
int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2||(argc==3&&(!wcscmp(argv[2],L"aware")||!wcscmp(argv[2],L"hooked"))));
    BOOL aware=argc==3&&!wcscmp(argv[2],L"aware");hooked=argc==3&&!wcscmp(argv[2],L"hooked");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    CHECK(SUCCEEDED(OleInitialize(NULL)));INITCOMMONCONTROLSEX common={sizeof(common),ICC_WIN95_CLASSES};CHECK(InitCommonControlsEx(&common));
    HMODULE rich20=LoadLibraryW(L"riched20.dll"),rich50=LoadLibraryW(L"msftedit.dll");CHECK(rich20&&rich50);
    HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);CHECK(dll);
    attach=(void *)GetProcAddress(dll,"Ntw2ksWindowAttach");attached=(void *)GetProcAddress(dll,"Ntw2ksWindowAttached");remove_style=(void *)GetProcAddress(dll,"Ntw2ksWindowRemove");CHECK(attach&&attached&&remove_style);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=instance,.lpszClassName=L"OwnedWin2kEditTest"};CHECK(RegisterClassW(&cls));
    HWND parent=CreateWindowExW(WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,cls.lpszClassName,L"",WS_POPUP|WS_VISIBLE,-10000,-10000,300,180,NULL,NULL,instance,NULL);CHECK(parent);
    HDC dc=CreateCompatibleDC(NULL);void *pixels;BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),200,-80,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(dc&&bitmap);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old&&old!=HGDI_ERROR);
    HFONT font=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(font);
    const DWORD modes[]={ES_AUTOHSCROLL,ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,ES_READONLY,WS_DISABLED,ES_PASSWORD,0,WS_BORDER};
    for(unsigned mode=0;mode<ARRAYSIZE(modes);++mode){
        DWORD ex=mode<5?WS_EX_CLIENTEDGE:0;
        HWND w=CreateWindowExW(ex,L"EDIT",L"alpha",WS_CHILD|WS_VISIBLE|modes[mode],0,0,160,48,parent,(HMENU)1,instance,NULL);CHECK(w);
        if(hooked)CHECK(attached(w));
        if(attached(w))CHECK(remove_style(w));
        SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);HFONT native_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        RECT before,after;CHECK(GetClientRect(w,&before));LRESULT mask=SendMessageW(w,EM_GETPASSWORDCHAR,0,0);
        unsigned baseline_gray=0;
        if(mode==3){capture(w,dc,TRUE);baseline_gray=count_color(dc,GetSysColor(COLOR_GRAYTEXT));}
        CHECK(attach(w)&&attached(w)&&GetClientRect(w,&after)&&EqualRect(&before,&after));capture(w,dc,TRUE);
        if(ex)CHECK(edge_pixels(w,dc));
        CHECK(GetPixel(dc,100,30)==((mode==2||mode==3)?RGB(212,208,200):RGB(255,255,255)));
        if(mode==3){
            fprintf(stderr,"EDIT_DISABLED_NATIVE system=%08lx before_count=%u after_count=%u original128_count=%u\n",GetSysColor(COLOR_GRAYTEXT),baseline_gray,count_color(dc,GetSysColor(COLOR_GRAYTEXT)),count_color(dc,RGB(128,128,128)));
            CHECK(baseline_gray>0&&count_color(dc,GetSysColor(COLOR_GRAYTEXT))>0);
        }
        CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==native_font&&SendMessageW(w,EM_GETPASSWORDCHAR,0,0)==mask);
        if(mode==4)CHECK(mask!=0);
        DWORD start=0,end=0;SendMessageW(w,EM_SETSEL,1,4);SendMessageW(w,EM_GETSEL,(WPARAM)&start,(LPARAM)&end);CHECK(start==1&&end==4);
        if(mode==0||mode==1||mode>=5){
            SendMessageW(w,EM_REPLACESEL,TRUE,(LPARAM)L"XYZ");CHECK(SendMessageW(w,EM_CANUNDO,0,0)&&SendMessageW(w,EM_UNDO,0,0));
        }
        WCHAR text[32];CHECK(GetWindowTextW(w,text,ARRAYSIZE(text))==5&&!wcscmp(text,L"alpha"));
        CHECK(SetWindowTextW(w,L"updated"));CHECK(GetWindowTextW(w,text,ARRAYSIZE(text))==7&&!wcscmp(text,L"updated"));
        CHECK(remove_style(w)&&!attached(w)&&GetClientRect(w,&after)&&EqualRect(&before,&after));CHECK(DestroyWindow(w));
    }
    DWORD resources=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(resources);
    for(unsigned cycle=0;cycle<32;++cycle){HWND w=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT","Borrowed",WS_CHILD|WS_VISIBLE|ES_READONLY,0,0,160,48,parent,(HMENU)6,instance,NULL);CHECK(w);SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);CHECK(attach(w));capture(w,dc,TRUE);CHECK(remove_style(w)&&DestroyWindow(w));}
    DWORD resources_after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    fprintf(stderr,"EDIT_GDI_CYCLES before=%lu after=%lu delta=%ld\n",resources,resources_after,(long)resources_after-(long)resources);
    CHECK(resources_after==resources);
    const WCHAR *const classes[]={L"RichEdit20A",L"RichEdit20W",L"RICHEDIT50W",HOTKEY_CLASSW};
    for(unsigned kind=0;kind<ARRAYSIZE(classes);++kind){
        HWND w=kind?CreateWindowExW(WS_EX_CLIENTEDGE,classes[kind],L"alpha",WS_CHILD|WS_VISIBLE,0,0,160,48,parent,(HMENU)2,instance,NULL):
            CreateWindowExA(WS_EX_CLIENTEDGE,"RichEdit20A","alpha",WS_CHILD|WS_VISIBLE,0,0,160,48,parent,(HMENU)2,instance,NULL);CHECK(w);
        if(hooked)CHECK(attached(w));
        if(attached(w))CHECK(remove_style(w));
        SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);
        if(kind<3){SendMessageW(w,EM_SETSEL,1,4);rich_format(w,TRUE,kind!=0);SendMessageW(w,EM_SETBKGNDCOLOR,0,RGB(55,79,109));}
        else SendMessageW(w,HKM_SETHOTKEY,MAKEWORD('K',HOTKEYF_CONTROL|HOTKEYF_ALT),0);
        DWORD effects=kind<3?rich_format(w,FALSE,kind!=0):0;HFONT native_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        capture(w,dc,FALSE);COLORREF native_client=GetPixel(dc,100,30);
        CHECK(attach(w));capture(w,dc,TRUE);CHECK(edge_pixels(w,dc));
        COLORREF framed_client=GetPixel(dc,100,30);capture(w,dc,FALSE);COLORREF styled_client=GetPixel(dc,100,30);
        if(native_client==RGB(201,17,199))fprintf(stderr,"NATIVE_CLIENT_PRINT_SKIP kind=%u reason=native_WM_PRINTCLIENT_no_pixels_before_or_after\n",kind);
        fprintf(stderr,"EDIT_NATIVE_CLIENT kind=%u unicode=%d before_effects=%08lx after_effects=%08lx before_client=%08lx after_full=%08lx after_client=%08lx\n",kind,IsWindowUnicode(w),effects,kind<3?rich_format(w,FALSE,kind!=0):0,native_client,framed_client,styled_client);
        if(kind<3){CHECK((effects&CFE_BOLD)&&(rich_format(w,FALSE,kind!=0)&CFE_BOLD)&&styled_client==native_client);}
        else CHECK(SendMessageW(w,HKM_GETHOTKEY,0,0)==MAKEWORD('K',HOTKEYF_CONTROL|HOTKEYF_ALT));
        CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==native_font&&remove_style(w)&&DestroyWindow(w));
    }
    HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"Custom",WS_CHILD|WS_VISIBLE|ES_READONLY,0,0,160,48,parent,(HMENU)3,instance,NULL);CHECK(w&&attach(w));
    custom_brush=CreateSolidBrush(RGB(12,34,56));CHECK(custom_brush);capture(w,dc,TRUE);CHECK(GetPixel(dc,100,30)==RGB(12,34,56));CHECK(DeleteObject(custom_brush));custom_brush=NULL;CHECK(DestroyWindow(w));
    for(action=1;action<=2;++action){
        fired=FALSE;w=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"Retire",WS_CHILD|WS_VISIBLE,0,0,160,48,parent,(HMENU)4,instance,NULL);CHECK(w&&attach(w));
        if(action==1)capture(w,dc,TRUE);
        /* Native v6 EDIT itself crashes if destroyed inside its paint-time
         * CTLCOLOR. Test parent callback retirement outside that native paint. */
        else SendMessageW(parent,WM_CTLCOLOREDIT,(WPARAM)dc,(LPARAM)w);
        CHECK(fired&&!attached(w));if(IsWindow(w))CHECK(DestroyWindow(w));
    }action=0;
    HWND other=CreateWindowExW(WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,cls.lpszClassName,L"",WS_POPUP|WS_VISIBLE,-10000,-10000,300,180,NULL,NULL,instance,NULL);CHECK(other);
    w=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"Reparent",WS_CHILD|WS_VISIBLE,0,0,160,48,parent,(HMENU)5,instance,NULL);CHECK(w&&attach(w));CHECK(SetParent(w,other)==parent);SendMessageW(w,WM_NULL,0,0);CHECK(!attached(w)&&attach(w));CHECK(DestroyWindow(w)&&DestroyWindow(other));
    if(hooked){CHECK(DialogBoxParamW(instance,MAKEINTRESOURCEW(240),NULL,dialog_proc,0)==73&&dialog_children==2);}
    CHECK(DestroyWindow(parent)&&UnregisterClassW(cls.lpszClassName,instance));BITMAP borrowed;LOGFONTW borrowed_font;
    CHECK(GetObjectW(bitmap,sizeof(borrowed),&borrowed)==sizeof(borrowed)&&GetObjectW(font,sizeof(borrowed_font),&borrowed_font)==sizeof(borrowed_font));
    CHECK(SelectObject(dc,old)&&DeleteObject(bitmap)&&DeleteDC(dc)&&DeleteObject(font)&&FreeLibrary(dll)&&FreeLibrary(rich20)&&FreeLibrary(rich50));OleUninitialize();
    printf("WIN2K_EDIT_OK bits=%u aware=%d hooked=%d edit_roles=7 rich_hotkey=4 source_edge=1 standard_face=1 native_text_undo_selection=1 font_password_preserved=1 custom_brush=1 parent_retire=2 reparent=1 gdi_cycles=32 initialized_children=%u\n",(unsigned)sizeof(void*)*8,aware,hooked,dialog_children);return 0;
}
