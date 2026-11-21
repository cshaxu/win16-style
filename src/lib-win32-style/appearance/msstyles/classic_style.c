#include "classic_style.h"
#include "v4_endian.h"
#include "lib-win32-style/application/menu/row_text.h"
#include <stdbool.h>
#include <string.h>

enum {
    header_size=16, record_size=8, group_header_size=8, slot_size=4,
    button_group_type=1, scroll_metrics_type=3, scroll_metrics_size=4,
    menu_metrics_type=7, menu_metrics_size=8,
    dialog_metrics_type=8, dialog_metrics_size=36,
    dialog_policy_type=9, dialog_policy_size=4,
    caption_presentation_type=10, caption_presentation_legacy_size=2, caption_presentation_size=4,
    border_presentation_type=11, border_presentation_legacy_size=10, border_presentation_size=17,
    menu_text_type=12, menu_text_size=2, separator_type=13, separator_size=1,
    thumb_type=14, thumb_size=1, push_type=15, push_size=1,
    control_lines_type=16, control_lines_size=2, progress_type=17, progress_size=6,
    trackbar_type=18, trackbar_size=1, header_frame_type=19, header_frame_size=1,
    tab_frame_type=20, tab_frame_size=1, stock_dialog_type=21, stock_dialog_size=3,
    menu_bar_type=22, menu_bar_size=2, native_edge_type=23, native_edge_size=1,
    caption_icon_type=24, caption_icon_size=1, choice_fallback_type=25, choice_fallback_size=1,
    choice_metrics_type=26, choice_metrics_size=4, updown_type=27, updown_size=3, updown_mode_type=28, updown_mode_size=1, menu_row_type=29, menu_row_size=2, control_font_type=30, control_font_size=1,
    scroll_repeat_type=31, scroll_repeat_size=4, scroll_drag_type=32, scroll_drag_size=3, mdi_inactive_type=33, mdi_inactive_size=1, frame_scroll_type=34, frame_scroll_size=1,
    tab_text_type=35, tab_text_size=1, max_bar_padding=64, required_flag=1
};

_Static_assert(offsetof(style_border_presentation,geometry)+offsetof(style_frame_geometry,scroll_shared_units)==
    border_presentation_size,"border resource layout");
_Static_assert(offsetof(style_caption_presentation,icon_mode)==caption_presentation_size,"caption resource layout");
_Static_assert(sizeof(style_progress_layout)==progress_size,"progress record layout");
_Static_assert(sizeof(style_choice_metrics)==choice_metrics_size,"choice metrics record layout");
_Static_assert(sizeof(style_scroll_drag)==scroll_drag_size,"scroll drag record layout");
_Static_assert(sizeof(style_arrow_button_metrics)==updown_size,"updown record layout");
_Static_assert(header_size+13*record_size+group_header_size+slot_size*CLASSIC_BUTTON_CLOSE+
    scroll_metrics_size+menu_metrics_size+dialog_metrics_size+dialog_policy_size+
    caption_presentation_size+border_presentation_size+menu_text_size+separator_size+thumb_size+push_size+control_lines_size+
    progress_size+trackbar_size+header_frame_size+tab_frame_size+11*record_size+stock_dialog_size+menu_bar_size+native_edge_size+caption_icon_size+choice_fallback_size+choice_metrics_size+updown_size+updown_mode_size+record_size+menu_row_size+record_size+control_font_size+record_size+scroll_repeat_size+record_size+scroll_drag_size+record_size+mdi_inactive_size+record_size+frame_scroll_size+record_size+tab_text_size==
    CLASSIC_STYLE_RESOURCE_CAPACITY,"classic resource capacity");

static uint16_t read_half(const unsigned char *bytes)
{
    return (uint16_t)(bytes[0]|(bytes[1]<<8));
}

static bool has_updown(style_arrow_button_metrics metrics)
{return metrics.outline || metrics.padding || metrics.crop;}

static bool valid_updown(style_arrow_button_metrics metrics)
{
    return has_updown(metrics) && metrics.outline<=4 && metrics.padding<=4 &&
        metrics.crop<(STYLE_SCROLL_ARROW_SIDE+1)/2;
}

static bool valid_choice_metrics(style_choice_metrics metrics)
{
    return metrics.width && metrics.width<=64 && metrics.height && metrics.height<=64 &&
        metrics.gap<=64 && !(metrics.flags&~(STYLE_CHOICE_DPI|STYLE_CHOICE_FONT_GAP|STYLE_CHOICE_ALIGN)) &&
        (!(metrics.flags&STYLE_CHOICE_FONT_GAP) || !metrics.gap);
}

static bool valid_progress(style_progress_layout layout)
{
    return (layout.flags&STYLE_PROGRESS_DEFINED) && !(layout.flags&~31u) &&
        layout.inset<=16 && layout.clearance<=16 && layout.gap<=16 &&
        layout.block_part>0 && layout.block_part<=layout.block_whole && layout.block_whole<=16;
}

static void write_half(unsigned char *bytes,uint16_t value)
{
    bytes[0]=(unsigned char)value;
    bytes[1]=(unsigned char)(value>>8);
}

