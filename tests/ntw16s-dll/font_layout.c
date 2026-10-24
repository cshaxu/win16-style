/* Actual candidate DLL, native dialog units and application font ownership. */
#include "ntw16s-dll/win16_style.h"
#include "isolated_process.h"
#include <commctrl.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FONT LAYOUT FAIL line=%d error=%lu\n",__LINE__,GetLastError());ExitProcess(1);}}while(0)
typedef BOOL (__cdecl *install_fn)(HWND,const win16_style_palette *);
typedef BOOL (__cdecl *remove_fn)(HWND);
static install_fn install_frame,install_control;
static BOOL (__cdecl *install_edit)(HWND,BOOL,const win16_style_palette *);
static remove_fn remove_frame,remove_control;
static void settle(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}GdiFlush();}
static RECT rect(HWND w,HWND parent)
{RECT r;CHECK(GetWindowRect(w,&r));MapWindowPoints(NULL,parent,(POINT *)&r,2);return r;}
static SIZE text_size(HWND w,HFONT f)
{WCHAR text[128];RECT r={0};HDC dc=GetDC(w);CHECK(dc);HGDIOBJ old=SelectObject(dc,f?f:GetStockObject(SYSTEM_FONT));
 if(!GetWindowTextW(w,text,128))lstrcpyW(text,L"Representative text");
 CHECK(DrawTextW(dc,text,-1,&r,DT_CALCRECT|DT_SINGLELINE));
 SelectObject(dc,old);ReleaseDC(w,dc);return (SIZE){r.right,r.bottom};}
static void unchanged(HWND w,HWND parent,HFONT font,RECT before,SIZE text)
{RECT now=rect(w,parent);SIZE size=text_size(w,font);CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font);
 CHECK(EqualRect(&now,&before) && size.cx==text.cx && size.cy==text.cy);}
static INT_PTR CALLBACK dialog_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)a;(void)b;return m==WM_INITDIALOG;}
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
    for(unsigned i=0;i<4;++i){HWND child=GetDlgItem(w,100+i);CHECK(install_control(child,NULL));unchanged(child,w,fonts[i],bounds[i],sizes[i]);CHECK(remove_control(child));unchanged(child,w,fonts[i],bounds[i],sizes[i]);}
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
        CHECK(i==4?install_edit(w,FALSE,NULL):install_control(w,NULL));unchanged(w,root,font,before,size);
        SendMessageW(w,WM_SETFONT,(WPARAM)next,FALSE);CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==next);
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
        settle();
        DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        dialog_case();for(unsigned dpi=96;dpi<=192;dpi+=48)for(unsigned bold=0;bold<2;++bold)controls_case(dpi,bold);
        settle();
        DWORD after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        printf("FONT LAYOUT CONTEXT PASS aware=%u GDI=%lu/%lu\n",context,before,after);
        CHECK(after<=before);
    }
    CHECK(FreeLibrary(dll));puts("FONT LAYOUT PASS 2 contexts 12 font variants native dialog and 84 control scenarios; repeated after warmup; actual candidate DLL");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
