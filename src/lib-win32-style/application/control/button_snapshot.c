#include "button_snapshot.h"
#include "../window/window_text.h"
#include "../graphics/icon_size.h"
void style_control_button_snapshot_clear(style_control_button_snapshot *snapshot)
{free(snapshot->text);*snapshot=(style_control_button_snapshot){0};}
BOOL style_control_button_snapshot_read(HWND window,style_instance *instance,const style_control_button_reader *reader,style_control_button_snapshot *snapshot)
{
    *snapshot=(style_control_button_snapshot){.style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE)};
    snapshot->role=reader->role(window,reader->context,snapshot->style);
    if(snapshot->role.label && (snapshot->style&SS_TYPEMASK)==SS_ICON){
        snapshot->icon=(HICON)SendMessageW(window,STM_GETICON,0,0);
        return !instance->detached && IsWindow(window);
    }
    snapshot->state=snapshot->role.label?0:SendMessageW(window,BM_GETSTATE,0,0);
    snapshot->check=snapshot->role.label?0:SendMessageW(window,BM_GETCHECK,0,0);
    snapshot->text=style_window_text_copy(window);if(!snapshot->text)return FALSE;
    snapshot->font=reader->font(window,reader->context);
    if(instance->detached || !IsWindow(window)){style_control_button_snapshot_clear(snapshot);return FALSE;}
    return TRUE;
}
BOOL style_control_button_image_read(HWND window,style_instance *instance,UINT type,style_control_button_image *snapshot)
{
    *snapshot=(style_control_button_image){.type=type};
    int width=0,height=0;
    snapshot->image=(HANDLE)SendMessageW(window,BM_GETIMAGE,type,0);
    if(instance->detached || !IsWindow(window))return FALSE;
    if(!snapshot->image){
        snapshot->listed=(BOOL)SendMessageW(window,BCM_GETIMAGELIST,0,(LPARAM)&snapshot->images);
        if(instance->detached || !IsWindow(window))return FALSE;
        snapshot->listed=snapshot->listed && snapshot->images.himl && snapshot->images.himl!=BCCL_NOGLYPH &&
            ImageList_GetIconSize(snapshot->images.himl,&width,&height);
    }
    if(snapshot->image && type==IMAGE_ICON){
        (void)style_icon_size((HICON)snapshot->image,&width,&height);
    }else if(snapshot->image){
        BITMAP bitmap;
        if(GetObjectW(snapshot->image,sizeof(bitmap),&bitmap)){width=bitmap.bmWidth;height=bitmap.bmHeight;}
    }
    snapshot->size=(SIZE){width,height};
    return TRUE;
}