static bool valid_group(const classic_button_group *group)
{
    if(!group->count || group->count>CLASSIC_BUTTON_CLOSE ||
       (group->layout && (!(group->layout&CLASSIC_BUTTON_LAYOUT_EXPLICIT) ||
        (group->layout&~(CLASSIC_BUTTON_LAYOUT_EXPLICIT|CLASSIC_BUTTON_LAYOUT_CLIP|CLASSIC_BUTTON_LAYOUT_FILL)))))
        return false;
    unsigned seen=0;
    unsigned side=CLASSIC_BUTTON_LEFT;
    for(unsigned i=0;i<group->count;++i){
        classic_button_slot slot=group->slots[i];
        if(slot.role<CLASSIC_BUTTON_SYSTEM || slot.role>CLASSIC_BUTTON_CLOSE ||
           slot.side<side || slot.side>CLASSIC_BUTTON_RIGHT || (seen&(1u<<slot.role)))return false;
        side=slot.side;
        seen|=1u<<slot.role;
    }
    return true;
}

static bool read_group(const unsigned char *bytes,size_t length,uint16_t minor,classic_button_group *group)
{
    if(length<group_header_size || (minor<28 && read_half(bytes+6)))return false;
    *group=(classic_button_group){
        .count=read_half(bytes),.left_margin=read_half(bytes+2),.right_margin=read_half(bytes+4),.layout=read_half(bytes+6)
    };
    if(group->count>CLASSIC_BUTTON_CLOSE || length!=group_header_size+slot_size*(unsigned)group->count)
        return false;
    bytes+=group_header_size;
    for(unsigned i=0;i<group->count;++i,bytes+=slot_size)
        group->slots[i]=(classic_button_slot){bytes[0],bytes[1],read_half(bytes+2)};
    return valid_group(group);
}

static bool valid_scroll(classic_scroll_metrics scroll)
{
    if(scroll.source==CLASSIC_SCROLL_EXTENT_FIXED)return scroll.logical_extent>0;
    return (scroll.source==CLASSIC_SCROLL_EXTENT_CROSS_AXIS ||
        scroll.source==CLASSIC_SCROLL_EXTENT_SYSTEM) && !scroll.logical_extent;
}

static bool valid_menu_text(uint16_t flags)
{
    const unsigned mask=STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_STIPPLE_DISABLED|
        STYLE_MENU_TEXT_SHORTCUT_RIGHT|STYLE_MENU_TEXT_BOLD_DEFAULT|STYLE_MENU_TEXT_CENTER_BAR|STYLE_MENU_COMMAND_GLYPHS;
    return (flags&STYLE_MENU_TEXT_DEFINED) && !(flags&~mask);
}

static bool valid_mdi_buttons(unsigned flags)
{
    return !flags || ((flags&CLASSIC_MDI_DEFINED) && !(flags&~(CLASSIC_MDI_DEFINED|CLASSIC_MDI_ALL)));
}

static bool valid_menu(classic_menu_metrics menu)
{
    return menu.check_column>=16 && menu.check_column<=256 && menu.shortcut_gap<=256 &&
        menu.arrow_column>=11 && menu.arrow_column<=256 &&
        menu.plain_column>0 && menu.plain_column<=menu.arrow_column && menu.bar_padding<=max_bar_padding && valid_mdi_buttons(menu.mdi_buttons) &&
        valid_mdi_buttons(menu.mdi_inactive) &&
        (!menu.text_flags || valid_menu_text(menu.text_flags)) &&
        menu.row_minimum<=64 && menu.row_padding<=16 && (menu.row_minimum || !menu.row_padding);
}

static bool valid_dialog(classic_dialog_metrics dialog)
{
    classic_file_metrics file=dialog.file;
    return file.compact_width && file.compact_height && file.places_width>=file.compact_width &&
        file.places_height>=file.compact_height && file.places_width<=4096 && file.places_height<=4096 &&
        file.client_dlu_width && file.client_dlu_width<=2048 && file.client_dlu_height &&
        file.client_dlu_height<=2048 && file.gap<=128 && file.bottom_reserve<=128 &&
        dialog.print_width && dialog.print_width<=4096 && dialog.print_full_height &&
        dialog.print_large_height>=dialog.print_full_height && dialog.print_large_height<=4096 &&
        dialog.color_width && dialog.color_width<=2048 && dialog.color_height && dialog.color_height<=2048 &&
        dialog.find_width && dialog.find_width<=2048 && dialog.find_classic_height &&
        dialog.find_classic_height<=dialog.find_native_height && dialog.find_native_height<=2048 &&
        dialog.font_width && dialog.font_width<=2048 && dialog.font_height && dialog.font_height<=2048;
}

static bool valid_policy(classic_dialog_policy policy)
{
    return policy.window_provider>=CLASSIC_WINDOW_PROVIDER_FULL_CLIENT &&
        policy.window_provider<=CLASSIC_WINDOW_PROVIDER_NONCLIENT &&
        policy.file<=CLASSIC_FILE_EXPLORER_LAYOUT && policy.folder<=CLASSIC_FOLDER_OBSERVED_NATIVE &&
        policy.print<=CLASSIC_PRINT_GENERAL_PAGE && policy.color<=CLASSIC_STOCK_OBSERVED_LAYOUT &&
        policy.font<=CLASSIC_STOCK_OBSERVED_LAYOUT && policy.find<=CLASSIC_STOCK_OBSERVED_LAYOUT;
}

