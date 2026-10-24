/* Actual candidate DLL, native dialog units and application font ownership. */
#include "ntw16s-dll/win16_style.h"
#define NTW16S_TEST_PROCESS_TIMEOUT 170000
#include "isolated_process.h"
#include <commctrl.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FONT LAYOUT FAIL line=%d error=%lu\n",__LINE__,GetLastError());ExitProcess(1);}}while(0)
typedef BOOL (__cdecl *install_fn)(HWND,const win16_style_palette *);
typedef BOOL (__cdecl *remove_fn)(HWND);
static install_fn install_frame,install_control;
static BOOL (__cdecl *install_edit)(HWND,BOOL,const win16_style_palette *);
static remove_fn remove_frame,remove_control;
static BOOL verify_pixels;
static void settle(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}GdiFlush();}
static RECT rect(HWND w,HWND parent)
{RECT r;CHECK(GetWindowRect(w,&r));MapWindowPoints(NULL,parent,(POINT *)&r,2);return r;}
static SIZE text_size(HWND w,HFONT f)
{WCHAR text[128];RECT r={0};HDC dc=GetDC(w);CHECK(dc);HGDIOBJ old=SelectObject(dc,f?f:GetStockObject(SYSTEM_FONT));
 if(!GetWindowTextW(w,text,128))lstrcpyW(text,L"Representative text");
 CHECK(DrawTextW(dc,text,-1,&r,DT_CALCRECT|DT_SINGLELINE));
 SelectObject(dc,old);ReleaseDC(w,dc);return (SIZE){r.right,r.bottom};}
static HFONT system_font(HWND w,HFONT original)
{
    HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);HDC dc=CreateCompatibleDC(NULL),expected=CreateCompatibleDC(NULL);
    HBITMAP a=CreateBitmap(128,64,1,1,NULL),b=CreateBitmap(128,64,1,1,NULL);
    HFONT base=CreateFontW(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_RASTER_PRECIS,0,NONANTIALIASED_QUALITY,0,L"System");
    CHECK(dc && expected && a && b && base);HGDIOBJ oa=SelectObject(dc,a),ob=SelectObject(expected,b);
    HGDIOBJ of=SelectObject(dc,original?original:GetStockObject(SYSTEM_FONT));TEXTMETRICW before,after,sys;
    CHECK(GetTextMetricsW(dc,&before));SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));
    char face[32];CHECK(GetTextFaceA(dc,32,face)>0 && (!strcmp(face,"System") || !strncmp(face,"Win16 System",12)));
    CHECK(GetTextMetricsW(dc,&after) && after.tmHeight==before.tmHeight &&
        after.tmAscent==before.tmAscent && after.tmInternalLeading==before.tmInternalLeading &&
        after.tmExternalLeading==before.tmExternalLeading);
    for(WCHAR ch=32;ch<127;++ch){SIZE oldsize,newsize;SelectObject(dc,original?original:GetStockObject(SYSTEM_FONT));
        CHECK(GetTextExtentPoint32W(dc,&ch,1,&oldsize));SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));
        CHECK(GetTextExtentPoint32W(dc,&ch,1,&newsize) && newsize.cx==oldsize.cx);}
    HGDIOBJ ef=SelectObject(expected,base);CHECK(GetTextMetricsW(expected,&sys));
    /* Independent host-System raster oracle. Exact pixel comparison rejects
     * merely renaming a Tahoma/MS Sans Serif font and the previous S1 DLL. */
    for(const WCHAR *ch=L"GQag012";verify_pixels && *ch;++ch){SIZE width,sw;CHECK(GetTextExtentPoint32W(dc,ch,1,&width) && GetTextExtentPoint32W(expected,ch,1,&sw));
        RECT clear={0,0,128,64};FillRect(dc,&clear,GetStockObject(WHITE_BRUSH));FillRect(expected,&clear,GetStockObject(WHITE_BRUSH));
        SetTextColor(dc,RGB(0,0,0));SetBkColor(dc,RGB(255,255,255));SetTextColor(expected,RGB(0,0,0));SetBkColor(expected,RGB(255,255,255));
        CHECK(TextOutW(dc,0,0,ch,1) && TextOutW(expected,0,0,ch,1));
        HDC scaled=CreateCompatibleDC(NULL);HBITMAP bitmap=CreateBitmap(128,64,1,1,NULL);CHECK(scaled && bitmap);
        HGDIOBJ old=SelectObject(scaled,bitmap);SetStretchBltMode(scaled,COLORONCOLOR);
        CHECK(StretchBlt(scaled,0,0,width.cx,after.tmHeight,expected,0,0,sw.cx,sys.tmHeight,SRCCOPY));
        for(int y=0;y<after.tmHeight;++y)for(int x=0;x<width.cx;++x)CHECK(GetPixel(dc,x,y)==GetPixel(scaled,x,y));
        SelectObject(scaled,old);DeleteObject(bitmap);DeleteDC(scaled);
    }
    SelectObject(dc,of);SelectObject(expected,ef);SelectObject(dc,oa);SelectObject(expected,ob);
    DeleteObject(a);DeleteObject(b);DeleteObject(base);DeleteDC(dc);DeleteDC(expected);return font;
}
static void unchanged(HWND w,HWND parent,HFONT font,RECT before,SIZE text)
{RECT now=rect(w,parent);SIZE size=text_size(w,font);CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font);
 CHECK(EqualRect(&now,&before) && size.cx==text.cx && size.cy==text.cy);}
