#include "special_paint.h"
#include "basic_paint.h"
#include "scroll_assets.h"
#include "control_protocol.h"
#include <commctrl.h>
#include <vssym32.h>

typedef struct spin_dib {HDC dc;HBITMAP bitmap;HGDIOBJ old;DWORD *pixels;} spin_dib;
static void spin_dib_close(spin_dib *d)
{ SelectObject(d->dc,d->old);DeleteObject(d->bitmap);DeleteDC(d->dc); }
static BOOL spin_dib_open(HDC source,int width,int height,spin_dib *d)
{
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=width;
    info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    d->dc=CreateCompatibleDC(source);if(!d->dc)return FALSE;
    d->bitmap=CreateDIBSection(source,&info,DIB_RGB_COLORS,(void**)&d->pixels,NULL,0);
    if(!d->bitmap){DeleteDC(d->dc);return FALSE;}
    d->old=SelectObject(d->dc,d->bitmap);
    if(!d->old || d->old==HGDI_ERROR){DeleteObject(d->bitmap);DeleteDC(d->dc);return FALSE;}
    int saved=SaveDC(source);BOOL ok=FALSE;
    if(saved){SetViewportOrgEx(source,0,0,NULL);SetWindowOrgEx(source,0,0,NULL);SelectClipRgn(source,NULL);
        ok=BitBlt(d->dc,0,0,width,height,source,0,0,SRCCOPY);GdiFlush();RestoreDC(source,saved);}
    if(!ok){spin_dib_close(d);return FALSE;}return TRUE;
}
BOOL win16_control_spin_snapshot(HDC dc,int width,int height,const POINT pt[2],DWORD *background,BYTE *valid)
{
    spin_dib d={0};if(!spin_dib_open(dc,width,height,&d))return FALSE;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<width;x++){
        int at=y*64+x;background[at]=d.pixels[y*width+x]&0x00ffffff;valid[at]=1;
    }
    spin_dib_close(&d);return TRUE;
}
BOOL win16_control_spin_compose(HWND window,HDC dest,int x,int y,int width,int height,HDC source,
    int source_width,int source_height,const RECT halves[2],const int states[2],const DWORD *background,DWORD *original)
{
    spin_dib d={0};if(!spin_dib_open(source,source_width,source_height,&d))return FALSE;
    for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++)original[j*64+i]=d.pixels[j*source_width+i];
    for(int half=0;half<2;half++)if(!win16_style_bitbtn_face(window,d.dc,&halves[half],NULL,states[half]==3?ODS_SELECTED:0)){spin_dib_close(&d);return FALSE;}
    GdiFlush();for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++){
        int at=j*64+i;if((original[at]&0x00ffffff)!=background[at])d.pixels[j*source_width+i]=original[at];
    }
    BOOL copied=BitBlt(dest,x,y,width,height,d.dc,x,y,SRCCOPY);spin_dib_close(&d);return copied;
}

BOOL win16_control_black_frame(HDC dc,const RECT *bounds,const RECT *clip)
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL drew=FALSE;
    if(!clip || IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)!=ERROR)
        drew=FrameRect(dc,bounds,(HBRUSH)GetStockObject(BLACK_BRUSH));
    RestoreDC(dc,saved);return drew;
}

void win16_control_group_frame(HDC dc,const RECT *bounds,const RECT *clip,const RECT *exclude,void (*drawn)(void *),void *context)
{
    int saved=SaveDC(dc);if(!saved)return;
    if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
    if(exclude)ExcludeClipRect(dc,exclude->left,exclude->top,exclude->right,exclude->bottom);
    if(FrameRect(dc,bounds,(HBRUSH)GetStockObject(BLACK_BRUSH)) && drawn)drawn(context);
    RestoreDC(dc,saved);
}

void win16_control_panel_shadow(HDC dc,const POINT points[3],BOOL (WINAPI *draw)(HDC,const POINT *,int),void (*drawn)(void *),void *context)
{
    int saved=SaveDC(dc);if(!saved)return;
    if(SelectObject(dc,GetStockObject(BLACK_PEN)) && draw(dc,points,3) && drawn)drawn(context);
    RestoreDC(dc,saved);
}