static bool valid_caption(style_caption_presentation caption)
{
    return (caption.text_mode==STYLE_CAPTION_TEXT_CENTER_CLIP ||
        caption.text_mode==STYLE_CAPTION_TEXT_LEFT_ELLIPSIS) &&
        caption.separator<=STYLE_CAPTION_SEPARATOR_AFTER &&
        caption.button_source<=STYLE_CAPTION_BUTTON_SYSTEM &&
        caption.button_outline<=STYLE_CAPTION_BUTTON_EDGE_OUTLINE && caption.icon_mode<=STYLE_CAPTION_ICON_NATIVE &&
        (!caption.button_outline || caption.button_source==STYLE_CAPTION_BUTTON_INDEXED);
}

static bool valid_stroke(style_border_stroke stroke)
{
    return stroke.anchor<=STYLE_BORDER_INNER &&
        stroke.leading<=STYLE_BORDER_TITLE && stroke.trailing<=STYLE_BORDER_TITLE;
}

static bool valid_border(style_border_presentation border)
{
    const style_frame_geometry g=border.geometry;
    return (border.fill==STYLE_BORDER_CLIPPED || border.fill==STYLE_BORDER_PERIMETER) &&
        border.dialog_fill<=STYLE_BORDER_TITLE &&
        valid_stroke(border.outer) && valid_stroke(border.inner) &&
        (border.dialog_inner<=STYLE_BORDER_TITLE || border.dialog_inner==STYLE_BORDER_KEEP) &&
        (border.divider<=STYLE_BORDER_TITLE || border.divider==STYLE_BORDER_KEEP) &&
        g.scale<=STYLE_FRAME_MONITOR_PIXELS && g.separator_units<=1 && g.scroll_shared_units<=1 &&
        (g.scale || !(g.sizeable || g.fixed || g.dialog || g.line || g.caption_floor || g.separator_units || g.scroll_shared_units));
}

enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group,classic_scroll_metrics *scroll,classic_menu_metrics *menu,classic_dialog_metrics *dialog,
    classic_dialog_policy *policy,style_caption_presentation *caption,style_border_presentation *border,classic_control_presentation *controls)
{
    if(!bytes || !group || length<header_size || memcmp(bytes,"CSTY",4) ||
       msstyles_v4_read_word(bytes+8)!=length)return CLASSIC_STYLE_INVALID;
    if(read_half(bytes+4)!=1)return CLASSIC_STYLE_VERSION;
    uint32_t count=msstyles_v4_read_word(bytes+12);
    const unsigned char *record=bytes+header_size;
    size_t remaining=length-header_size;
    classic_button_group value={0};
    classic_scroll_metrics metrics={0};
    style_scroll_thumb thumb=STYLE_SCROLL_THUMB_DEFAULT;
    classic_menu_metrics columns={0};
    uint16_t menu_text=0,bar_padding=0;
    uint8_t mdi_buttons=0,row_minimum=0,row_padding=0,mdi_inactive=0;
    classic_dialog_metrics dimensions={0};
    classic_dialog_policy routes={0};
    bool stock_dialog_seen=false;
    style_caption_presentation presentation={0};
    style_border_presentation strokes={0};
    uint8_t scroll_shared=0;
    classic_control_presentation control_presentation={0};
    for(uint32_t i=0;i<count;++i){
        if(remaining<record_size)return CLASSIC_STYLE_INVALID;
        uint16_t type=read_half(record),flags=read_half(record+2);
        size_t payload=msstyles_v4_read_word(record+4);
        if((flags&~required_flag) || payload>remaining-record_size)return CLASSIC_STYLE_INVALID;
        if(type==button_group_type){
            if(value.count || !read_group(record+record_size,payload,read_half(bytes+6),&value))return CLASSIC_STYLE_INVALID;
        }else if(type==scroll_metrics_type){
            if(metrics.source || payload!=scroll_metrics_size)return CLASSIC_STYLE_INVALID;
            metrics.source=read_half(record+record_size);
            metrics.logical_extent=read_half(record+record_size+2);
            if(!valid_scroll(metrics))return CLASSIC_STYLE_INVALID;
        }else if(type==thumb_type){
            if(thumb || flags || payload!=thumb_size)return CLASSIC_STYLE_INVALID;
            thumb=record[record_size];
            if(thumb!=STYLE_SCROLL_THUMB_FIXED && thumb!=STYLE_SCROLL_THUMB_PAGE)
                return CLASSIC_STYLE_INVALID;
        }else if(type==menu_metrics_type){
            if(columns.check_column || payload!=menu_metrics_size)return CLASSIC_STYLE_INVALID;
            const unsigned char *data=record+record_size;
            columns=(classic_menu_metrics){read_half(data),read_half(data+2),read_half(data+4),read_half(data+6),0,0,0,0,0,0};
            if(!valid_menu(columns))return CLASSIC_STYLE_INVALID;
        }else if(type==menu_text_type){
            if(menu_text || payload!=menu_text_size || flags)return CLASSIC_STYLE_INVALID;
            menu_text=read_half(record+record_size);
            if(!valid_menu_text(menu_text))return CLASSIC_STYLE_INVALID;
        }else if(type==menu_bar_type){
            if(bar_padding || mdi_buttons || (payload!=1 && payload!=menu_bar_size) || flags)
                return CLASSIC_STYLE_INVALID;
            bar_padding=record[record_size];
            if(payload==menu_bar_size)mdi_buttons=record[record_size+1];
            if((!bar_padding && !mdi_buttons) || bar_padding>max_bar_padding || !valid_mdi_buttons(mdi_buttons))
                return CLASSIC_STYLE_INVALID;
        }else if(type==dialog_metrics_type){
            if(dimensions.print_width || payload!=dialog_metrics_size)return CLASSIC_STYLE_INVALID;
            const unsigned char *data=record+record_size;
            dimensions=(classic_dialog_metrics){
                {read_half(data),read_half(data+2),read_half(data+4),read_half(data+6),
                 read_half(data+8),read_half(data+10),read_half(data+12),read_half(data+14)},
                read_half(data+16),read_half(data+18),read_half(data+20),
                read_half(data+22),read_half(data+24),read_half(data+26),read_half(data+28),
                read_half(data+30),read_half(data+32),read_half(data+34)};
            if(!valid_dialog(dimensions))return CLASSIC_STYLE_INVALID;
        }else if(type==dialog_policy_type){
            if(routes.window_provider || payload!=dialog_policy_size || flags!=required_flag)
                return CLASSIC_STYLE_INVALID;
            const unsigned char *data=record+record_size;
            routes.window_provider=data[0];routes.file=data[1];routes.folder=data[2];routes.print=data[3];
            if(!valid_policy(routes))return CLASSIC_STYLE_INVALID;
        }else if(type==stock_dialog_type){
            if(stock_dialog_seen || flags || payload!=stock_dialog_size)return CLASSIC_STYLE_INVALID;
            stock_dialog_seen=true;
            const unsigned char *data=record+record_size;
            routes.color=data[0];routes.font=data[1];routes.find=data[2];
            if(routes.color>CLASSIC_STOCK_OBSERVED_LAYOUT || routes.font>CLASSIC_STOCK_OBSERVED_LAYOUT ||
               routes.find>CLASSIC_STOCK_OBSERVED_LAYOUT)return CLASSIC_STYLE_INVALID;
        }else if(type==caption_presentation_type){
            if(presentation.text_mode || (payload!=caption_presentation_size &&
               payload!=caption_presentation_legacy_size))return CLASSIC_STYLE_INVALID;
            memcpy(&presentation,record+record_size,payload);
            if(!valid_caption(presentation))return CLASSIC_STYLE_INVALID;
        }else if(type==border_presentation_type){
            if(strokes.fill || (payload!=border_presentation_size &&
               payload!=border_presentation_legacy_size))return CLASSIC_STYLE_INVALID;
            memcpy(&strokes,record+record_size,payload);
            if(!valid_border(strokes))return CLASSIC_STYLE_INVALID;
        }else if(type==separator_type){
            if(control_presentation.separator || flags || payload!=separator_size)return CLASSIC_STYLE_INVALID;
            control_presentation.separator=record[record_size];
            if(control_presentation.separator!=STYLE_LINE_FLAT && control_presentation.separator!=STYLE_LINE_RELIEF)
                return CLASSIC_STYLE_INVALID;
        }else if(type==push_type){
            if(control_presentation.push || flags || payload!=push_size)return CLASSIC_STYLE_INVALID;
            control_presentation.push=record[record_size];
            if(control_presentation.push!=STYLE_PUSH_STEPPED && control_presentation.push!=STYLE_PUSH_RELIEF)
                return CLASSIC_STYLE_INVALID;
        }else if(type==control_lines_type){
            if(control_presentation.group || control_presentation.edit || flags || payload!=control_lines_size)
                return CLASSIC_STYLE_INVALID;
            control_presentation.group=record[record_size];
            control_presentation.edit=record[record_size+1];
            if((!control_presentation.group && !control_presentation.edit) ||
               control_presentation.group>STYLE_LINE_RELIEF || control_presentation.edit>STYLE_LINE_RELIEF)
                return CLASSIC_STYLE_INVALID;
        }else if(type==progress_type){
            if(control_presentation.progress.flags || flags || payload!=progress_size)return CLASSIC_STYLE_INVALID;
            memcpy(&control_presentation.progress,record+record_size,progress_size);
            if(!valid_progress(control_presentation.progress))return CLASSIC_STYLE_INVALID;
        }else if(type==trackbar_type){
            if(control_presentation.trackbar || flags || payload!=trackbar_size)return CLASSIC_STYLE_INVALID;
            unsigned profile=record[record_size];
            if(profile!=STYLE_TRACKBAR_OUTLINED && profile!=STYLE_TRACKBAR_RELIEF)return CLASSIC_STYLE_INVALID;
            control_presentation.trackbar=(style_trackbar_profile)profile;
        }else if(type==header_frame_type){
            if(control_presentation.header || flags || payload!=header_frame_size)return CLASSIC_STYLE_INVALID;
            unsigned profile=record[record_size];
            if(profile!=STYLE_LINE_FLAT && profile!=STYLE_LINE_RELIEF)return CLASSIC_STYLE_INVALID;
            control_presentation.header=(style_line_profile)profile;
        }else if(type==tab_frame_type){
            if(control_presentation.tab || flags || payload!=tab_frame_size)return CLASSIC_STYLE_INVALID;
            unsigned profile=record[record_size];
            if(profile!=STYLE_LINE_FLAT && profile!=STYLE_LINE_RELIEF)return CLASSIC_STYLE_INVALID;
            control_presentation.tab=(style_line_profile)profile;
        }else if(type==tab_text_type){
            if(control_presentation.tab_text || flags || payload!=tab_text_size)return CLASSIC_STYLE_INVALID;
            unsigned text=record[record_size];
            if(text!=STYLE_TAB_TEXT_LITERAL && text!=STYLE_TAB_TEXT_MNEMONIC)return CLASSIC_STYLE_INVALID;
            control_presentation.tab_text=(style_tab_text)text;
        }else if(type==native_edge_type){
            if(control_presentation.native_edge || flags || payload!=native_edge_size)return CLASSIC_STYLE_INVALID;
            unsigned profile=record[record_size];
            if(profile!=STYLE_LINE_FLAT && profile!=STYLE_LINE_RELIEF)return CLASSIC_STYLE_INVALID;
            control_presentation.native_edge=(style_line_profile)profile;
        }else if(type==caption_icon_type){
            if(presentation.icon_mode || flags || payload!=caption_icon_size)return CLASSIC_STYLE_INVALID;
            uint8_t mode=record[record_size];
            if(mode!=STYLE_CAPTION_ICON_HIDDEN && mode!=STYLE_CAPTION_ICON_NATIVE)return CLASSIC_STYLE_INVALID;
            presentation.icon_mode=mode;
        }else if(type==choice_fallback_type){
            if(control_presentation.choice_fallback || flags || payload!=choice_fallback_size ||
               record[record_size]!=CLASSIC_CHOICE_SYMBOLS)return CLASSIC_STYLE_INVALID;
            control_presentation.choice_fallback=CLASSIC_CHOICE_SYMBOLS;
        }else if(type==choice_metrics_type){
            if(control_presentation.choice_metrics.width || flags || payload!=choice_metrics_size)return CLASSIC_STYLE_INVALID;
            memcpy(&control_presentation.choice_metrics,record+record_size,choice_metrics_size);
            if(!valid_choice_metrics(control_presentation.choice_metrics))return CLASSIC_STYLE_INVALID;
        }else if(type==updown_type){
            if(has_updown(control_presentation.updown) || flags || payload!=updown_size)return CLASSIC_STYLE_INVALID;
            memcpy(&control_presentation.updown,record+record_size,updown_size);
            if(!valid_updown(control_presentation.updown))return CLASSIC_STYLE_INVALID;
        }else if(type==updown_mode_type){
            if(control_presentation.updown_mode || flags || payload!=updown_mode_size)return CLASSIC_STYLE_INVALID;
            unsigned mode=record[record_size];
            if(mode!=CLASSIC_UPDOWN_ART && mode!=CLASSIC_UPDOWN_RECOLOR)return CLASSIC_STYLE_INVALID;
            control_presentation.updown_mode=(enum classic_updown_mode)mode;
        }else if(type==menu_row_type){
            if(row_minimum || flags || payload!=menu_row_size)return CLASSIC_STYLE_INVALID;
            row_minimum=record[record_size];row_padding=record[record_size+1];
            if(!row_minimum || row_minimum>64 || row_padding>16)return CLASSIC_STYLE_INVALID;
        }else if(type==control_font_type){
            if(control_presentation.control_font_points || flags || payload!=control_font_size ||
               !record[record_size] || record[record_size]>48)return CLASSIC_STYLE_INVALID;
            control_presentation.control_font_points=record[record_size];
        }else if(type==scroll_repeat_type){
            if(metrics.initial_repeat_ms || flags || payload!=scroll_repeat_size)return CLASSIC_STYLE_INVALID;
            metrics.initial_repeat_ms=read_half(record+record_size);
            metrics.repeat_ms=read_half(record+record_size+2);
            if(!metrics.initial_repeat_ms || !metrics.repeat_ms)return CLASSIC_STYLE_INVALID;
        }else if(type==scroll_drag_type){
            if(metrics.drag.scale || flags || payload!=scroll_drag_size)return CLASSIC_STYLE_INVALID;
            memcpy(&metrics.drag,record+record_size,scroll_drag_size);
            if(!metrics.drag.scale || metrics.drag.scale>STYLE_SCROLL_DRAG_BAR)return CLASSIC_STYLE_INVALID;
        }else if(type==mdi_inactive_type){
            if(mdi_inactive || flags || payload!=mdi_inactive_size)return CLASSIC_STYLE_INVALID;
            mdi_inactive=record[record_size];
            if(!mdi_inactive || !valid_mdi_buttons(mdi_inactive))return CLASSIC_STYLE_INVALID;
        }else if(type==frame_scroll_type){
            if(scroll_shared || flags || payload!=frame_scroll_size || record[record_size]!=1)
                return CLASSIC_STYLE_INVALID;
            scroll_shared=record[record_size];
        }else if(flags&required_flag)return CLASSIC_STYLE_REQUIRED_RECORD;
        record+=record_size+payload;
        remaining-=record_size+payload;
    }
    if(remaining || ((menu_text || mdi_inactive) && !columns.check_column) || ((thumb || metrics.initial_repeat_ms || metrics.drag.scale) && !metrics.source) ||
       (stock_dialog_seen && !routes.window_provider) || (presentation.icon_mode && !presentation.text_mode) ||
       (scroll_shared && !strokes.geometry.scale) || (control_presentation.tab_text && !control_presentation.tab))
        return CLASSIC_STYLE_INVALID;
    metrics.thumb=thumb;
    strokes.geometry.scroll_shared_units=scroll_shared;
    columns.text_flags=menu_text;columns.bar_padding=bar_padding;columns.mdi_buttons=mdi_buttons;
    columns.row_minimum=row_minimum;columns.row_padding=row_padding;columns.mdi_inactive=mdi_inactive;
    if(!value.count)return CLASSIC_STYLE_NO_BUTTON_GROUP;
    *group=value;
    if(scroll)*scroll=metrics;
    if(menu)*menu=columns;
    if(dialog)*dialog=dimensions;
    if(policy)*policy=routes;
    if(caption)*caption=presentation;
    if(border)*border=strokes;
    if(controls)*controls=control_presentation;
    return CLASSIC_STYLE_OK;
}

