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
    menu_text_type=12, menu_text_size=2, required_flag=1
};

_Static_assert(sizeof(style_border_presentation)==border_presentation_size,"border resource layout");
_Static_assert(sizeof(style_caption_presentation)==caption_presentation_size,"caption resource layout");
_Static_assert(header_size+8*record_size+group_header_size+slot_size*CLASSIC_BUTTON_CLOSE+
    scroll_metrics_size+menu_metrics_size+dialog_metrics_size+dialog_policy_size+
    caption_presentation_size+border_presentation_size+menu_text_size==CLASSIC_STYLE_RESOURCE_CAPACITY,"classic resource capacity");

static uint16_t read_half(const unsigned char *bytes)
{
    return (uint16_t)(bytes[0]|(bytes[1]<<8));
}

static void write_half(unsigned char *bytes,uint16_t value)
{
    bytes[0]=(unsigned char)value;
    bytes[1]=(unsigned char)(value>>8);
}

static bool valid_group(const classic_button_group *group)
{
    if(!group->count || group->count>CLASSIC_BUTTON_CLOSE)return false;
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

static bool read_group(const unsigned char *bytes,size_t length,classic_button_group *group)
{
    if(length<group_header_size || read_half(bytes+6))return false;
    *group=(classic_button_group){
        .count=read_half(bytes),.left_margin=read_half(bytes+2),.right_margin=read_half(bytes+4)
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

static bool valid_menu(classic_menu_metrics menu)
{
    return menu.check_column>=16 && menu.check_column<=256 && menu.shortcut_gap<=256 &&
        menu.arrow_column>=11 && menu.arrow_column<=256 &&
        menu.plain_column>0 && menu.plain_column<=menu.arrow_column &&
        (!menu.text_flags || valid_menu_text(menu.text_flags));
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
    return policy.appearance>=CLASSIC_DIALOG_WIN16 && policy.appearance<=CLASSIC_DIALOG_WIN2K &&
        policy.file<=CLASSIC_DIALOG_WIN2K && policy.folder<=CLASSIC_DIALOG_WIN2K &&
        policy.print<=CLASSIC_DIALOG_WIN2K;
}

static bool valid_caption(style_caption_presentation caption)
{
    return (caption.text_mode==STYLE_CAPTION_TEXT_CENTER_CLIP ||
        caption.text_mode==STYLE_CAPTION_TEXT_LEFT_ELLIPSIS) &&
        caption.separator<=STYLE_CAPTION_SEPARATOR_AFTER &&
        caption.button_source<=STYLE_CAPTION_BUTTON_SYSTEM &&
        caption.button_outline<=STYLE_CAPTION_BUTTON_EDGE_OUTLINE &&
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
        g.scale<=STYLE_FRAME_MONITOR_PIXELS && g.separator_units<=1 &&
        (g.scale || !(g.sizeable || g.fixed || g.dialog || g.line || g.caption_floor || g.separator_units));
}

enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group,classic_scroll_metrics *scroll,classic_menu_metrics *menu,classic_dialog_metrics *dialog,
    classic_dialog_policy *policy,style_caption_presentation *caption,style_border_presentation *border)
{
    if(!bytes || !group || length<header_size || memcmp(bytes,"CSTY",4) ||
       msstyles_v4_read_word(bytes+8)!=length)return CLASSIC_STYLE_INVALID;
    if(read_half(bytes+4)!=1)return CLASSIC_STYLE_VERSION;
    uint32_t count=msstyles_v4_read_word(bytes+12);
    const unsigned char *record=bytes+header_size;
    size_t remaining=length-header_size;
    classic_button_group value={0};
    classic_scroll_metrics metrics={0};
    classic_menu_metrics columns={0};
    uint16_t menu_text=0;
    classic_dialog_metrics dimensions={0};
    classic_dialog_policy routes={0};
    style_caption_presentation presentation={0};
    style_border_presentation strokes={0};
    for(uint32_t i=0;i<count;++i){
        if(remaining<record_size)return CLASSIC_STYLE_INVALID;
        uint16_t type=read_half(record),flags=read_half(record+2);
        size_t payload=msstyles_v4_read_word(record+4);
        if((flags&~required_flag) || payload>remaining-record_size)return CLASSIC_STYLE_INVALID;
        if(type==button_group_type){
            if(value.count || !read_group(record+record_size,payload,&value))return CLASSIC_STYLE_INVALID;
        }else if(type==scroll_metrics_type){
            if(metrics.source || payload!=scroll_metrics_size)return CLASSIC_STYLE_INVALID;
            metrics=(classic_scroll_metrics){read_half(record+record_size),read_half(record+record_size+2)};
            if(!valid_scroll(metrics))return CLASSIC_STYLE_INVALID;
        }else if(type==menu_metrics_type){
            if(columns.check_column || payload!=menu_metrics_size)return CLASSIC_STYLE_INVALID;
            const unsigned char *data=record+record_size;
            columns=(classic_menu_metrics){read_half(data),read_half(data+2),read_half(data+4),read_half(data+6),0};
            if(!valid_menu(columns))return CLASSIC_STYLE_INVALID;
        }else if(type==menu_text_type){
            if(menu_text || payload!=menu_text_size || flags)return CLASSIC_STYLE_INVALID;
            menu_text=read_half(record+record_size);
            if(!valid_menu_text(menu_text))return CLASSIC_STYLE_INVALID;
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
            if(routes.appearance || payload!=dialog_policy_size || flags!=required_flag)
                return CLASSIC_STYLE_INVALID;
            const unsigned char *data=record+record_size;
            routes=(classic_dialog_policy){data[0],data[1],data[2],data[3]};
            if(!valid_policy(routes))return CLASSIC_STYLE_INVALID;
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
        }else if(flags&required_flag)return CLASSIC_STYLE_REQUIRED_RECORD;
        record+=record_size+payload;
        remaining-=record_size+payload;
    }
    if(remaining || (menu_text && !columns.check_column))return CLASSIC_STYLE_INVALID;
    columns.text_flags=menu_text;
    if(!value.count)return CLASSIC_STYLE_NO_BUTTON_GROUP;
    *group=value;
    if(scroll)*scroll=metrics;
    if(menu)*menu=columns;
    if(dialog)*dialog=dimensions;
    if(policy)*policy=routes;
    if(caption)*caption=presentation;
    if(border)*border=strokes;
    return CLASSIC_STYLE_OK;
}

size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group,
    const classic_scroll_metrics *scroll,const classic_menu_metrics *menu,const classic_dialog_metrics *dialog,
    const classic_dialog_policy *policy,const style_caption_presentation *caption,const style_border_presentation *border)
{
    if(!bytes || !group || !valid_group(group) || (scroll && !valid_scroll(*scroll)) ||
       (menu && !valid_menu(*menu)) || (dialog && !valid_dialog(*dialog)) ||
       (policy && !valid_policy(*policy)) || (caption && !valid_caption(*caption)) ||
       (border && !valid_border(*border)))return 0;
    size_t caption_size=caption && caption->button_source?caption_presentation_size:caption_presentation_legacy_size;
    size_t border_size=border && border->geometry.scale?border_presentation_size:border_presentation_legacy_size;
    bool menu_text=menu && menu->text_flags;
    size_t payload=group_header_size+slot_size*(unsigned)group->count;
    size_t length=header_size+record_size+payload+(scroll?record_size+scroll_metrics_size:0)+
        (menu?record_size+menu_metrics_size:0)+(menu_text?record_size+menu_text_size:0)+
        (dialog?record_size+dialog_metrics_size:0)+
        (policy?record_size+dialog_policy_size:0)+(caption?record_size+caption_size:0)+
        (border?record_size+border_size:0);
    if(capacity<length)return 0;
    memcpy(bytes,"CSTY",4);
    write_half(bytes+4,1);
    write_half(bytes+6,menu_text?9:border_size==border_presentation_size?8:caption_size==caption_presentation_size?7:
        border?6:caption?5:policy?4:dialog?3:menu?2:scroll?1:0);
    msstyles_v4_write_word(bytes+8,(uint32_t)length);
    msstyles_v4_write_word(bytes+12,1+(scroll!=NULL)+(menu!=NULL)+menu_text+(dialog!=NULL)+(policy!=NULL)+(caption!=NULL)+(border!=NULL));
    write_half(bytes+16,button_group_type);
    write_half(bytes+18,required_flag);
    msstyles_v4_write_word(bytes+20,(uint32_t)payload);
    write_half(bytes+24,group->count);
    write_half(bytes+26,group->left_margin);
    write_half(bytes+28,group->right_margin);
    write_half(bytes+30,0);
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
        slot[8]=policy->appearance;
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
    }
    return length;
}