void win16_control_month_fill(HDC dc,const RECT *bounds,const RECT *clip)
{
    int saved=SaveDC(dc);if(!saved)return;
    if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return;}
    win16_style_palette palette;win16_style_default_palette(&palette);
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);SetDCBrushColor(dc,palette.active);
    FillRect(dc,bounds,brush);RestoreDC(dc,saved);
}

BOOL win16_control_cool_grip(HDC dc,const RECT *bounds)
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    RECT r=*bounds;BOOL drew=FALSE;
    if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR){
        FillRect(dc,&r,GetSysColorBrush(COLOR_BTNFACE));
        DrawEdge(dc,&r,BDR_RAISEDINNER,BF_RECT);drew=TRUE;
    }
    RestoreDC(dc,saved);return drew;
}

void win16_style_month_arrow(HDC dc,const RECT *r,const RECT *clip,int part,int state)
{if(!dc || !r || r->right<=r->left || r->bottom<=r->top || r->right-r->left>128 || r->bottom-r->top>128 || r->right-r->left!=r->bottom-r->top)return;
 int saved=SaveDC(dc);if(!saved)return;if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return;}
 win16_style_palette palette;win16_style_default_palette(&palette);const char (*bits)[18]=part==MC_NAVPREV?(state==MCNP_DISABLED?win16_control_scroll_glyph(WIN16_SCROLL_LEFT_DISABLED):state==MCNP_PRESSED?win16_control_scroll_glyph(WIN16_SCROLL_LEFT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_LEFT)):(state==MCNN_DISABLED?win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_DISABLED):state==MCNN_PRESSED?win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_RIGHT));
 win16_control_scroll_arrow(dc,*r,bits,&palette);RestoreDC(dc,saved);
}

/* USER18:0208..027F: scrollbar-class size box shares the inner border
 * and fills with the system scrollbar brush, not a later sizing-grip image.
 * Native creation/alignment and parent size/maximize commands stay native. */
void win16_control_draw_sizebox(HWND w,HDC dc)
{
    RECT outer,client,r;UINT dpi=GetDpiForWindow(w);int saved,cx,cy,bx,by;
    if(!dc || !GetWindowRect(w,&outer) || !GetClientRect(w,&client))return;
    saved=SaveDC(dc);if(!saved)return;
    cx=GetSystemMetricsForDpi(SM_CXVSCROLL,dpi);cy=GetSystemMetricsForDpi(SM_CYHSCROLL,dpi);
    bx=GetSystemMetricsForDpi(SM_CXBORDER,dpi);by=GetSystemMetricsForDpi(SM_CYBORDER,dpi);
    SetRect(&r,outer.right-outer.left-cx+bx,outer.bottom-outer.top-cy+by,
        outer.right-outer.left,outer.bottom-outer.top);
    IntersectClipRect(dc,client.left,client.top,client.right,client.bottom);
    FillRect(dc,&r,GetSysColorBrush(COLOR_SCROLLBAR));RestoreDC(dc,saved);
}
void win16_control_legacy_progress_border(HWND w,const win16_style_palette *palette,HDC target)
{
    RECT outer,client;POINT origin={0,0};DWORD saved_error=GetLastError();
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(!ntw16s_control_protocol_legacy_progress(w) ||
        !(ex&WS_EX_STATICEDGE) || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_CLIENTEDGE)) ||
        (style&(PBS_MARQUEE|WS_HSCROLL|WS_VSCROLL)) ||
        !GetWindowRect(w,&outer) || !GetClientRect(w,&client) || !ClientToScreen(w,&origin))goto done;
    int width=outer.right-outer.left,height=outer.bottom-outer.top;
    int left=origin.x-outer.left,top=origin.y-outer.top;
    if(width<3 || height<3 || left<1 || top<1 || left+client.right>=width || top+client.bottom>=height)goto done;
    RECT bounds={0,0,width,height};HDC dc=target?target:GetWindowDC(w);if(!dc)goto done;
    int saved=SaveDC(dc);if(saved){win16_control_control_frame(dc,bounds,palette->edge,1);RestoreDC(dc,saved);}
    if(!target)ReleaseDC(w,dc);
