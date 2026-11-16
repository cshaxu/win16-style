#include "scroll_managed.h"
static BOOL active(const style_scroll_controller *scroll)
{return scroll->resources.frame || scroll->resources.list || scroll->resources.edit;}
BOOL style_scroll_managed_active(HWND window,const style_scroll_managed_ops *ops)
{
    if(!window || GetWindowThreadProcessId(window,NULL)!=GetCurrentThreadId())return FALSE;
    style_scroll_binding binding=ops->lookup(window);
    return binding.instance && !binding.instance->detached && active(binding.scroll);
}
void style_scroll_managed_frame_remove(HWND window,const style_scroll_managed_ops *ops)
{
    style_scroll_binding binding=ops->lookup(window);
    if(binding.instance && binding.scroll->resources.frame)(void)ops->remove(window);
}
BOOL style_scroll_managed_get_info(HWND window,int bar,LPSCROLLINFO info,const style_scroll_managed_ops *ops)
{
    BOOL result=ops->get_info(window,bar,info);DWORD error=GetLastError();
    if(result && (bar==SB_HORZ || bar==SB_VERT) && info && info->cbSize==sizeof(*info) && (info->fMask&SIF_TRACKPOS)){
        style_scroll_binding binding=ops->lookup(window);
        if(binding.instance && active(binding.scroll) && binding.scroll->object==(bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL)){
            if(binding.scroll->reporting)info->nTrackPos=binding.scroll->report;
            else if(binding.scroll->track.active)info->nTrackPos=binding.scroll->track.position;
        }
    }
    SetLastError(error);return result;
}
int style_scroll_managed_set_info(HWND window,int bar,LPCSCROLLINFO info,BOOL redraw,const style_scroll_managed_ops *ops)
{
    style_scroll_binding binding={0};
    if(bar==SB_HORZ || bar==SB_VERT)binding=ops->lookup(window);
    BOOL styled=binding.instance && active(binding.scroll);
    int result=ops->set_info(window,bar,info,styled?FALSE:redraw);DWORD error=GetLastError();
    if(styled && redraw){
        binding=ops->lookup(window);
        if(binding.instance){
            style_instance_retain(binding.instance);
            HDC dc=GetWindowDC(window);
            if(dc){ops->draw(window,binding.instance->context,dc,bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL,bar==SB_VERT);ReleaseDC(window,dc);}
            style_instance_release(binding.instance);
        }else RedrawWindow(window,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
    }
    SetLastError(error);return result;
}
BOOL style_scroll_managed_enable(HWND window,style_scroll_role role,const style_scroll_managed_ops *ops)
{
    LONG_PTR style=role==STYLE_SCROLL_EDIT?GetWindowLongPtrW(window,GWL_STYLE):0;
    style_scroll_binding binding={0};
    if(GetWindowThreadProcessId(window,NULL)==GetCurrentThreadId() && ops->eligible(window,role) &&
        (role!=STYLE_SCROLL_EDIT || (style&ES_MULTILINE)))binding=ops->lookup(window);
    if(!binding.instance){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(role==STYLE_SCROLL_EDIT)binding.scroll->resources.edit=TRUE;
    else binding.scroll->resources.list=TRUE;
    RedrawWindow(window,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);return TRUE;
}
