#include "ntw16s-dll/attachment.h"
#include <commctrl.h>
#include <imm.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTROL_EXTENSION_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static unsigned clicks,early_queries,protocol_queries;
static void settle(void)
{MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}GdiFlush();}
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_COMMAND && HIWORD(a)==BN_CLICKED)++clicks;return DefWindowProcW(w,m,a,b);}
/* A deliberately cooperative application-owned public button protocol. It
 * keeps its registered/real class, unlike USER native superclass fixtures.
 * Bad variants expose a dialog flag alone, or an invalid ideal size. */
static LRESULT CALLBACK protocol_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_CREATE){SetWindowLongPtrW(w,0,1);return 0;}
    if(m==WM_GETDLGCODE || m==BCM_GETIDEALSIZE || m==BM_GETSTATE || m==BM_GETCHECK){
        ++protocol_queries;
        if(!GetWindowLongPtrW(w,0))++early_queries;
        if(m==WM_GETDLGCODE)return DLGC_BUTTON;
        if(m==BCM_GETIDEALSIZE){int id=GetDlgCtrlID(w);if(id==2)return FALSE;
            SIZE *s=(SIZE*)b;s->cx=id==3?-1:70;s->cy=24;return TRUE;}
        return 0;
    }
    return DefWindowProcW(w,m,a,b);
}
typedef struct capture {HDC dc;HBITMAP bitmap;HGDIOBJ old;DWORD *bits;} capture;
static capture image;
static BOOL capture_start(void)
{
    BITMAPINFO info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=200,.biHeight=-80,.biPlanes=1,.biBitCount=32}};
    image.dc=CreateCompatibleDC(NULL);
    image.bitmap=CreateDIBSection(image.dc,&info,DIB_RGB_COLORS,(void**)&image.bits,NULL,0);
    if(!image.dc || !image.bitmap)return FALSE;
    image.old=SelectObject(image.dc,image.bitmap);return image.old!=NULL;
}
static DWORD paint(HWND w,BOOL whole)
{
    RECT r={0,0,200,80};FillRect(image.dc,&r,GetStockObject(WHITE_BRUSH));
    SendMessageW(w,whole?WM_PRINT:WM_PRINTCLIENT,(WPARAM)image.dc,PRF_CLIENT|PRF_NONCLIENT|PRF_ERASEBKGND);
    GdiFlush();DWORD hash=2166136261u;
    for(unsigned i=0;i<200*80;++i)hash=(hash^image.bits[i])*16777619u;
    return hash;
}
static unsigned color_count(DWORD color)
{unsigned n=0;for(unsigned i=0;i<200*80;++i)if((image.bits[i]&0xffffff)==color)++n;return n;}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc>=2);CHECK(ImmDisableIME(0));BOOL protocol_only=argc>2 && !wcscmp(argv[2],L"protocol");
    HMODULE dll=LoadLibraryW(argv[1]);CHECK(dll);
    ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void*)GetProcAddress(dll,"Ntw16sCreationBegin");
    BOOL (__cdecl *end)(ntw16s_creation_scope*,ntw16s_creation_result*)=(void*)GetProcAddress(dll,"Ntw16sCreationEnd");
    BOOL (__cdecl *attached)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");
    BOOL (__cdecl *remove_style)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");
    CHECK(begin && end && attached && remove_style);
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW wc={.lpfnWndProc=parent_proc,.hInstance=instance,.lpszClassName=L"ExtensionOwner"};
    CHECK(RegisterClassW(&wc));WNDCLASSEXW native={.cbSize=sizeof(native)};CHECK(GetClassInfoExW(NULL,L"BUTTON",&native));
    wc.lpfnWndProc=protocol_proc;wc.lpszClassName=L"TButton";wc.cbWndExtra=max((int)sizeof(LONG_PTR),native.cbWndExtra);CHECK(RegisterClassW(&wc));
    HWND parent=CreateWindowW(L"ExtensionOwner",L"extensions",WS_OVERLAPPEDWINDOW,0,0,400,300,NULL,NULL,instance,NULL);CHECK(parent);
    ShowWindow(parent,SW_SHOW);CHECK(capture_start());
    DWORD before_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    if(!protocol_only){
        BITMAPINFO bi={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=16,.biHeight=-16,.biPlanes=1,.biBitCount=32}};DWORD *bits;
        HBITMAP bitmap=CreateDIBSection(image.dc,&bi,DIB_RGB_COLORS,(void**)&bits,NULL,0);CHECK(bitmap);
        for(unsigned i=0;i<256;++i)bits[i]=0x00e01020;
        HBITMAP mask=CreateBitmap(16,16,1,1,NULL);CHECK(mask);
        HDC maskdc=CreateCompatibleDC(NULL);CHECK(maskdc);HGDIOBJ oldmask=SelectObject(maskdc,mask);PatBlt(maskdc,0,0,16,16,BLACKNESS);SelectObject(maskdc,oldmask);CHECK(DeleteDC(maskdc));
        ICONINFO ii={.fIcon=TRUE,.hbmMask=mask,.hbmColor=bitmap};HICON icon=CreateIconIndirect(&ii);CHECK(icon);
        static const DWORD styles[]={BS_PUSHBUTTON|BS_ICON,BS_DEFPUSHBUTTON|BS_BITMAP,BS_AUTOCHECKBOX|BS_PUSHLIKE};
        for(unsigned k=0;k<3;++k){
            ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
            HWND w=CreateWindowW(L"BUTTON",L"toggle",WS_CHILD|WS_VISIBLE|WS_TABSTOP|styles[k],10,10,160,50,parent,(HMENU)(UINT_PTR)(20+k),instance,NULL);
            ntw16s_creation_result result={0};CHECK(end(scope,&result) && w && attached(w));
            HANDLE handle=k? (HANDLE)bitmap:(HANDLE)icon;UINT type=k?IMAGE_BITMAP:IMAGE_ICON;
            if(k<2)SendMessageW(w,BM_SETIMAGE,type,(LPARAM)handle);
            paint(w,FALSE);CHECK(color_count(0xc0c0c0)>300);
            if(k<2){CHECK(color_count(0xe01020)==256);CHECK((HANDLE)SendMessageW(w,BM_GETIMAGE,type,0)==handle);}
            unsigned oldclicks=clicks;SendMessageW(w,BM_CLICK,0,0);CHECK(clicks==oldclicks+1);
            if(k==2){CHECK(SendMessageW(w,BM_GETCHECK,0,0)==BST_CHECKED);DWORD checked=paint(w,FALSE);
                SendMessageW(w,BM_SETCHECK,BST_UNCHECKED,0);CHECK(paint(w,FALSE)!=checked);}
            SetFocus(w);oldclicks=clicks;SendMessageW(w,WM_KEYDOWN,VK_SPACE,0);SendMessageW(w,WM_KEYUP,VK_SPACE,0);CHECK(clicks==oldclicks+1);
            EnableWindow(w,FALSE);oldclicks=clicks;SendMessageW(w,BM_CLICK,0,0);unsigned disabled_notices=clicks-oldclicks;paint(w,FALSE);EnableWindow(w,TRUE);
            CHECK(remove_style(w) && !attached(w));if(k<2)CHECK((HANDLE)SendMessageW(w,BM_GETIMAGE,type,0)==handle);
            EnableWindow(w,FALSE);oldclicks=clicks;SendMessageW(w,BM_CLICK,0,0);CHECK(clicks-oldclicks==disabled_notices);
            CHECK(DestroyWindow(w));
        }
        CHECK(DestroyIcon(icon) && DeleteObject(mask) && DeleteObject(bitmap));
        HIMAGELIST list=ImageList_Create(32,32,ILC_COLOR32,5,0);CHECK(list);
        static const DWORD colors[]={0xe01020,0x10c030,0x1040e0,0xe0b010,0xc010d0};
        bi.bmiHeader.biWidth=32;bi.bmiHeader.biHeight=-32;
        bitmap=CreateDIBSection(image.dc,&bi,DIB_RGB_COLORS,(void**)&bits,NULL,0);CHECK(bitmap);
        for(unsigned i=0;i<5;++i){
            for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x)
                bits[y*32+x]=x>=8 && x<24 && y>=8 && y<24?0xff000000|colors[i]:0;
            CHECK(ImageList_Add(list,bitmap,NULL)==(int)i);}
        CHECK(DeleteObject(bitmap));
        DWORD native_colors[5];RECT clear={0,0,200,80};
        for(unsigned i=0;i<5;++i){FillRect(image.dc,&clear,GetStockObject(WHITE_BRUSH));
            CHECK(ImageList_DrawEx(list,(int)i,image.dc,50,0,32,32,CLR_NONE,CLR_NONE,ILD_NORMAL));GdiFlush();
            native_colors[i]=image.bits[16*200+66]&0xffffff;CHECK(native_colors[i]!=0xffffff);}
        ntw16s_creation_scope *image_scope=begin(TRUE);CHECK(image_scope);
        HWND image_button=CreateWindowW(L"BUTTON",L"",WS_CHILD|WS_VISIBLE|BS_ICON,10,10,29,26,parent,NULL,instance,NULL);
        ntw16s_creation_result image_result={0};CHECK(end(image_scope,&image_result) && image_button && attached(image_button));
        BUTTON_IMAGELIST images={.himl=list,.uAlign=BUTTON_IMAGELIST_ALIGN_CENTER};CHECK(SendMessageW(image_button,BCM_SETIMAGELIST,0,(LPARAM)&images));
        paint(image_button,FALSE);CHECK(color_count(native_colors[0])==256);
        SendMessageW(image_button,BM_SETSTATE,TRUE,0);paint(image_button,FALSE);CHECK(color_count(native_colors[2])==256);
        SendMessageW(image_button,BM_SETSTATE,FALSE,0);EnableWindow(image_button,FALSE);paint(image_button,FALSE);CHECK(color_count(native_colors[3])==256);
        EnableWindow(image_button,TRUE);SendMessageW(image_button,BM_SETSTYLE,BS_DEFPUSHBUTTON|BS_ICON,TRUE);paint(image_button,FALSE);CHECK(color_count(native_colors[4])==256);
        for(int i=4;i>0;--i)CHECK(ImageList_Remove(list,i));
        DWORD normal=paint(image_button,FALSE);CHECK(color_count(native_colors[0])==256);
        EnableWindow(image_button,FALSE);CHECK(paint(image_button,FALSE)!=normal);
        CHECK(remove_style(image_button));BUTTON_IMAGELIST retained={0};CHECK(SendMessageW(image_button,BCM_GETIMAGELIST,0,(LPARAM)&retained) && retained.himl==list);
        CHECK(DestroyWindow(image_button) && ImageList_GetImageCount(list)==1 && ImageList_Destroy(list));
        puts("CONTROL_EXTENSION_IMAGELIST_OK normal_pressed_disabled_default=1 single=1 borrowed=1 native_pixels=256");
        ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
        HWND edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"12345",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,10,80,160,30,parent,NULL,instance,NULL);
        ntw16s_creation_result result={0};CHECK(end(scope,&result) && edit && attached(edit));RECT client,after;GetClientRect(edit,&client);
        DWORD styled=paint(edit,TRUE);SendMessageW(edit,EM_SETSEL,1,4);SendMessageW(edit,EM_REPLACESEL,TRUE,(LPARAM)L"8");
        WCHAR text[40];GetWindowTextW(edit,text,40);CHECK(!wcscmp(text,L"185"));CHECK(SendMessageW(edit,EM_UNDO,0,0));
        GetWindowTextW(edit,text,40);CHECK(!wcscmp(text,L"12345"));CHECK(remove_style(edit));GetClientRect(edit,&after);
        CHECK(EqualRect(&client,&after) && styled!=paint(edit,TRUE));CHECK(DestroyWindow(edit));
        puts("CONTROL_EXTENSION_NATIVE_OK images=2 borrowed=1 pushlike=1 native_click_keyboard=1 edit_client_undo=1");
    }
    for(unsigned round=0;round<20;++round)for(unsigned variant=1;variant<=3;++variant){
        ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
        HWND w=CreateWindowW(L"TButton",L"verified protocol",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,10,10,160,50,parent,(HMENU)(UINT_PTR)variant,instance,NULL);
        ntw16s_creation_result result={0};CHECK(end(scope,&result) && w);
        WCHAR real[40];CHECK(RealGetWindowClassW(w,real,40) && !wcscmp(real,L"TButton"));
        printf("PROTOCOL_RESULT round=%u variant=%u attached=%u scope_error=%lu initialized=%lld extra=%lu prop=%p queries=%u owner=%p exe=%p\n",round,variant,attached(w),result.failure,(long long)GetWindowLongPtrW(w,0),(unsigned long)GetClassLongPtrW(w,GCL_CBWNDEXTRA),(void*)GetPropW(w,L"ntw16s.private.control-protocol.v1"),protocol_queries,(void*)GetClassLongPtrW(w,GCLP_HMODULE),(void*)instance);fflush(stdout);
        CHECK(attached(w)==(variant==1));
        if(variant==1){paint(w,FALSE);CHECK(color_count(0xc0c0c0)>300);
            if(round&1)CHECK(remove_style(w) && !attached(w) && !GetPropW(w,L"ntw16s.private.control-protocol.v1"));}
        CHECK(DestroyWindow(w));settle();
    }
    DWORD after_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    /* USER/IME may initialize thread-owned helper windows asynchronously.
     * Report the process total; exact protocol-node ownership is separately
     * checked by control-protocol-owner, not inferred from this noisy total. */
    printf("EXTENSION_RESOURCES cold=%lu after=%lu process_total_diagnostic_only=1\n",before_gdi,after_gdi);
    CHECK(!early_queries);
    CHECK(DestroyWindow(parent));CHECK(SelectObject(image.dc,image.old));CHECK(DeleteObject(image.bitmap) && DeleteDC(image.dc));
    CHECK(FreeLibrary(dll));printf("CONTROL_EXTENSION_OK bits=%u protocol_cases=60 early_queries=0 owned_images_freed=1 dpi=%u\n",(unsigned)(sizeof(void*)*8),GetDpiForSystem());return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
