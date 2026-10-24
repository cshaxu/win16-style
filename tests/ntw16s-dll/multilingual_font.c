/* Actual DLL and native text raster: unsupported scripts must retain the
 * application's font, not merely the same nominal character-cell height. */
#include "isolated_process.h"
#include <commctrl.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MULTILINGUAL FONT FAIL line=%d error=%lu\n",__LINE__,GetLastError());ExitProcess(1);}}while(0)
static BOOL (__cdecl *install_control)(HWND,const void *);
static BOOL (__cdecl *install_edit)(HWND,BOOL,const void *);
static BOOL (__cdecl *remove_control)(HWND);
static void settle(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}GdiFlush();}
static HFONT current(HWND w){return (HFONT)SendMessageW(w,WM_GETFONT,0,0);}
static void western(HWND w,HFONT source)
{
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HFONT f=current(w);CHECK(f && f!=source);
    HGDIOBJ old=SelectObject(dc,f);char face[64];CHECK(GetTextFaceA(dc,64,face)>0 && !strncmp(face,"Win16 System ",13));
    SelectObject(dc,old);DeleteDC(dc);
}
static void raster(HFONT font,const WCHAR *text,DWORD *pixels,SIZE *extent,RECT *ink)
{
    HDC dc=CreateCompatibleDC(NULL);DWORD *bits=NULL;
    BITMAPINFO bi={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=512,.biHeight=-64,.biPlanes=1,.biBitCount=32,.biCompression=BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,(void **)&bits,NULL,0);CHECK(dc && bitmap);
    HGDIOBJ oldb=SelectObject(dc,bitmap),oldf=SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));
    RECT bounds={0,0,512,64};CHECK(FillRect(dc,&bounds,GetStockObject(WHITE_BRUSH)));
    CHECK(SetBkMode(dc,TRANSPARENT));SetTextColor(dc,RGB(0,0,0));
    CHECK(GetTextExtentPoint32W(dc,text,(int)wcslen(text),extent));
    CHECK(DrawTextW(dc,text,-1,&bounds,DT_SINGLELINE|DT_NOPREFIX));GdiFlush();
    *ink=(RECT){512,64,-1,-1};
    for(int y=0;y<64;++y)for(int x=0;x<512;++x){DWORD pixel=bits[y*512+x]&0xffffff;pixels[y*512+x]=pixel;
        if(pixel!=0xffffff){ink->left=min(ink->left,x);ink->top=min(ink->top,y);ink->right=max(ink->right,x);ink->bottom=max(ink->bottom,y);}}
    SelectObject(dc,oldf);SelectObject(dc,oldb);DeleteObject(bitmap);DeleteDC(dc);
}
static void native_font(HWND w,HFONT source,const WCHAR *text)
{
    if(current(w)!=source){char cls[32];WCHAR content[128];GetClassNameA(w,cls,32);GetWindowTextW(w,content,128);
        printf("MULTILINGUAL FONT MISMATCH class=%s expected=%p actual=%p text=",cls,(void *)source,(void *)current(w));
        for(const WCHAR *p=content;*p;++p){printf("%04x",*p);}
        puts("");fflush(stdout);}
    CHECK(current(w)==source);DWORD *a=malloc(512*64*4),*b=malloc(512*64*4);CHECK(a && b);
    SIZE x,y;RECT ix,iy;raster(source,text,a,&x,&ix);raster(current(w),text,b,&y,&iy);
    CHECK(x.cx==y.cx && x.cy==y.cy && EqualRect(&ix,&iy) && !memcmp(a,b,512*64*4));
    printf("MULTILINGUAL RASTER dpi=%u advance=%ld height=%ld ink=%ldx%ld\n",GetDpiForWindow(w),x.cx,x.cy,ix.right-ix.left+1,ix.bottom-ix.top+1);
    free(a);free(b);
}
static HFONT font(const WCHAR *face,BYTE charset,int height)
{HFONT f=CreateFontW(-height,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,charset,0,0,NONANTIALIASED_QUALITY,0,face);CHECK(f);
 HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HGDIOBJ old=SelectObject(dc,f);TEXTMETRICW tm;char actual[64];
 CHECK(GetTextMetricsW(dc,&tm) && GetTextFaceA(dc,64,actual)>0);
 printf("MULTILINGUAL SOURCE face=%s requested-charset=%u actual-charset=%u height=%ld\n",actual,charset,tm.tmCharSet,tm.tmHeight);
 SelectObject(dc,old);DeleteDC(dc);return f;}
