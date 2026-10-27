#include "ntw16s-dll/attachment.h"
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"HEADER_CONTROL_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static unsigned changing,changed,clicks,owner_draws;
static BOOL cancel;
typedef struct callback_state {
    BOOL (__cdecl *remove)(HWND),(__cdecl *install)(HWND,const void *);
    unsigned mode;
    BOOL armed,removed,installed;
    HFONT font;
} callback_state;
static LRESULT CALLBACK during_query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    callback_state *s=(callback_state*)data;(void)id;
    if(m==HDM_GETITEMW && s->armed){s->armed=FALSE;
        if(s->mode==2){DestroyWindow(w);return FALSE;}
        if(s->mode==1 || s->mode==3){s->removed=s->remove(w);if(s->mode==3)s->installed=s->install(w,NULL);}
        if(s->mode==4){HDITEMW item={.mask=HDI_WIDTH,.cxy=101};SendMessageW(w,HDM_SETITEMW,0,(LPARAM)&item);}
        if(s->mode==5)SendMessageW(w,WM_SETFONT,(WPARAM)s->font,FALSE);
    }
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_NOTIFYFORMAT)return NFR_UNICODE;
    if(m==WM_NOTIFY){NMHDR *n=(NMHDR*)b;
        if(n->code==HDN_ITEMCHANGINGW || n->code==HDN_ITEMCHANGINGA){++changing;return cancel;}
        if(n->code==HDN_ITEMCHANGEDW || n->code==HDN_ITEMCHANGEDA)++changed;
        if(n->code==HDN_ITEMCLICKW || n->code==HDN_ITEMCLICKA)++clicks;
    }
    if(m==WM_DRAWITEM){DRAWITEMSTRUCT *d=(DRAWITEMSTRUCT*)b;++owner_draws;
        SetDCBrushColor(d->hDC,RGB(220,20,40));FillRect(d->hDC,&d->rcItem,GetStockObject(DC_BRUSH));return TRUE;}
    return DefWindowProcW(w,m,a,b);
}
typedef struct capture {HDC dc;HBITMAP bitmap;HGDIOBJ old;DWORD *bits;} capture;
static DWORD paint(capture *s,HWND w)
{
    RECT r={0,0,360,50};FillRect(s->dc,&r,GetStockObject(WHITE_BRUSH));
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)s->dc,PRF_CLIENT);GdiFlush();DWORD hash=2166136261u;
    for(unsigned i=0;i<360*50;++i)hash=(hash^(s->bits[i]&0xffffff))*16777619u;
    return hash;
}
static int width(HWND w)
{HDITEMW item={.mask=HDI_WIDTH};return SendMessageW(w,HDM_GETITEMW,0,(LPARAM)&item)?item.cxy:-1;}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc>=2);BOOL aware=argc>2 && !wcscmp(argv[2],L"aware");if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_LISTVIEW_CLASSES};CHECK(InitCommonControlsEx(&init));
    HMODULE dll=LoadLibraryW(argv[1]);CHECK(dll);
    ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void*)GetProcAddress(dll,"Ntw16sCreationBegin");
    BOOL (__cdecl *end)(ntw16s_creation_scope*,ntw16s_creation_result*)=(void*)GetProcAddress(dll,"Ntw16sCreationEnd");
    BOOL (__cdecl *attached)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");
    BOOL (__cdecl *remove_style)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(begin && end && attached && remove_style);
    BOOL (__cdecl *install_leaf)(HWND,const void*)=(void*)GetProcAddress(dll,"win16_style_control_install");
    BOOL (__cdecl *remove_leaf)(HWND)=(void*)GetProcAddress(dll,"win16_style_control_remove");CHECK(install_leaf && remove_leaf);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW wc={.lpfnWndProc=owner,.hInstance=instance,.lpszClassName=L"HeaderOwner"};CHECK(RegisterClassW(&wc));
    HWND parent=CreateWindowW(wc.lpszClassName,L"header",WS_OVERLAPPEDWINDOW,0,0,650,400,NULL,NULL,instance,NULL);CHECK(parent);ShowWindow(parent,SW_SHOW);
    HFONT font=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,0,0,0,0,L"Arial");CHECK(font);
    HFONT late=CreateFontW(-19,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,0,0,0,0,L"Arial");CHECK(late);
    WNDCLASSEXW native={.cbSize=sizeof(native)};CHECK(GetClassInfoExW(NULL,WC_HEADERW,&native));
    WNDCLASSEXW alias=native;alias.hInstance=instance;alias.lpszClassName=L"THeaderControl";alias.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&alias));
    capture image={0};image.dc=CreateCompatibleDC(NULL);CHECK(image.dc);
    BITMAPINFO bi={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=360,.biHeight=-50,.biPlanes=1,.biBitCount=32}};
    image.bitmap=CreateDIBSection(image.dc,&bi,DIB_RGB_COLORS,(void**)&image.bits,NULL,0);CHECK(image.bitmap);image.old=SelectObject(image.dc,image.bitmap);CHECK(image.old);
    const DWORD styles[]={0,HDS_BUTTONS|HDS_DRAGDROP|HDS_FULLDRAG,HDS_FILTERBAR,HDS_CHECKBOXES};unsigned styled=0,preserved=0;
    for(unsigned k=0;k<ARRAYSIZE(styles);++k){HWND pair[2];RECT rects[2][3];int widths[2],orders[2][3];unsigned notifications[2][3];DWORD hashes[2];BOOL supported=FALSE;
        for(unsigned decorated=0;decorated<2;++decorated){
            ntw16s_creation_scope *scope=decorated?begin(TRUE):NULL;CHECK(!decorated || scope);
            HWND w=pair[decorated]=CreateWindowW(WC_HEADERW,L"",WS_CHILD|WS_VISIBLE|styles[k],80,40,360,26,parent,(HMENU)(UINT_PTR)(20+decorated),instance,NULL);CHECK(w);
            supported=!(GetWindowLongPtrW(w,GWL_STYLE)&(HDS_FILTERBAR|HDS_CHECKBOXES|HDS_OVERFLOW|HDS_HIDDEN));
            if(scope){ntw16s_creation_result result={0};CHECK(end(scope,&result) && attached(w)==supported);}
            SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font);
            for(unsigned i=0;i<3;++i){HDITEMW item={.mask=HDI_WIDTH|HDI_TEXT|HDI_FORMAT|HDI_LPARAM,.cxy=100,.pszText=i?L"Metadata":L"Filename",.fmt=HDF_STRING|(i==1?HDF_RIGHT:i==2?HDF_CENTER:HDF_LEFT),.lParam=1000+i};CHECK(SendMessageW(w,HDM_INSERTITEMW,i,(LPARAM)&item)==(LRESULT)i);}
            changing=changed=clicks=0;cancel=TRUE;HDITEMW item={.mask=HDI_WIDTH,.cxy=130};
            CHECK(!SendMessageW(w,HDM_SETITEMW,0,(LPARAM)&item) && width(w)==100 && changing==1 && !changed);
            cancel=FALSE;CHECK(SendMessageW(w,HDM_SETITEMW,0,(LPARAM)&item) && width(w)==130 && changing==2 && changed==1);
            int order[3]={2,0,1};CHECK(SendMessageW(w,HDM_SETORDERARRAY,3,(LPARAM)order));
            CHECK(SendMessageW(w,HDM_GETORDERARRAY,3,(LPARAM)orders[decorated]));widths[decorated]=width(w);
            for(unsigned i=0;i<3;++i){HDITEMW data={.mask=HDI_LPARAM};CHECK(SendMessageW(w,HDM_GETITEMW,i,(LPARAM)&data) && data.lParam==(LPARAM)(1000+i));CHECK(SendMessageW(w,HDM_GETITEMRECT,i,(LPARAM)&rects[decorated][i]));}
            RECT r=rects[decorated][0];LPARAM point=MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2);
            SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,point);SendMessageW(w,WM_LBUTTONUP,0,point);
            notifications[decorated][0]=changing;notifications[decorated][1]=changed;notifications[decorated][2]=clicks;hashes[decorated]=paint(&image,w);
        }
        CHECK(widths[0]==widths[1] && !memcmp(rects[0],rects[1],sizeof(rects[0])) && !memcmp(orders[0],orders[1],sizeof(orders[0])) && !memcmp(notifications[0],notifications[1],sizeof(notifications[0])));
        if(supported){CHECK(hashes[0]!=hashes[1]);++styled;
            for(unsigned i=0;i<2;++i){
                CallWindowProcW(native.lpfnWndProc,pair[i],WM_SETFONT,(WPARAM)late,FALSE);
                RECT before,after;CHECK(SendMessageW(pair[i],HDM_GETITEMRECT,0,(LPARAM)&before));
                CHECK(paint(&image,pair[i])!=hashes[i] && (HFONT)SendMessageW(pair[i],WM_GETFONT,0,0)==late);
                CHECK(SendMessageW(pair[i],HDM_GETITEMRECT,0,(LPARAM)&after) && EqualRect(&before,&after));
            }
            /* Per-item special formats preserve the entire original header,
             * including real owner-draw callbacks and native sort indicators. */
            const int formats[]={HDF_OWNERDRAW|HDF_STRING,HDF_SORTUP|HDF_STRING,HDF_IMAGE|HDF_STRING};
            for(unsigned f=0;f<ARRAYSIZE(formats);++f){DWORD native_hash[2];for(unsigned i=0;i<2;++i){HDITEMW item={.mask=HDI_FORMAT,.fmt=formats[f]};CHECK(SendMessageW(pair[i],HDM_SETITEMW,0,(LPARAM)&item));native_hash[i]=paint(&image,pair[i]);}CHECK(native_hash[0]==native_hash[1]);}
            CHECK(remove_style(pair[1]) && !attached(pair[1]) && (HFONT)SendMessageW(pair[1],WM_GETFONT,0,0)==late);
        }else {CHECK(hashes[0]==hashes[1]);++preserved;}
        CHECK(DestroyWindow(pair[0]) && DestroyWindow(pair[1]));
    }
    for(unsigned mode=1;mode<=5;++mode){
        ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
        HWND w=CreateWindowW(WC_HEADERW,L"",WS_CHILD|WS_VISIBLE|HDS_BUTTONS,0,0,360,26,parent,NULL,instance,NULL);
        ntw16s_creation_result result={0};CHECK(w && end(scope,&result) && attached(w));
        HDITEMW item={.mask=HDI_WIDTH|HDI_TEXT|HDI_FORMAT,.cxy=100,.pszText=L"Callback lifetime",.fmt=HDF_STRING};
        CHECK(SendMessageW(w,HDM_INSERTITEMW,0,(LPARAM)&item)==0);SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);
        callback_state state={.remove=remove_style,.install=install_leaf,.mode=mode,.armed=TRUE,.font=late};
        CHECK(SetWindowSubclass(w,during_query,1,(DWORD_PTR)&state));paint(&image,w);CHECK(!state.armed);
        if(mode==2){CHECK(!IsWindow(w));continue;}
        if(mode==1 || mode==3){CHECK(state.removed && !attached(w));if(mode==3)CHECK(state.installed && remove_leaf(w));}
        else {CHECK(attached(w));if(mode==4)CHECK(width(w)==101);if(mode==5)CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==late);CHECK(remove_style(w));}
        LOGFONTW valid;CHECK(GetObjectW((HFONT)SendMessageW(w,WM_GETFONT,0,0),sizeof(valid),&valid));
        CHECK(!GetPropW(w,L"ntw16s.private.control-protocol.v1") && DestroyWindow(w));
    }
    ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
    HWND wrapped=CreateWindowW(L"THeaderControl",L"",WS_CHILD|WS_VISIBLE,0,0,360,26,parent,NULL,instance,NULL);ntw16s_creation_result result={0};CHECK(wrapped && end(scope,&result) && attached(wrapped));
    HDITEMW wrapped_item={.mask=HDI_WIDTH|HDI_TEXT|HDI_FORMAT,.cxy=100,.pszText=L"VCL header",.fmt=HDF_STRING};CHECK(SendMessageW(wrapped,HDM_INSERTITEMW,0,(LPARAM)&wrapped_item)==0 && remove_style(wrapped) && DestroyWindow(wrapped));
    /* Exact-name EXE impostor must not inherit the native module's role. */
    wc.lpszClassName=WC_HEADERW;wc.lpfnWndProc=DefWindowProcW;wc.cbWndExtra=native.cbWndExtra;CHECK(RegisterClassW(&wc));
    scope=begin(TRUE);CHECK(scope);HWND fake=CreateWindowW(WC_HEADERW,L"fake",WS_CHILD,0,0,100,20,parent,NULL,instance,NULL);CHECK(fake && end(scope,&result) && !attached(fake) && DestroyWindow(fake));
    CHECK(UnregisterClassW(L"THeaderControl",instance));alias.lpfnWndProc=DefWindowProcW;alias.lpszClassName=L"THeaderControl";CHECK(RegisterClassExW(&alias));scope=begin(TRUE);CHECK(scope);fake=CreateWindowW(L"THeaderControl",L"fake",WS_CHILD,0,0,100,20,parent,NULL,instance,NULL);CHECK(fake && end(scope,&result) && !attached(fake) && DestroyWindow(fake));
    UINT dpi=GetDpiForWindow(parent);CHECK(owner_draws>0);SelectObject(image.dc,image.old);CHECK(DeleteObject(image.bitmap) && DeleteDC(image.dc));CHECK(DestroyWindow(parent) && DeleteObject(font) && DeleteObject(late) && FreeLibrary(dll));
    printf("HEADER_CONTROL_OK bits=%u dpi=%u aware=%u styled=%u preserved=%u native_geometry=1 order=1 data=1 cancel=1 click_native=1 formats_preserved=3 late_native_font=1 callback_cases=5 roles_released=1 impostor=1\n",(unsigned)(sizeof(void*)*8),dpi,aware,styled,preserved);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
