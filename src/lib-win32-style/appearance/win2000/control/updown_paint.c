/* Win2000 updown.c uses the native scroll-arrow frame for both halves.
 * Keep those native pixels and their state, and map the USER palette roles
 * in the painted area. An odd split can leave a one-pixel unpainted seam. */
#include "updown_paint.h"
#include "../common/paint.h"
#include "lib-win32-style/application/graphics/bitmap.h"
#include <commctrl.h>

BOOL win2k_updown_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    DWORD allowed=UDS_WRAP|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ALIGNLEFT|UDS_AUTOBUDDY|UDS_ARROWKEYS|
        UDS_HORZ|UDS_NOTHOUSANDS|UDS_HOTTRACK;
    return !(style&(0xffffu&~allowed)) && !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

static DWORD dib_color(COLORREF color)
{
    return ((DWORD)GetRValue(color)<<16)|((DWORD)GetGValue(color)<<8)|GetBValue(color);
}

BOOL win2k_updown_paint(HWND window,style_instance *instance,HDC target,const win2k_palette *p)
{
    const COLORREF key=RGB(253,11,239);
    const int indices[]={COLOR_BTNFACE,COLOR_3DLIGHT,COLOR_3DSHADOW,COLOR_3DDKSHADOW,COLOR_3DHIGHLIGHT,COLOR_BTNTEXT};
    const COLORREF colors[]={p->face,p->light,p->shadow,p->dark,p->highlight,p->text};
    DWORD input[ARRAYSIZE(indices)],output[ARRAYSIZE(indices)];
    for(unsigned i=0;i<ARRAYSIZE(indices);++i){
        input[i]=dib_color(GetSysColor(indices[i]));
        output[i]=dib_color(colors[i]);
        if(input[i]==dib_color(key))return FALSE;
        for(unsigned j=0;j<i;++j)if(input[i]==input[j] && output[i]!=output[j])return FALSE;
    }
    RECT client;
    if(!target || !GetClientRect(window,&client) || client.right<1 || client.bottom<1 ||
        client.right>512 || client.bottom>512)return FALSE;
    style_bitmap surface={0};
    SIZE size={client.right,client.bottom};
    if(!style_bitmap_ensure(&surface,target,size))return FALSE;
    BOOL painted=FALSE;
    if(!style_paint_color(surface.dc,&client,key))goto done;
    DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)surface.dc,PRF_CLIENT);
    if(instance->detached || !GdiFlush())goto done;
    DWORD *pixels=(DWORD *)surface.pixels;
    size_t count=(size_t)size.cx*(size_t)size.cy;
    BOOL mapped=FALSE;
    for(size_t n=0;n<count;++n){
        DWORD pixel=pixels[n]&0xffffffu;
        for(unsigned i=0;i<ARRAYSIZE(input);++i){
            if(pixel==input[i]){pixels[n]=output[i];mapped=TRUE;break;}
        }
    }
    if(mapped)painted=TransparentBlt(target,0,0,size.cx,size.cy,surface.dc,0,0,size.cx,size.cy,key);
done:
    style_bitmap_dispose(&surface);
    return painted;
}
