#include "native_overlay.h"
void style_control_native_overlay_begin(HWND w,UINT m,WPARAM p,const style_instance *instance,const style_control_native_combo_state *color,
    const style_control_native_combo_state *date,style_control_native_overlay *overlay)
{
    *overlay=(style_control_native_overlay){.clip={NULL,-1}};
    overlay->update=(color->candidate || date->candidate) && m==WM_PAINT && GetUpdateRect(w,&overlay->dirty,FALSE);
    if((color->installed || date->installed) && !instance->detached && m==WM_PRINTCLIENT && p)
        style_clip_snapshot_capture(&overlay->clip,(HDC)p);
}
static void draw_button(HWND w,const style_instance *instance,const style_control_native_combo_state *color,const style_control_native_combo_state *date,
    HDC dc,const RECT *dirty,style_control_native_button_painter paint)
{
    style_control_native_button button;
    if(style_control_native_button_read(w,instance,color->installed?color:date,!color->installed,&button))paint(w,instance,dc,dirty,&button);
}
void style_control_native_overlay_finish(HWND w,UINT m,WPARAM p,const style_instance *instance,const style_control_native_combo_state *color,
    const style_control_native_combo_state *date,style_control_native_overlay *overlay,style_control_native_button_painter paint)
{
    if((color->installed || date->installed) && !instance->detached && ((m==WM_PRINTCLIENT && overlay->clip.kind>=0) || (m==WM_PAINT && overlay->update))){
        DWORD error=GetLastError();HDC dc=m==WM_PRINTCLIENT?(HDC)p:GetDC(w);
        if(dc){
            if(m==WM_PRINTCLIENT){int saved=SaveDC(dc);if(saved){
                if(style_clip_snapshot_apply(&overlay->clip,dc))draw_button(w,instance,color,date,dc,NULL,paint);
                RestoreDC(dc,saved);
            }}else {draw_button(w,instance,color,date,dc,&overlay->dirty,paint);ReleaseDC(w,dc);}
        }
        SetLastError(error);
    }
    style_control_native_overlay_dispose(overlay);
}
