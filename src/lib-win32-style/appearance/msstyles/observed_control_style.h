#ifndef STYLE_MSSTYLES_OBSERVED_CONTROL_STYLE_H
#define STYLE_MSSTYLES_OBSERVED_CONTROL_STYLE_H
#include "native_scroll_style.h"
#include "lib-win32-style/application/control/observed/control.h"
#include <string.h>

/* Transfer the mapping to the control runtime after projecting parsed data. */
static inline DWORD classic_observed_control_load(HMODULE module,style_observed_control_theme *target)
{
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout(module,CLASSIC_WINDOW_PROVIDER_NONCLIENT,&source);
    if(error){classic_theme_close(&source);return error;}
    const classic_theme *m=&source.model;
    style_relief_palette relief=classic_theme_relief_palette(m);
    *target=(style_observed_control_theme){
        .resources=source.resources,.read_choice_art=classic_choice_art_get,
        .palette=relief,.scroll_palette=classic_theme_native_scroll_palette(m,relief),
        .controls=m->controls,.push=m->push,.choice=m->choice,.label=m->label,.list=m->list,
        .combo=m->combo,.tab=m->tab,.header=m->header,.group=m->group,.frame=m->frame,.calendar=m->calendar,
        .date_button_face=m->date_button_face,.has_date_button_face=m->has_date_button_face,
        .scroll_art=m->scroll_art,.scroll=m->scroll,.scroll_metrics=m->scroll_metrics
    };
    memcpy(target->edit,m->edit,sizeof(target->edit));
    memcpy(target->report,m->report,sizeof(target->report));
    memcpy(target->bars,m->bars,sizeof(target->bars));
    memcpy(target->progress,m->progress,sizeof(target->progress));
    memcpy(target->trackbar,m->trackbar,sizeof(target->trackbar));
    return ERROR_SUCCESS;
}
#endif
