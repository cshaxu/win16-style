#include "ntw16s-dll/attachment.h"
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include <stddef.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TAB_CONTROL_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static WNDPROC native;
static unsigned changing,changed,early,query_count;
static BOOL cancel,armed,destroy_query;
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_NOTIFY){NMHDR *n=(NMHDR*)b;
        if(n->code==TCN_SELCHANGING){++changing;return cancel;}
        if(n->code==TCN_SELCHANGE)++changed;
    }
    return DefWindowProcW(w,m,a,b);
}
static LRESULT CALLBACK superclass(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==TCM_GETITEMCOUNT || m==TCM_ADJUSTRECT || m==TCM_HITTEST){
        ++query_count;if(!GetPropW(w,L"initialized"))++early;
    }
    if(armed && destroy_query && m==TCM_GETITEMW){armed=FALSE;DestroyWindow(w);return FALSE;}
    LRESULT result=CallWindowProcA(native,w,m,a,b);
    if(m==WM_CREATE)SetPropW(w,L"initialized",(HANDLE)1);
    if(m==WM_NCDESTROY)RemovePropW(w,L"initialized");
    return result;
}
typedef struct snapshot {HDC dc;HBITMAP bitmap;HGDIOBJ old;DWORD *bits;} snapshot;
static BOOL start(snapshot *s)
{
    BITMAPINFO bi={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=300,.biHeight=-200,.biPlanes=1,.biBitCount=32}};
    s->dc=CreateCompatibleDC(NULL);s->bitmap=CreateDIBSection(s->dc,&bi,DIB_RGB_COLORS,(void**)&s->bits,NULL,0);
    if(!s->dc || !s->bitmap)return FALSE;
    s->old=SelectObject(s->dc,s->bitmap);return s->old!=NULL;
}
static DWORD paint(snapshot *s,HWND w)
{
    RECT r={0,0,300,200};SetDCBrushColor(s->dc,RGB(29,53,71));FillRect(s->dc,&r,GetStockObject(DC_BRUSH));
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)s->dc,PRF_CLIENT);GdiFlush();DWORD hash=2166136261u;
    for(unsigned i=0;i<300*200;++i)hash=(hash^(s->bits[i]&0xffffff))*16777619u;
    return hash;
}
static void click(HWND w,unsigned index)
{
    RECT r={0};SendMessageW(w,TCM_GETITEMRECT,index,(LPARAM)&r);LPARAM p=MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2);
    SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,p);SendMessageW(w,WM_LBUTTONUP,0,p);
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc>=2);BOOL aware=argc>2 && !wcscmp(argv[2],L"aware");
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_TAB_CLASSES};CHECK(InitCommonControlsEx(&init));
    HMODULE dll=LoadLibraryW(argv[1]);CHECK(dll);
    ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void*)GetProcAddress(dll,"Ntw16sCreationBegin");
    BOOL (__cdecl *end)(ntw16s_creation_scope*,ntw16s_creation_result*)=(void*)GetProcAddress(dll,"Ntw16sCreationEnd");
    BOOL (__cdecl *attached)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");
    BOOL (__cdecl *remove_style)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(begin && end && attached && remove_style);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW wc={.lpfnWndProc=owner,.hInstance=instance,.lpszClassName=L"TabFixtureOwner"};CHECK(RegisterClassW(&wc));
    WNDCLASSEXA cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExA(NULL,WC_TABCONTROLA,&cls));native=cls.lpfnWndProc;
    cls.lpfnWndProc=superclass;cls.hInstance=instance;cls.lpszClassName="TPageControl";cls.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExA(&cls));
    cls.lpszClassName="TTabControl";CHECK(RegisterClassExA(&cls));
    HWND parent=CreateWindowW(wc.lpszClassName,L"tabs",WS_OVERLAPPEDWINDOW,0,0,700,450,NULL,NULL,instance,NULL);CHECK(parent);ShowWindow(parent,SW_SHOW);
    HFONT font=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,0,0,0,0,L"Arial");CHECK(font);
    snapshot image={0};CHECK(start(&image));
    HIMAGELIST icons=ImageList_Create(12,12,ILC_COLOR32,1,0);CHECK(icons);
    HBITMAP icon=CreateCompatibleBitmap(image.dc,12,12);CHECK(icon);HDC icon_dc=CreateCompatibleDC(image.dc);CHECK(icon_dc);
    HGDIOBJ previous_icon=SelectObject(icon_dc,icon);RECT icon_rect={0,0,12,12};SetDCBrushColor(icon_dc,RGB(220,20,40));
    FillRect(icon_dc,&icon_rect,GetStockObject(DC_BRUSH));SelectObject(icon_dc,previous_icon);
    CHECK(ImageList_Add(icons,icon,NULL)==0 && DeleteObject(icon) && DeleteDC(icon_dc));
    const DWORD styles[]={0,TCS_MULTILINE|TCS_HOTTRACK,TCS_BUTTONS,TCS_OWNERDRAWFIXED,TCS_BOTTOM,TCS_VERTICAL|TCS_MULTILINE,TCS_FIXEDWIDTH|TCS_FORCEICONLEFT};
    const char *classes[]={WC_TABCONTROLA,"TPageControl","TTabControl"};
    unsigned styled=0,preserved=0,dpi=GetDpiForWindow(parent);
    for(unsigned clone=0;clone<ARRAYSIZE(classes);++clone)for(unsigned k=0;k<ARRAYSIZE(styles);++k){
        HWND pair[2];RECT item_rects[2],pages[2];DWORD hashes[2];LRESULT keyboard[2];
        for(unsigned decorated=0;decorated<2;++decorated){
            ntw16s_creation_scope *scope=decorated?begin(TRUE):NULL;CHECK(!decorated || scope);
            pair[decorated]=CreateWindowA(classes[clone],"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|styles[k],0,0,300,200,parent,(HMENU)(UINT_PTR)(10+decorated),instance,NULL);
            HWND w=pair[decorated];CHECK(w);if(scope){ntw16s_creation_result r={0};CHECK(end(scope,&r));CHECK(attached(w)==(k<2));}
            CHECK(SendMessageW(w,TCM_SETITEMEXTRA,64,0));SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);
            SendMessageW(w,TCM_SETIMAGELIST,0,(LPARAM)icons);
            HFONT actual=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
            printf("TAB_FONT clone=%u style=%u decorated=%u actual=%p source=%p\n",clone,k,decorated,(void*)actual,(void*)font);
            /* Native vertical tabs return a private rotated font. */
            if(!(styles[k]&TCS_VERTICAL))CHECK(actual==font);
            for(unsigned i=0;i<4;++i){
                union {TCITEMW item;BYTE bytes[sizeof(TCITEMW)+64];} data={0};TCITEMW *t=&data.item;
                t->mask=TCIF_TEXT|TCIF_PARAM|TCIF_IMAGE;t->pszText=i?L"More tabs":L"Native geometry";t->iImage=0;
                memset(data.bytes+offsetof(TCITEMW,lParam),(int)(40+i),64);
                CHECK(SendMessageW(w,TCM_INSERTITEMW,i,(LPARAM)t)==(LRESULT)i);
                memset(data.bytes+offsetof(TCITEMW,lParam),0,64);t->mask=TCIF_PARAM;
                CHECK(SendMessageW(w,TCM_GETITEMW,i,(LPARAM)t));
                for(unsigned j=0;j<64;++j)CHECK(data.bytes[offsetof(TCITEMW,lParam)+j]==40+i);
            }
            CHECK(SendMessageW(w,TCM_GETITEMRECT,0,(LPARAM)&item_rects[decorated]));pages[decorated]=(RECT){0,0,300,200};
            SendMessageW(w,TCM_ADJUSTRECT,FALSE,(LPARAM)&pages[decorated]);
            changing=changed=0;SendMessageW(w,TCM_SETCURSEL,0,0);CHECK(!changing && !changed);
            cancel=TRUE;click(w,1);CHECK(SendMessageW(w,TCM_GETCURSEL,0,0)==0 && changing==1 && changed==0);
            cancel=FALSE;click(w,1);CHECK(SendMessageW(w,TCM_GETCURSEL,0,0)==1 && changing==2 && changed==1);
            SetFocus(w);SendMessageW(w,WM_KEYDOWN,VK_RIGHT,0);keyboard[decorated]=SendMessageW(w,TCM_GETCURSEL,0,0);
            SetFocus(parent);hashes[decorated]=paint(&image,w);
            if(decorated && k<2){
                CHECK(image.bits[100*300+150]==0x001d3547); /* Page interior stays application-owned. */
                unsigned colored=0;for(unsigned p=0;p<300*60;++p)colored+=(image.bits[p]&0xffffff)==0x00dc1428;
                CHECK(colored>=12*12 && (HIMAGELIST)SendMessageW(w,TCM_GETIMAGELIST,0,0)==icons);
                CHECK(hashes[0]!=hashes[1]);++styled;
            }else if(decorated)++preserved;
        }
        CHECK(EqualRect(&item_rects[0],&item_rects[1]) && EqualRect(&pages[0],&pages[1]) && keyboard[0]==keyboard[1]);
        if(k>=2)CHECK(hashes[0]==hashes[1]);
        if(k<2){CHECK(remove_style(pair[1]) && !attached(pair[1]));CHECK((HFONT)SendMessageW(pair[1],WM_GETFONT,0,0)==font);}
        CHECK(DestroyWindow(pair[0]) && DestroyWindow(pair[1]));
    }
    /* VCL can call its saved native procedure directly after creation,
     * bypassing WM_SETFONT subclasses. The paint boundary must refresh its
     * borrowed font without changing native geometry or font ownership. */
    ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
    HWND w=CreateWindowA("TPageControl","",WS_CHILD|WS_VISIBLE,0,0,300,200,parent,NULL,instance,NULL);ntw16s_creation_result result={0};CHECK(w && end(scope,&result) && attached(w));
    TCITEMW item={.mask=TCIF_TEXT,.pszText=L"Destroy during query"};CHECK(SendMessageW(w,TCM_INSERTITEMW,0,(LPARAM)&item)==0);
    SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);DWORD first=paint(&image,w);
    HFONT late=CreateFontW(-19,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,0,0,0,0,L"Arial");CHECK(late);
    CallWindowProcA(native,w,WM_SETFONT,(WPARAM)late,FALSE);
    RECT before,after;CHECK(SendMessageW(w,TCM_GETITEMRECT,0,(LPARAM)&before));
    CHECK(paint(&image,w)!=first && (HFONT)SendMessageW(w,WM_GETFONT,0,0)==late);
    CHECK(SendMessageW(w,TCM_GETITEMRECT,0,(LPARAM)&after) && EqualRect(&before,&after));
    /* A getter can destroy a real superclass while our painter is active. */
    destroy_query=armed=TRUE;paint(&image,w);CHECK(!IsWindow(w) && !armed);
    CHECK(DeleteObject(late));
    CHECK(!early && query_count);
    CHECK(UnregisterClassA("TPageControl",instance));CHECK(UnregisterClassA("TTabControl",instance));cls.lpfnWndProc=DefWindowProcA;cls.lpszClassName="TPageControl";CHECK(RegisterClassExA(&cls));cls.lpszClassName="TTabControl";CHECK(RegisterClassExA(&cls));
    scope=begin(TRUE);CHECK(scope);w=CreateWindowA("TPageControl","fake",WS_CHILD,0,0,100,30,parent,NULL,instance,NULL);
    CHECK(w && end(scope,&result) && !attached(w) && DestroyWindow(w));
    scope=begin(TRUE);CHECK(scope);w=CreateWindowA("TTabControl","fake",WS_CHILD,0,0,100,30,parent,NULL,instance,NULL);
    CHECK(w && end(scope,&result) && !attached(w) && DestroyWindow(w));
    SelectObject(image.dc,image.old);CHECK(DeleteObject(image.bitmap) && DeleteDC(image.dc));
    CHECK(DestroyWindow(parent) && DeleteObject(font) && ImageList_Destroy(icons) && FreeLibrary(dll));
    printf("TAB_CONTROL_OK bits=%u dpi=%u aware=%u styled=%u preserved=%u native_geometry=1 item_data64=1 cancel=1 keyboard=1 late_native_font=1 destroy_query=1 early_queries=0 impostor=1\n",(unsigned)(sizeof(void*)*8),dpi,aware,styled,preserved);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
