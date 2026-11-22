#ifndef STYLE_MSSTYLES_CONTROL_THEME_STYLE_H
#define STYLE_MSSTYLES_CONTROL_THEME_STYLE_H
#include "native_scroll_style.h"
#include "raster_font.h"
#include "lib-win32-style/application/control/control_class.h"
#include "lib-win32-style/application/control/control_state.h"
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
static inline COLORREF classic_control_month_selected_color(const classic_theme *theme,COLORREF fallback)
{
    if(theme->calendar.present&(1u<<CLASSIC_CALENDAR_SELECTED))
        return theme->calendar.values[CLASSIC_CALENDAR_SELECTED];
    return theme->list.present?theme->list.selection:fallback;
}

static inline BOOL classic_control_checklist_read(HMODULE module,UINT flags,style_check_glyph *glyph)
{
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout(module,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source);
    if(error || !source.resources){classic_theme_close(&source);return FALSE;}
    const classic_choice_surface *entry=classic_choice_entry(&source.model.choice,FALSE,
        flags&DFCS_CHECKED?BST_CHECKED:BST_UNCHECKED,FALSE,flags&DFCS_PUSHED?BST_PUSHED:0);
    BOOL decoded=classic_choice_art_get(source.resources,entry->image,entry->fallback_image,&glyph->art)!=NULL;
    const classic_surface_colors *colors=classic_choice_color(entry);
    glyph->ink=colors?colors->edge:GetSysColor(COLOR_WINDOWTEXT);
    glyph->paper=colors?colors->face:GetSysColor(COLOR_WINDOW);
    classic_theme_close(&source);
    return decoded;
}

static inline HFONT classic_control_outline_acquire_font(HWND w,HFONT source,BOOL native,unsigned points)
{
    if(!points)return source;
    BOOL vb=!native && (style_control_vb_frame(w) || style_control_vb_edit(w) || style_control_vb_button(w) || style_control_vb_combo(w) || style_control_vb_list(w));
    return vb?style_raster_font_acquire_stroke_fit(source,points):style_raster_font_acquire(source,points);
}
static inline const style_control_fonts *classic_control_outline_font_operations(void)
{
    static const style_control_fonts fonts={
        style_raster_font_source,classic_control_outline_acquire_font,style_raster_font_retain,style_raster_font_release
    };
    return &fonts;
}

#endif