static HWND child(HWND root,const WCHAR *cls,DWORD style,HFONT f,const WCHAR *text)
{
    HWND w=CreateWindowW(cls,text,WS_CHILD|WS_VISIBLE|style,5,5,450,50,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    SendMessageW(w,WM_SETFONT,(WPARAM)f,FALSE);CHECK(!wcscmp(cls,L"EDIT")?install_edit(w,FALSE,NULL):install_control(w,NULL));return w;
}
static void dispose(HWND w,HFONT latest)
{CHECK(remove_control(w) && current(w)==latest && DestroyWindow(w));}
typedef struct reentry {BOOL armed,entered,destroy;} reentry;
static LRESULT CALLBACK reenter(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    reentry *state=(reentry *)data;(void)id;
    if(m==WM_SETFONT && state->armed && !state->entered){state->entered=TRUE;
        if(state->destroy){CHECK(DestroyWindow(w));return 0;}
        CHECK(remove_control(w));
    }
    return DefSubclassProc(w,m,a,b);
}
static void combo_reentry(HWND root,int height)
{
    HFONT f=font(L"Tahoma",ANSI_CHARSET,height);
    for(unsigned destroy=0;destroy<2;++destroy){
        HWND w=child(root,L"COMBOBOX",CBS_DROPDOWN,f,L"English");
        COMBOBOXINFO info={.cbSize=sizeof(info)};CHECK(GetComboBoxInfo(w,&info));
        reentry state={.destroy=destroy};CHECK(SetWindowSubclass(w,reenter,1,(DWORD_PTR)&state));
        state.armed=TRUE;SetWindowTextW(info.hwndItem,L"\x4e2d\x6587");CHECK(state.entered);
        if(!destroy){CHECK(current(w)==f && RemoveWindowSubclass(w,reenter,1));
            CHECK(SetWindowTextW(info.hwndItem,L"English") && DestroyWindow(w));}
        else CHECK(!IsWindow(w));
    }
    CHECK(DeleteObject(f));puts("MULTILINGUAL COMBO REENTRY PASS remove/destroy during edit observer font update");
}
static void scripts(HWND root,int height)
{
    const struct {const WCHAR *face,*text;BYTE charset;} cases[]={
        {L"SimSun",L"\x521d\x7ea7 999 \x79d2 \x533f\x540d",GB2312_CHARSET},
        {L"MS Gothic",L"\x65e5\x672c\x8a9e 999",SHIFTJIS_CHARSET},
        {L"Malgun Gothic",L"\xd55c\xad6d\xc5b4 999",HANGEUL_CHARSET},
        {L"Tahoma",L"\x0395\x03bb\x03bb\x03b7\x03bd\x03b9\x03ba\x03ac 999",GREEK_CHARSET},
        {L"Tahoma",L"\x0420\x0443\x0441\x0441\x043a\x0438\x0439 999",RUSSIAN_CHARSET},
        {L"Tahoma",L"\x0627\x0644\x0639\x0631\x0628\x064a\x0629 999",ARABIC_CHARSET},
        {L"Tahoma",L"\x05e2\x05d1\x05e8\x05d9\x05ea 999",HEBREW_CHARSET},
        {L"Tahoma",L"\x0e44\x0e17\x0e22 999",THAI_CHARSET}};
    for(unsigned i=0;i<ARRAYSIZE(cases);++i){HFONT f=font(cases[i].face,cases[i].charset,height);
        const WCHAR *classes[]={L"BUTTON",L"STATIC",L"EDIT"};
        for(unsigned k=0;k<ARRAYSIZE(classes);++k){HWND w=child(root,classes[k],0,f,cases[i].text);
            RECT before,after;CHECK(GetWindowRect(w,&before));native_font(w,f,cases[i].text);
            CHECK(SetWindowTextW(w,L"999"));native_font(w,f,L"999");
            CHECK(GetWindowRect(w,&after) && EqualRect(&before,&after));dispose(w,f);}
        CHECK(DeleteObject(f));printf("MULTILINGUAL SCRIPT PASS index=%u requested-charset=%u native-font-and-pixels\n",i,cases[i].charset);
    }
}
static void changes(HWND root,int height)
{
    HFONT f=font(L"Tahoma",ANSI_CHARSET,height),next=font(L"Arial",ANSI_CHARSET,height+1);
    HWND w=child(root,L"BUTTON",0,f,L"Western");western(w,f);
    const WCHAR *mixed[]={L"English \x4e2d\x6587",L"English \x03a9",L"English \x042f",L"e\x0301",L"A\xd83d\xde00",L"A\x200d"};
    for(unsigned i=0;i<ARRAYSIZE(mixed);++i){CHECK(SetWindowTextW(w,mixed[i]));native_font(w,f,mixed[i]);
        CHECK(SetWindowTextW(w,L"Western"));western(w,f);}
    CHECK(SetWindowTextA(w,"caf\xe9"));western(w,f);CHECK(SetWindowTextW(w,L"\x4e2d\x6587"));
    SendMessageW(w,WM_SETFONT,(WPARAM)next,FALSE);native_font(w,next,L"\x4e2d\x6587");dispose(w,next);
    /* An ANSI-created window still receives normalized text via USER reads. */
    w=CreateWindowA("BUTTON","ASCII",WS_CHILD,0,0,400,40,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    SendMessageW(w,WM_SETFONT,(WPARAM)f,FALSE);CHECK(install_control(w,NULL));western(w,f);
    CHECK(SetWindowTextW(w,L"\x4e2d\x6587"));native_font(w,f,L"\x4e2d\x6587");dispose(w,f);
    w=child(root,L"EDIT",ES_AUTOHSCROLL,f,L"English");western(w,f);
    SendMessageW(w,EM_SETSEL,7,7);SendMessageW(w,EM_REPLACESEL,TRUE,(LPARAM)L" \x65e5\x672c\x8a9e");native_font(w,f,L"English \x65e5\x672c\x8a9e");
    WCHAR buffer[128];CHECK(GetWindowTextW(w,buffer,128) && !wcscmp(buffer,L"English \x65e5\x672c\x8a9e"));
    CHECK(SendMessageW(w,EM_UNDO,0,0));western(w,f);CHECK(GetWindowTextW(w,buffer,128) && !wcscmp(buffer,L"English"));
    SendMessageW(w,WM_CHAR,0x4e2d,1);native_font(w,f,L"\x4e2d");CHECK(GetWindowTextW(w,buffer,128) && wcschr(buffer,0x4e2d));
    CHECK(SetWindowTextW(w,L"English"));western(w,f);dispose(w,f);
    /* Check every item, including an unselected foreign-language row. */
    const WCHAR *classes[]={L"LISTBOX",L"COMBOBOX",L"COMBOBOX"};
    for(unsigned i=0;i<3;++i){BOOL combo=i!=0;w=child(root,classes[i],i==2?CBS_DROPDOWN:combo?CBS_DROPDOWNLIST:0,f,L"");
        UINT add=combo?CB_ADDSTRING:LB_ADDSTRING,del=combo?CB_DELETESTRING:LB_DELETESTRING;
        CHECK(SendMessageW(w,add,0,(LPARAM)L"English")>=0);western(w,f);
        CHECK(SendMessageW(w,add,0,(LPARAM)L"\x4e2d\x6587")>=0);native_font(w,f,L"\x4e2d\x6587");
        SendMessageW(w,combo?CB_SETCURSEL:LB_SETCURSEL,0,0);native_font(w,f,L"English");
        CHECK(SendMessageW(w,del,1,0)>=0);western(w,f);
        if(i==2){COMBOBOXINFO info={.cbSize=sizeof(info)};CHECK(GetComboBoxInfo(w,&info) && info.hwndItem);
            CHECK(SetWindowTextW(info.hwndItem,L"English \x4e2d\x6587"));native_font(w,f,L"English \x4e2d\x6587");
            CHECK(SetWindowTextW(info.hwndItem,L"English"));western(w,f);}
        dispose(w,f);}
    /* Failed/budget-exceeding reads conservatively keep the app font. */
    WCHAR *longtext=malloc(20001*sizeof(WCHAR));CHECK(longtext);for(unsigned i=0;i<20000;++i)longtext[i]=L'A';longtext[20000]=0;
    w=child(root,L"STATIC",SS_LEFT,f,L"English");BOOL set=SetWindowTextW(w,longtext);printf("LONG TEXT set=%d length=%d native=%d error=%lu\n",set,GetWindowTextLengthW(w),current(w)==f,GetLastError());CHECK(set);CHECK(current(w)==f);
    CHECK(SetWindowTextW(w,L"English"));western(w,f);dispose(w,f);free(longtext);
    w=child(root,L"BUTTON",0,NULL,L"\x4e2d\x6587");CHECK(!current(w));dispose(w,NULL);
    CHECK(DeleteObject(f) && DeleteObject(next));puts("MULTILINGUAL UPDATE PASS mixed/combining/surrogate ANSI/W edit/undo/char list/combo unselected budget null");
}
static int worker(int argc,WCHAR **argv)
{
    setvbuf(stdout,NULL,_IONBF,0);
    CHECK(argc==2);HMODULE dll=LoadLibraryW(argv[1]);CHECK(dll);
    install_control=(void *)GetProcAddress(dll,"win16_style_control_install");install_edit=(void *)GetProcAddress(dll,"win16_style_edit_install");remove_control=(void *)GetProcAddress(dll,"win16_style_control_remove");CHECK(install_control && install_edit && remove_control);
    for(unsigned context=0;context<2;++context){CHECK(SetThreadDpiAwarenessContext(context?DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2:DPI_AWARENESS_CONTEXT_UNAWARE));
        HWND root=CreateWindowW(L"STATIC",L"Multilingual fonts",WS_OVERLAPPEDWINDOW,0,0,600,400,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
        int height=MulDiv(12,(int)GetDpiForWindow(root),96);
        scripts(root,height);changes(root,height);combo_reentry(root,height);settle();
        DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        for(unsigned loop=0;loop<4;++loop)changes(root,height);
        settle();DWORD after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        printf("MULTILINGUAL RESOURCE PASS dpi=%u GDI=%lu/%lu\n",GetDpiForWindow(root),before,after);CHECK(after<=before);CHECK(DestroyWindow(root));
    }
    /* Leave the native editable combo alive across DLL unload: an orphaned
     * child observer would jump into unmapped code on the following update. */
    HWND root=CreateWindowW(L"STATIC",L"Unload",WS_OVERLAPPEDWINDOW,0,0,500,300,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HFONT f=font(L"Tahoma",ANSI_CHARSET,12);HWND combo=child(root,L"COMBOBOX",CBS_DROPDOWN,f,L"English");
    COMBOBOXINFO info={.cbSize=sizeof(info)};CHECK(GetComboBoxInfo(combo,&info) && remove_control(combo));
    CHECK(FreeLibrary(dll) && !GetModuleHandleW(argv[1]));
    CHECK(SetWindowTextW(info.hwndItem,L"\x4e2d\x6587") && current(combo)==f && DestroyWindow(root) && DeleteObject(f));
    puts("MULTILINGUAL UNLOAD PASS live native combo after DLL unload");
    puts("MULTILINGUAL FONT PASS 2 contexts 8 scripts original glyphs dynamic updates and lifetime");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