done: SetLastError(saved_error);
}

void win16_control_vb_data_arrow(HWND w,HDC dc,int x,int y,int width,HDC source,const RECT *client_bounds)
{
    RECT client=*client_bounds;
    int n=-1;const int xs[4]={4,24,client.right-33,client.right-15};
    const int starts[4]={0,18,client.right-35,client.right-18},widths[4]={18,17,17,18};
    int base_y=(client.bottom-13)/2;
    for(int i=0;i<4;++i)if(x==xs[i]+(y-base_y) && width==(i==0 || i==3?11:8)){n=i;break;}
    if(n<0)return;
    BOOL disabled=!IsWindowEnabled(w),pressed=(y==base_y+1);const char (*bits)[18]=n<2?
        (disabled?win16_control_scroll_glyph(WIN16_SCROLL_LEFT_DISABLED):pressed?win16_control_scroll_glyph(WIN16_SCROLL_LEFT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_LEFT)):
        (disabled?win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_DISABLED):pressed?win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_RIGHT));
    win16_style_palette palette;win16_style_default_palette(&palette);
    COLORREF colors[4]={palette.edge,palette.light,palette.shadow,palette.face};DWORD pixels[18*29];
    int extra=client.bottom-17,mid=extra/2;
    for(int j=0;j<client.bottom;++j){int by=j<2?j:j>=client.bottom-2?j-extra:j<2+mid?2:j<15+mid?j-mid:14;
        for(int i=0;i<widths[n];++i){int bx=i<2?i:i>=widths[n]-2?i-(widths[n]-17):i;if(bx>14)bx=14;
            COLORREF c=colors[bits[by][bx]-'0'];pixels[j*widths[n]+i]=((DWORD)GetRValue(c)<<16)|((DWORD)GetGValue(c)<<8)|GetBValue(c);}}
    if(n==0 || n==3){int column=n==0?1:8;unsigned mark=0;
        for(int j=0;j<13;++j)for(int i=0;i<2;++i)if(GetPixel(source,column+i,j)==RGB(0,0,0)){
            int px=x+column+i-starts[n],py=y+j;if(px>=0 && px<widths[n] && py>=0 && py<client.bottom){COLORREF c=disabled?palette.shadow:palette.edge;
                pixels[py*widths[n]+px]=((DWORD)GetRValue(c)<<16)|((DWORD)GetGValue(c)<<8)|GetBValue(c);++mark;}}
        if(mark!=22)return;
    }
    BITMAPINFO info={0};info.bmiHeader=(BITMAPINFOHEADER){.biSize=sizeof(BITMAPINFOHEADER),.biWidth=widths[n],.biHeight=-client.bottom,.biPlanes=1,.biBitCount=32,.biCompression=BI_RGB};
    (void)SetDIBitsToDevice(dc,starts[n],0,widths[n],client.bottom,0,0,0,client.bottom,pixels,&info,DIB_RGB_COLORS);
}

BOOL win16_style_bitbtn_face(HWND w,HDC dc,const RECT *rect,const RECT *clip,UINT state)
{
    if(!w || !dc || !rect || rect->right-rect->left<4 || rect->bottom-rect->top<4)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return FALSE;}
    win16_style_palette palette;win16_style_default_palette(&palette);int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    SetDCBrushColor(dc,palette.face);FillRect(dc,rect,(HBRUSH)GetStockObject(DC_BRUSH));
    (void)win16_control_button_border(dc,*rect,&palette,u,u,(state&ODS_SELECTED)!=0);RestoreDC(dc,saved);return TRUE;
}

BOOL win16_control_graphical_face(HWND w,HDC dc,const RECT *rect,UINT flags,const win16_style_palette *palette)
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    /* This native primitive owns its face before VB draws the caption/image.
     * The native call has already preserved its return/rect/error semantics. */
    SetDCBrushColor(dc,palette->face);FillRect(dc,rect,(HBRUSH)GetStockObject(DC_BRUSH));
    (void)win16_control_button_border(dc,*rect,palette,u,u,(flags&(DFCS_PUSHED|DFCS_CHECKED))!=0);
    RestoreDC(dc,saved);return TRUE;
}
