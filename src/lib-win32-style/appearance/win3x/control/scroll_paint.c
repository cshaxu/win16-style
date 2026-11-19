#include "scroll_paint.h"
#include "basic_paint.h"
#include "../../../application/graphics/paint_frame.h"
#include "../resources/scroll_assets.h"
#include "lib-win32-style/application/graphics/present.h"
#include <oleacc.h>
#include "lib-win32-style/application/control/scroll_surface.h"

static BOOL paint_pixels(HWND w,const win16_control_scroll_view *v,HDC dc,LONG object,BOOL vertical)
{
    style_scroll_surface surface;RECT r,first,last,thumb,pixel;int saved;
    if(!dc || !style_scroll_surface_read(w,object,&surface))return FALSE;
    SCROLLBARINFO info=surface.info;r=surface.bar;
    saved=SaveDC(dc);if(!saved)return FALSE;
    if(object!=OBJID_CLIENT)OffsetViewportOrgEx(dc,surface.origin.x,surface.origin.y,NULL);
    IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
    /* Native state remains authoritative; opted-in bars replace geometry
     * below with the same fixed-thumb layout used for hit testing. */
    {static const WORD checks[8]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
        HBITMAP bitmap=CreateBitmap(8,8,1,1,checks);HBRUSH brush=bitmap?CreatePatternBrush(bitmap):NULL;
        SetTextColor(dc,v->palette->face);SetBkColor(dc,v->palette->light);
        if(brush){FillRect(dc,&r,brush);DeleteObject(brush);}
        if(bitmap)DeleteObject(bitmap);
        if(!brush){RestoreDC(dc,saved);return FALSE;}}
    first=last=thumb=r;
    if(vertical){first.bottom=first.top+info.dxyLineButton;last.top=last.bottom-info.dxyLineButton;
        thumb.top=info.xyThumbTop;thumb.bottom=info.xyThumbBottom;}
    else {first.right=first.left+info.dxyLineButton;last.left=last.right-info.dxyLineButton;
        thumb.left=info.xyThumbTop;thumb.right=info.xyThumbBottom;}
    if(object==OBJID_CLIENT || v->scroll->resources.frame || (v->scroll->resources.edit && v->edit_thumb_messages(w,object)) || (v->scroll->resources.list &&
        v->list_axis_owned(w,object))){style_scroll_layout layout;POINT origin;
        if(object==OBJID_CLIENT?v->scroll_layout(w,&layout):v->embedded_scroll_layout(w,object,&layout,&origin)){
            first=layout.first;last=layout.last;thumb=layout.thumb;
            if(v->scroll->track.active && (object==OBJID_CLIENT || object==v->scroll->object) && v->scroll->track.part==STYLE_SCROLL_THUMB){
                if(vertical)OffsetRect(&thumb,0,v->scroll->track.pixel-thumb.top);
                else OffsetRect(&thumb,v->scroll->track.pixel-thumb.left,0);}
            info.xyThumbBottom=layout.thumb_fits?1:0;info.xyThumbTop=0;
            info.rgstate[1]&=~STATE_SYSTEM_PRESSED;info.rgstate[5]&=~STATE_SYSTEM_PRESSED;
            if(v->scroll->track.active && (object==OBJID_CLIENT || object==v->scroll->object) && v->scroll->track.inside){
                if(v->scroll->track.part==STYLE_SCROLL_LINE_BEFORE)info.rgstate[1]|=STATE_SYSTEM_PRESSED;
                if(v->scroll->track.part==STYLE_SCROLL_LINE_AFTER)info.rgstate[5]|=STATE_SYSTEM_PRESSED;}
        }
    }
    style_paint_solid_frame(dc,r,v->palette->edge,1);
    const win16_style_palette *arrow_palette=v->arrow_palette?v->arrow_palette:v->palette;
    win16_control_scroll_arrow(dc,first,(info.rgstate[1]&STATE_SYSTEM_UNAVAILABLE)?(vertical?win16_control_scroll_glyph(WIN16_SCROLL_UP_DISABLED):win16_control_scroll_glyph(WIN16_SCROLL_LEFT_DISABLED)):
        vertical?(info.rgstate[1]&STATE_SYSTEM_PRESSED?win16_control_scroll_glyph(WIN16_SCROLL_UP_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_UP)):
        (info.rgstate[1]&STATE_SYSTEM_PRESSED?win16_control_scroll_glyph(WIN16_SCROLL_LEFT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_LEFT)),arrow_palette);
    win16_control_scroll_arrow(dc,last,(info.rgstate[5]&STATE_SYSTEM_UNAVAILABLE)?(vertical?win16_control_scroll_glyph(WIN16_SCROLL_DOWN_DISABLED):win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_DISABLED)):
        vertical?(info.rgstate[5]&STATE_SYSTEM_PRESSED?win16_control_scroll_glyph(WIN16_SCROLL_DOWN_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_DOWN)):
        (info.rgstate[5]&STATE_SYSTEM_PRESSED?win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_RIGHT)),arrow_palette);
    if(info.xyThumbBottom>info.xyThumbTop){
        SetDCBrushColor(dc,v->palette->face);FillRect(dc,&thumb,(HBRUSH)GetStockObject(DC_BRUSH));
        style_paint_solid_frame(dc,thumb,v->palette->edge,1);InflateRect(&thumb,-1,-1);
        SetDCBrushColor(dc,v->palette->light);pixel=thumb;pixel.bottom=pixel.top+1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
        pixel=thumb;pixel.right=pixel.left+1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
        SetDCBrushColor(dc,v->palette->shadow);pixel=thumb;pixel.top=pixel.bottom-1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
        pixel=thumb;pixel.left=pixel.right-1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    }
    RestoreDC(dc,saved);return TRUE;
}

void win16_control_draw_scroll(HWND w,const win16_control_scroll_view *v,HDC dc,LONG object,BOOL vertical)
{
    style_scroll_surface surface;style_present_buffer buffer;POINT brush,device;BOOL painted;
    if(!dc || !style_scroll_surface_read(w,object,&surface))return;
    RECT r=surface.target;
    if(!style_present_buffer_begin(&buffer,dc,&r,FALSE)){
        (void)style_present_buffer_end(&buffer,FALSE);
        paint_pixels(w,v,dc,object,vertical);return;}
    /* Preserve the checker phase while composing only this bar. One BitBlt
     * prevents the background erase and subsequent thumb draw being visible
     * as separate states. No persistent buffer or redraw timer is needed. */
    device.x=r.left;device.y=r.top;LPtoDP(dc,&device,1);GetBrushOrgEx(dc,&brush);
    SetBrushOrgEx(buffer.memory,brush.x-device.x,brush.y-device.y,NULL);
    painted=paint_pixels(w,v,buffer.memory,object,vertical);
    /* A failed painter has no complete image to publish. Keep the previous
     * target pixels rather than copying an uninitialized temporary bitmap. */
    (void)style_present_buffer_end(&buffer,painted);
}