size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group,
    const classic_scroll_metrics *scroll,const classic_menu_metrics *menu,const classic_dialog_metrics *dialog,
    const classic_dialog_policy *policy,const style_caption_presentation *caption,const style_border_presentation *border,
    const classic_control_presentation *controls)
{
    style_line_profile separator=controls?controls->separator:STYLE_LINE_DEFAULT;
    style_push_profile push=controls?controls->push:STYLE_PUSH_DEFAULT;
    bool lines=controls && (controls->group || controls->edit);
    bool progress=controls && controls->progress.flags;
    bool trackbar=controls && controls->trackbar;
    style_line_profile header=controls?controls->header:STYLE_LINE_DEFAULT;
    style_line_profile tab=controls?controls->tab:STYLE_LINE_DEFAULT;
    style_tab_text tab_text=controls?controls->tab_text:STYLE_TAB_TEXT_DEFAULT;
    style_line_profile native_edge=controls?controls->native_edge:STYLE_LINE_DEFAULT;
    bool stock=policy && (policy->color || policy->font || policy->find);
    bool icon=caption && caption->icon_mode;
    bool choice=controls && controls->choice_fallback;
    style_choice_metrics metrics=controls?controls->choice_metrics:(style_choice_metrics){0};
    bool geometry=metrics.width || metrics.height || metrics.gap || metrics.flags;
    style_arrow_button_metrics updown=controls?controls->updown:(style_arrow_button_metrics){0};
    bool arrow_button=has_updown(updown);
    unsigned updown_mode=controls?controls->updown_mode:CLASSIC_UPDOWN_NATIVE;
    unsigned control_font=controls?controls->control_font_points:0;
    if(!bytes || !group || !valid_group(group) || control_font>48 ||
       (scroll && (!valid_scroll(*scroll) || (unsigned)scroll->thumb>STYLE_SCROLL_THUMB_PAGE ||
         (!scroll->initial_repeat_ms!=!scroll->repeat_ms) || scroll->drag.scale>STYLE_SCROLL_DRAG_BAR ||
         (!scroll->drag.scale && (scroll->drag.first || scroll->drag.second)))) ||
       (menu && !valid_menu(*menu)) || (dialog && !valid_dialog(*dialog)) ||
       (policy && !valid_policy(*policy)) || (caption && !valid_caption(*caption)) ||
       (border && !valid_border(*border)) || (unsigned)separator>STYLE_LINE_RELIEF ||
       (unsigned)push>STYLE_PUSH_RELIEF || (lines &&
       ((unsigned)controls->group>STYLE_LINE_RELIEF || (unsigned)controls->edit>STYLE_LINE_RELIEF)) ||
       (progress && !valid_progress(controls->progress)) ||
       (trackbar && controls->trackbar!=STYLE_TRACKBAR_OUTLINED && controls->trackbar!=STYLE_TRACKBAR_RELIEF) ||
       (unsigned)header>STYLE_LINE_RELIEF || (unsigned)tab>STYLE_LINE_RELIEF || (unsigned)native_edge>STYLE_LINE_RELIEF ||
       (unsigned)tab_text>STYLE_TAB_TEXT_MNEMONIC || (tab_text && !tab) ||
       (choice && controls->choice_fallback!=CLASSIC_CHOICE_SYMBOLS) || (geometry && !valid_choice_metrics(metrics)) ||
       (arrow_button && !valid_updown(updown)) || updown_mode>CLASSIC_UPDOWN_RECOLOR)return 0;
    size_t caption_size=caption && caption->button_source?caption_presentation_size:caption_presentation_legacy_size;
    size_t border_size=border && border->geometry.scale?border_presentation_size:border_presentation_legacy_size;
    bool menu_text=menu && menu->text_flags;
    bool thumb=scroll && scroll->thumb;
    bool scroll_repeat=scroll && scroll->initial_repeat_ms;
    bool scroll_drag=scroll && scroll->drag.scale;
    bool menu_row=menu && menu->row_minimum;
    bool mdi_inactive=menu && menu->mdi_inactive;
    bool frame_scroll=border && border->geometry.scroll_shared_units;
    size_t menu_bar=menu && menu->mdi_buttons?menu_bar_size:menu && menu->bar_padding?1:0;
    size_t payload=group_header_size+slot_size*(unsigned)group->count;
    size_t length=header_size+record_size+payload+(scroll?record_size+scroll_metrics_size:0)+
        (menu?record_size+menu_metrics_size:0)+(menu_text?record_size+menu_text_size:0)+
        (dialog?record_size+dialog_metrics_size:0)+
        (policy?record_size+dialog_policy_size:0)+(caption?record_size+caption_size:0)+
        (border?record_size+border_size:0)+(separator?record_size+separator_size:0)+
        (thumb?record_size+thumb_size:0)+(push?record_size+push_size:0)+
        (lines?record_size+control_lines_size:0)+(progress?record_size+progress_size:0)+(trackbar?record_size+trackbar_size:0)+
        (header?record_size+header_frame_size:0)+(tab?record_size+tab_frame_size:0)+
        (tab_text?record_size+tab_text_size:0)+
        (stock?record_size+stock_dialog_size:0)+(menu_bar?record_size+menu_bar:0)+(native_edge?record_size+native_edge_size:0)+
        (icon?record_size+caption_icon_size:0)+(choice?record_size+choice_fallback_size:0)+
        (geometry?record_size+choice_metrics_size:0)+(arrow_button?record_size+updown_size:0)+
        (updown_mode?record_size+updown_mode_size:0)+(menu_row?record_size+menu_row_size:0)+
        (control_font?record_size+control_font_size:0)+(scroll_repeat?record_size+scroll_repeat_size:0)+(scroll_drag?record_size+scroll_drag_size:0)+
        (mdi_inactive?record_size+mdi_inactive_size:0)+(frame_scroll?record_size+frame_scroll_size:0);
    if(capacity<length)return 0;
    memcpy(bytes,"CSTY",4);
    write_half(bytes+4,1);
    write_half(bytes+6,tab_text?33:frame_scroll?32:mdi_inactive?31:scroll_drag?30:scroll_repeat?29:group->layout?28:control_font?27:menu_row?26:updown_mode?25:arrow_button?24:geometry?23:choice?22:icon?21:native_edge?20:menu_bar?19:stock?18:tab?17:header?16:trackbar?15:progress?14:lines?13:push?12:thumb?11:separator?10:menu_text?9:border_size==border_presentation_size?8:caption_size==caption_presentation_size?7:
        border?6:caption?5:policy?4:dialog?3:menu?2:scroll?1:0);
    msstyles_v4_write_word(bytes+8,(uint32_t)length);
    msstyles_v4_write_word(bytes+12,1+(scroll!=NULL)+(menu!=NULL)+menu_text+(dialog!=NULL)+(policy!=NULL)+
        (caption!=NULL)+(border!=NULL)+(separator!=0)+thumb+(push!=0)+lines+progress+trackbar+(header!=0)+(tab!=0)+(tab_text!=0)+stock+(menu_bar!=0)+(native_edge!=0)+icon+choice+geometry+arrow_button+(updown_mode!=0)+menu_row+(control_font!=0)+scroll_repeat+scroll_drag+mdi_inactive+frame_scroll);
    write_half(bytes+16,button_group_type);
    write_half(bytes+18,required_flag);
    msstyles_v4_write_word(bytes+20,(uint32_t)payload);
    write_half(bytes+24,group->count);
    write_half(bytes+26,group->left_margin);
    write_half(bytes+28,group->right_margin);
    write_half(bytes+30,group->layout);
    unsigned char *slot=bytes+32;
    for(unsigned i=0;i<group->count;++i,slot+=slot_size){
        slot[0]=group->slots[i].role;
        slot[1]=group->slots[i].side;
        write_half(slot+2,group->slots[i].gap_before);
    }
    if(scroll){
        write_half(slot,scroll_metrics_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,scroll_metrics_size);
        write_half(slot+8,scroll->source);
        write_half(slot+10,scroll->logical_extent);
        slot+=record_size+scroll_metrics_size;
    }
    if(menu){
        write_half(slot,menu_metrics_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,menu_metrics_size);
        write_half(slot+8,menu->check_column);
        write_half(slot+10,menu->shortcut_gap);
        write_half(slot+12,menu->arrow_column);
        write_half(slot+14,menu->plain_column);
        slot+=record_size+menu_metrics_size;
    }
    if(menu_text){
        write_half(slot,menu_text_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,menu_text_size);
        write_half(slot+8,menu->text_flags);
        slot+=record_size+menu_text_size;
    }
    if(dialog){
        const classic_file_metrics *file=&dialog->file;
        write_half(slot,dialog_metrics_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,dialog_metrics_size);
        write_half(slot+8,file->compact_width);
        write_half(slot+10,file->compact_height);
        write_half(slot+12,file->places_width);
        write_half(slot+14,file->places_height);
        write_half(slot+16,file->client_dlu_width);
        write_half(slot+18,file->client_dlu_height);
        write_half(slot+20,file->gap);
        write_half(slot+22,file->bottom_reserve);
        write_half(slot+24,dialog->print_width);
        write_half(slot+26,dialog->print_full_height);
        write_half(slot+28,dialog->print_large_height);
        write_half(slot+30,dialog->color_width);
        write_half(slot+32,dialog->color_height);
        write_half(slot+34,dialog->find_width);
        write_half(slot+36,dialog->find_native_height);
        write_half(slot+38,dialog->find_classic_height);
        write_half(slot+40,dialog->font_width);
        write_half(slot+42,dialog->font_height);
        slot+=record_size+dialog_metrics_size;
    }
    if(policy){
        write_half(slot,dialog_policy_type);
        write_half(slot+2,required_flag);
        msstyles_v4_write_word(slot+4,dialog_policy_size);
        slot[8]=policy->window_provider;
        slot[9]=policy->file;
        slot[10]=policy->folder;
        slot[11]=policy->print;
        slot+=record_size+dialog_policy_size;
    }
    if(caption){
        write_half(slot,caption_presentation_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,(uint32_t)caption_size);
        memcpy(slot+record_size,caption,caption_size);
        slot+=record_size+caption_size;
    }
    if(border){
        write_half(slot,border_presentation_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,(uint32_t)border_size);
        memcpy(slot+record_size,border,border_size);
        slot+=record_size+border_size;
    }
    if(separator){
        write_half(slot,separator_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,separator_size);
        slot[record_size]=(unsigned char)separator;
        slot+=record_size+separator_size;
    }
    if(thumb){
        write_half(slot,thumb_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,thumb_size);
        slot[record_size]=(unsigned char)scroll->thumb;
        slot+=record_size+thumb_size;
    }
    if(push){
        write_half(slot,push_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,push_size);
        slot[record_size]=(unsigned char)push;
        slot+=record_size+push_size;
    }
    if(lines){
        write_half(slot,control_lines_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,control_lines_size);
        slot[record_size]=(unsigned char)controls->group;
        slot[record_size+1]=(unsigned char)controls->edit;
        slot+=record_size+control_lines_size;
    }
    if(progress){
        write_half(slot,progress_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,progress_size);
        memcpy(slot+record_size,&controls->progress,progress_size);
        slot+=record_size+progress_size;
    }
    if(trackbar){
        write_half(slot,trackbar_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,trackbar_size);
        slot[record_size]=(unsigned char)controls->trackbar;
        slot+=record_size+trackbar_size;
    }
    if(header){
        write_half(slot,header_frame_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,header_frame_size);
        slot[record_size]=(unsigned char)header;
        slot+=record_size+header_frame_size;
    }
    if(tab){
        write_half(slot,tab_frame_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,tab_frame_size);
        slot[record_size]=(unsigned char)tab;
        slot+=record_size+tab_frame_size;
    }
    if(tab_text){
        write_half(slot,tab_text_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,tab_text_size);
        slot[record_size]=(unsigned char)tab_text;
        slot+=record_size+tab_text_size;
    }
    if(stock){
        write_half(slot,stock_dialog_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,stock_dialog_size);
        slot[8]=policy->color;slot[9]=policy->font;slot[10]=policy->find;
        slot+=record_size+stock_dialog_size;
    }
    if(menu_bar){
        write_half(slot,menu_bar_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,(uint32_t)menu_bar);
        slot[record_size]=(unsigned char)menu->bar_padding;
        if(menu_bar==menu_bar_size)slot[record_size+1]=menu->mdi_buttons;
        slot+=record_size+menu_bar;
    }
    if(native_edge){
        write_half(slot,native_edge_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,native_edge_size);
        slot[record_size]=(unsigned char)native_edge;
        slot+=record_size+native_edge_size;
    }
    if(icon){
        write_half(slot,caption_icon_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,caption_icon_size);
        slot[record_size]=caption->icon_mode;
        slot+=record_size+caption_icon_size;
    }
    if(choice){
        write_half(slot,choice_fallback_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,choice_fallback_size);
        slot[record_size]=CLASSIC_CHOICE_SYMBOLS;
        slot+=record_size+choice_fallback_size;
    }
    if(geometry){
        write_half(slot,choice_metrics_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,choice_metrics_size);
        memcpy(slot+record_size,&metrics,choice_metrics_size);
        slot+=record_size+choice_metrics_size;
    }
    if(arrow_button){
        write_half(slot,updown_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,updown_size);
        memcpy(slot+record_size,&updown,updown_size);
        slot+=record_size+updown_size;
    }
    if(updown_mode){
        write_half(slot,updown_mode_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,updown_mode_size);
        slot[record_size]=(BYTE)updown_mode;
        slot+=record_size+updown_mode_size;
    }
    if(menu_row){
        write_half(slot,menu_row_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,menu_row_size);
        slot[record_size]=menu->row_minimum;
        slot[record_size+1]=menu->row_padding;
        slot+=record_size+menu_row_size;
    }
    if(control_font){
        write_half(slot,control_font_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,control_font_size);
        slot[record_size]=(BYTE)control_font;
        slot+=record_size+control_font_size;
    }
    if(scroll_repeat){
        write_half(slot,scroll_repeat_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,scroll_repeat_size);
        write_half(slot+record_size,scroll->initial_repeat_ms);
        write_half(slot+record_size+2,scroll->repeat_ms);
        slot+=record_size+scroll_repeat_size;
    }
    if(scroll_drag){
        write_half(slot,scroll_drag_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,scroll_drag_size);
        memcpy(slot+record_size,&scroll->drag,scroll_drag_size);
        slot+=record_size+scroll_drag_size;
    }
    if(mdi_inactive){
        write_half(slot,mdi_inactive_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,mdi_inactive_size);
        slot[record_size]=menu->mdi_inactive;
        slot+=record_size+mdi_inactive_size;
    }
    if(frame_scroll){
        write_half(slot,frame_scroll_type);
        write_half(slot+2,0);
        msstyles_v4_write_word(slot+4,frame_scroll_size);
        slot[record_size]=border->geometry.scroll_shared_units;
    }
    return length;
}
