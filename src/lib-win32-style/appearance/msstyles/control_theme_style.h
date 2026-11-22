#ifndef STYLE_MSSTYLES_CONTROL_THEME_STYLE_H
#define STYLE_MSSTYLES_CONTROL_THEME_STYLE_H
#include "native_scroll_style.h"
#include "lib-win32-style/application/control/control_theme.h"
#include "lib-win32-style/application/control/choice_glyph.h"
#include <string.h>

/* Transfer the mapping to the control runtime after projecting parsed data. */
static inline DWORD classic_control_theme_load(HMODULE module,enum classic_window_provider layout,style_control_theme *target)
{
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout(module,layout,&source);
    if(error){classic_theme_close(&source);return error;}
    const classic_theme *m=&source.model;
    *target=(style_control_theme){
        .resources=source.resources,.read_choice_art=classic_choice_art_get,
        .controls=m->controls,.push=m->push,.choice=m->choice,.label=m->label,.list=m->list,
        .combo=m->combo,.tab=m->tab,.header=m->header,.group=m->group,.frame=m->frame,.calendar=m->calendar,
        .date_button_face=m->date_button_face,.has_date_button_face=m->has_date_button_face,
        .scroll_art=m->scroll_art,.scroll=m->scroll,.scroll_metrics=m->scroll_metrics,
        .scroll_arrow=m->scroll_arrow,.combo_arrow=m->combo_arrow
    };
    if(source.resources){
        if(layout==CLASSIC_WINDOW_PROVIDER_FULL_CLIENT){
            style_window_palette_default(&target->window_palette);
            classic_theme_window_palette(&target->window_palette,m);
        }else{
            target->palette=classic_theme_relief_palette(m);
            target->scroll_palette=classic_theme_native_scroll_palette(m,target->palette);
        }
    }
    memcpy(target->edit,m->edit,sizeof(target->edit));
    memcpy(target->report,m->report,sizeof(target->report));
    memcpy(target->bars,m->bars,sizeof(target->bars));
    memcpy(target->progress,m->progress,sizeof(target->progress));
    memcpy(target->trackbar,m->trackbar,sizeof(target->trackbar));
    return ERROR_SUCCESS;
}
static inline BOOL classic_control_checklist_read(HMODULE module,UINT flags,style_check_glyph *glyph)
{
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout(module,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source);
    if(error || !source.resources){classic_theme_close(&source);return FALSE;}
    const classic_choice_surface *entry=classic_choice_entry(&source.model.choice,FALSE,
        flags&DFCS_CHECKED?BST_CHECKED:BST_UNCHECKED,FALSE,flags&DFCS_PUSHED?BST_PUSHED:0);
    BOOL decoded=classic_choice_art_get(source.resources,entry->image,&glyph->art)!=NULL;
    const classic_surface_colors *colors=classic_choice_color(entry);
    glyph->ink=colors?colors->edge:GetSysColor(COLOR_WINDOWTEXT);
    glyph->paper=colors?colors->face:GetSysColor(COLOR_WINDOW);
    classic_theme_close(&source);
    return decoded;
}
#endif
