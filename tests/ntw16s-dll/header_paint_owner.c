#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include "isolated_process.h"
static unsigned allocations,attempts,fail_on;
static void *tracked_calloc(size_t count,size_t size)
{if(++attempts==fail_on)return NULL;void *p=calloc(count,size);if(p)++allocations;return p;}
static void tracked_free(void *p){if(p){if(!allocations)abort();--allocations;free(p);}}
#define calloc tracked_calloc
#define free tracked_free
#include "../../src/ntw16s-dll/header_paint.c"
#undef calloc
#undef free
#define CHECK(x) do{if(!(x)){fprintf(stderr,"HEADER_OWNER_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static WNDPROC original;
static BOOL armed,detached;
static unsigned epoch,mode;
static LRESULT CALLBACK callback(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(armed && m==HDM_GETITEMW){armed=FALSE;
        if(mode==1)detached=TRUE;
        if(mode==2){detached=TRUE;DestroyWindow(w);return FALSE;}
        if(mode==3)++epoch;
    }
    return CallWindowProcW(original,w,m,a,b);
}
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;INITCOMMONCONTROLSEX init={sizeof(init),ICC_LISTVIEW_CLASSES};CHECK(InitCommonControlsEx(&init));
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSEXW cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,WC_HEADERW,&cls));
    original=cls.lpfnWndProc;cls.lpfnWndProc=callback;cls.hInstance=instance;cls.style&=~CS_GLOBALCLASS;cls.lpszClassName=L"HeaderPaintOwner";CHECK(RegisterClassExW(&cls));
    HWND parent=CreateWindowW(L"STATIC",L"header owner",WS_POPUP,0,0,400,100,NULL,NULL,instance,NULL);CHECK(parent);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HBITMAP bitmap=CreateBitmap(360,50,1,32,NULL);CHECK(bitmap);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old);
    HFONT source=(HFONT)GetStockObject(DEFAULT_GUI_FONT);SelectObject(dc,source);SetTextColor(dc,RGB(27,53,71));
    win16_style_palette palette={.face=RGB(192,192,192),.light=RGB(255,255,255),.shadow=RGB(128,128,128),.edge=0};
    unsigned cases=0;
    for(mode=0;mode<4;++mode)for(unsigned fault=0;fault<4;++fault){
        HWND w=CreateWindowW(cls.lpszClassName,L"",WS_CHILD|WS_VISIBLE|HDS_BUTTONS,0,0,360,26,parent,NULL,instance,NULL);CHECK(w);
        for(unsigned i=0;i<2;++i){HDITEMW item={.mask=HDI_WIDTH|HDI_TEXT|HDI_FORMAT,.cxy=100,.pszText=L"Owned header",.fmt=HDF_STRING};CHECK(SendMessageW(w,HDM_INSERTITEMW,i,(LPARAM)&item)==(LRESULT)i);}
        armed=mode!=0;detached=FALSE;epoch=0;attempts=0;fail_on=fault;
        BOOL painted=win16_style_header_paint(w,dc,&palette,source,&detached,&epoch,-1);
        CHECK(painted==(mode==0 && fault==0));CHECK(!allocations);
        CHECK(GetCurrentObject(dc,OBJ_FONT)==source && GetTextColor(dc)==RGB(27,53,71));
        armed=FALSE;if(IsWindow(w))CHECK(DestroyWindow(w));++cases;
    }
    SelectObject(dc,old);CHECK(DeleteObject(bitmap) && DeleteDC(dc) && DestroyWindow(parent));
    printf("HEADER_OWNER_OK bits=%u cases=%u allocations=0 dc_restored=1 callback_remove=1 callback_destroy=1 epoch_change=1 oom_partial=1\n",(unsigned)(sizeof(void*)*8),cases);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