static INT_PTR CALLBACK dialog_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)a;(void)b;return m==WM_INITDIALOG;}
typedef struct reentrant_font {HFONT next;BOOL armed,entered;} reentrant_font;
static LRESULT CALLBACK font_callback(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    reentrant_font *state=(reentrant_font *)data;(void)id;
    if(m==WM_SETFONT && state->armed && !state->entered){LOGFONTW lf;state->entered=TRUE;
        SendMessageW(w,WM_SETFONT,(WPARAM)state->next,FALSE);
        CHECK(GetObjectW((HFONT)a,sizeof(lf),&lf)==sizeof(lf));
        CHECK(remove_control(w));
        CHECK(GetObjectW((HFONT)a,sizeof(lf),&lf)==sizeof(lf));
        CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==state->next);return 0;
    }
    return DefSubclassProc(w,m,a,b);
}
static void reentrant_case(void)
{
    HWND root=CreateWindowW(L"STATIC",L"Font lifetime",WS_OVERLAPPEDWINDOW,0,0,400,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HWND child=CreateWindowW(L"BUTTON",L"Nested",WS_CHILD,0,0,200,40,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(child);
    HFONT first=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Tahoma");
    HFONT next=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Arial");CHECK(first && next);
    SendMessageW(child,WM_SETFONT,(WPARAM)first,FALSE);reentrant_font state={.next=next};
    CHECK(SetWindowSubclass(child,font_callback,1,(DWORD_PTR)&state) && install_control(child,NULL));
    state.armed=TRUE;SendMessageW(child,WM_SETFONT,(WPARAM)first,FALSE);
    CHECK(state.entered && (HFONT)SendMessageW(child,WM_GETFONT,0,0)==next);
    CHECK(RemoveWindowSubclass(child,font_callback,1) && DestroyWindow(root) && DeleteObject(first) && DeleteObject(next));
    puts("FONT REENTRANT PASS nested change/remove retains outbound System font and restores latest app font");
}
static void menu_case(void)
{
    HWND w=CreateWindowW(L"STATIC",L"Typography",WS_OVERLAPPEDWINDOW,0,0,900,700,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    UINT dpi=GetDpiForWindow(w);HMENU popup=CreatePopupMenu();CHECK(popup && AppendMenuW(popup,MF_STRING,101,L"GQag012"));
    CHECK(install_frame(w,NULL));SendMessageW(w,WM_INITMENUPOPUP,(WPARAM)popup,0);
    MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_DATA|MIIM_FTYPE};CHECK(GetMenuItemInfoW(popup,0,TRUE,&item) && (item.fType&MFT_OWNERDRAW));
    HFONT source=CreateFontW(-MulDiv(8,(int)dpi,72),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"MS Sans Serif");CHECK(source);
    HWND label=CreateWindowW(L"STATIC",L"GQag012",WS_CHILD,0,0,300,60,w,NULL,GetModuleHandleW(NULL),NULL);CHECK(label);
    SendMessageW(label,WM_SETFONT,(WPARAM)source,FALSE);CHECK(install_control(label,NULL));HFONT font=system_font(label,source);
    HDC dc=CreateCompatibleDC(NULL),expected=CreateCompatibleDC(NULL);HBITMAP a=CreateBitmap(400,100,1,1,NULL),b=CreateBitmap(400,100,1,1,NULL);CHECK(dc && expected && a && b);
    HGDIOBJ oa=SelectObject(dc,a),ob=SelectObject(expected,b),of=SelectObject(expected,font);TEXTMETRICW tm;CHECK(GetTextMetricsW(expected,&tm));
    MEASUREITEMSTRUCT measure={.CtlType=ODT_MENU,.itemID=101,.itemData=item.dwItemData};CHECK(SendMessageW(w,WM_MEASUREITEM,0,(LPARAM)&measure));
    CHECK(measure.itemHeight==(UINT)(tm.tmHeight+2*max(1,MulDiv(1,(int)dpi,96))) && measure.itemWidth<400 && measure.itemHeight<100);
    DRAWITEMSTRUCT draw={.CtlType=ODT_MENU,.itemID=101,.itemAction=ODA_DRAWENTIRE,.hwndItem=(HWND)popup,.hDC=dc,
        .rcItem={0,0,(LONG)measure.itemWidth,(LONG)measure.itemHeight},.itemData=item.dwItemData};
    CHECK(SendMessageW(w,WM_DRAWITEM,0,(LPARAM)&draw));RECT r=draw.rcItem;
    FillRect(expected,&r,GetStockObject(WHITE_BRUSH));SetTextColor(expected,RGB(0,0,0));SetBkMode(expected,TRANSPARENT);
    r.left+=MulDiv(18,(int)dpi,96);r.right-=MulDiv(18,(int)dpi,96);CHECK(DrawTextW(expected,L"GQag012",-1,&r,DT_LEFT|DT_SINGLELINE|DT_VCENTER));
    for(int y=0;y<draw.rcItem.bottom;++y)for(int x=0;x<draw.rcItem.right;++x)
        CHECK((GetPixel(dc,x,y)==RGB(0,0,0))==(GetPixel(expected,x,y)==RGB(0,0,0)));
    SelectObject(expected,of);SelectObject(dc,oa);SelectObject(expected,ob);DeleteObject(a);DeleteObject(b);DeleteDC(dc);DeleteDC(expected);
    CHECK(remove_control(label) && remove_frame(w) && DestroyWindow(w) && DestroyMenu(popup) && DeleteObject(source));
    printf("FONT MENU SYSTEM PASS dpi=%u original 8pt metrics; actual WM_DRAWITEM pixel oracle\n",dpi);
}
static void put_word(BYTE **p,WORD w){memcpy(*p,&w,2);*p+=2;}
static void put_string(BYTE **p,const WCHAR *s)
{do{put_word(p,*s);}while(*s++);}
static HWND native_dialog(void)
{
    BYTE data[2048]={0},*p=data+sizeof(DLGTEMPLATE);
    DLGTEMPLATE t={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_SETFONT,.cdit=4,.cx=190,.cy=95};memcpy(data,&t,sizeof(t));
    put_word(&p,0);put_word(&p,0);put_string(&p,L"Options");put_word(&p,8);put_string(&p,L"MS Sans Serif");
    const WCHAR *labels[]={L"Draw three",L"Standard",L"Timed game",L"Outline dragging"};
    for(unsigned i=0;i<4;++i){
        p=data+(((size_t)(p-data)+3)&~(size_t)3);
        DLGITEMTEMPLATE item={.style=WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,.x=8,.y=(short)(8+i*19),.cx=172,.cy=16,.id=(WORD)(100+i)};
        memcpy(p,&item,sizeof(item));p+=sizeof(item);put_word(&p,0xffff);put_word(&p,0x80);put_string(&p,labels[i]);put_word(&p,0);
    }
    return CreateDialogIndirectParamW(GetModuleHandleW(NULL),(DLGTEMPLATE *)data,NULL,dialog_proc,0);
}
static void dialog_case(void)
{
    HWND w=native_dialog();CHECK(w);HFONT fonts[4];RECT bounds[4];SIZE sizes[4];
    for(unsigned i=0;i<4;++i){HWND child=GetDlgItem(w,100+i);CHECK(child);fonts[i]=(HFONT)SendMessageW(child,WM_GETFONT,0,0);bounds[i]=rect(child,w);sizes[i]=text_size(child,fonts[i]);
        CHECK(sizes[i].cx+26<=bounds[i].right-bounds[i].left && sizes[i].cy<=bounds[i].bottom-bounds[i].top);}
    CHECK(install_frame(w,NULL));
    for(unsigned refresh=0;refresh<3;++refresh)SendMessageW(w,WM_SETTINGCHANGE,0,0);
    for(unsigned i=0;i<4;++i){HWND child=GetDlgItem(w,100+i);CHECK(install_control(child,NULL));unchanged(child,w,system_font(child,fonts[i]),bounds[i],sizes[i]);CHECK(remove_control(child));unchanged(child,w,fonts[i],bounds[i],sizes[i]);}
    printf("FONT LAYOUT DIALOG PASS dpi=%u native DS_SETFONT DLU labels fit; repeated frame font refresh\n",GetDpiForWindow(w));
    CHECK(remove_frame(w) && DestroyWindow(w));
}
static void controls_case(unsigned scale,unsigned bold)
{
    DWORD start=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    HWND root=CreateWindowW(L"STATIC",L"Typography",WS_OVERLAPPEDWINDOW,0,0,900,700,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HFONT font=CreateFontW(-MulDiv(11,(int)scale,96),0,0,0,bold?FW_BOLD:FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Tahoma");
    HFONT next=CreateFontW(-MulDiv(10,(int)scale,96),0,0,0,FW_NORMAL,TRUE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Arial");CHECK(font && next);
    const WCHAR *classes[]={L"BUTTON",L"BUTTON",L"BUTTON",L"STATIC",L"EDIT",L"LISTBOX",L"COMBOBOX"};
    const DWORD styles[]={BS_AUTOCHECKBOX,BS_AUTORADIOBUTTON,BS_GROUPBOX,SS_LEFT,ES_AUTOHSCROLL,LBS_NOINTEGRALHEIGHT,CBS_DROPDOWNLIST};
    const WCHAR *labels[]={L"Outline dragging",L"Standard scoring",L"Draw",L"Timed game",L"Editable text",L"List",L"Combo"};
    for(unsigned i=0;i<7;++i){
        HWND w=CreateWindowW(classes[i],labels[i],WS_CHILD|WS_VISIBLE|styles[i],10,(int)i*80,600,60,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
        SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);RECT before=rect(w,root);SIZE size=text_size(w,font);
        CHECK(size.cx+32<=before.right-before.left && size.cy<=before.bottom-before.top);
        CHECK(i==4?install_edit(w,FALSE,NULL):install_control(w,NULL));unchanged(w,root,system_font(w,font),before,size);
        SendMessageW(w,WM_SETFONT,(WPARAM)next,FALSE);system_font(w,next);
        CHECK(remove_control(w));CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==next);
        CHECK(i==4?install_edit(w,FALSE,NULL):install_control(w,NULL));SendMessageW(w,WM_SETFONT,0,FALSE);CHECK(!SendMessageW(w,WM_GETFONT,0,0));CHECK(remove_control(w));CHECK(!SendMessageW(w,WM_GETFONT,0,0));
        CHECK(DestroyWindow(w));
    }
    CHECK(DestroyWindow(root));CHECK(DeleteObject(next) && DeleteObject(font));
    settle();
    printf("FONT RESOURCE DIAGNOSTIC start=%lu end=%lu\n",start,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS));
    printf("FONT LAYOUT CONTROLS PASS nominal-dpi=%u weight=%u seven classes; app update/null/detach\n",scale,bold?700:400);
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_WITH_ALTERED_SEARCH_PATH);CHECK(dll);
    install_frame=(install_fn)(void *)GetProcAddress(dll,"win16_style_install");remove_frame=(remove_fn)(void *)GetProcAddress(dll,"win16_style_remove");
    install_control=(install_fn)(void *)GetProcAddress(dll,"win16_style_control_install");remove_control=(remove_fn)(void *)GetProcAddress(dll,"win16_style_control_remove");CHECK(install_frame && remove_frame && install_control && remove_control);
    install_edit=(void *)GetProcAddress(dll,"win16_style_edit_install");CHECK(install_edit);
    for(unsigned context=0;context<2;++context){
        CHECK(SetThreadDpiAwarenessContext(context?DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2:DPI_AWARENESS_CONTEXT_UNAWARE));
        /* USER lazily allocates per-class/font caches on first use. Warm every
         * tested variant before checking repeated attach/detach ownership. */
        for(unsigned warm=0;warm<8;++warm){
            dialog_case();for(unsigned dpi=96;dpi<=192;dpi+=48)for(unsigned bold=0;bold<2;++bold)controls_case(dpi,bold);
            settle();
        }
        verify_pixels=TRUE;menu_case();reentrant_case();settle();
        DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        dialog_case();for(unsigned dpi=96;dpi<=192;dpi+=48)for(unsigned bold=0;bold<2;++bold)controls_case(dpi,bold);
        settle();
        DWORD after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        printf("FONT LAYOUT CONTEXT PASS aware=%u GDI=%lu/%lu\n",context,before,after);
        CHECK(after<=before);
        verify_pixels=FALSE;
    }
    CHECK(FreeLibrary(dll));puts("FONT LAYOUT PASS 2 contexts 12 font variants native dialog and 84 control scenarios; repeated after warmup; actual candidate DLL");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
