#include "combo_snapshot.h"
#include "../window/window_text.h"
void style_control_combo_snapshot_clear(style_control_combo_snapshot *snapshot)
{free(snapshot->text);*snapshot=(style_control_combo_snapshot){0};}
BOOL style_control_combo_snapshot_read(HWND window,style_instance *instance,const style_control_combo_reader *reader,style_control_combo_snapshot *snapshot)
{
    *snapshot=(style_control_combo_snapshot){.info.cbSize=sizeof(snapshot->info)};
    if(!reader->get_info(window,&snapshot->info))return FALSE;
    snapshot->text=style_window_text_copy(window);if(!snapshot->text)return FALSE;
    snapshot->font=reader->font(window,reader->context);
    snapshot->pressed=SendMessageW(window,CB_GETDROPPEDSTATE,0,0)!=0;
    snapshot->simple=(GetWindowLongPtrW(window,GWL_STYLE)&3)==CBS_SIMPLE;
    snapshot->editable=(GetWindowLongPtrW(window,GWL_STYLE)&3)!=CBS_DROPDOWNLIST;
    if(instance->detached){style_control_combo_snapshot_clear(snapshot);return FALSE;}
    return TRUE;
}
