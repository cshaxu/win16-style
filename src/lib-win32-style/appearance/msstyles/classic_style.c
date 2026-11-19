#include "classic_style.h"
#include "v4_endian.h"
#include <stdbool.h>
#include <string.h>

enum {
    header_size=16, record_size=8, group_header_size=8, slot_size=4,
    button_group_type=1, scroll_metrics_type=3, scroll_metrics_size=4,
    menu_metrics_type=7, menu_metrics_size=8, required_flag=1
};

_Static_assert(header_size+3*record_size+group_header_size+slot_size*CLASSIC_BUTTON_CLOSE+
    scroll_metrics_size+menu_metrics_size==CLASSIC_STYLE_RESOURCE_CAPACITY,"classic resource capacity");

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

static bool valid_menu(classic_menu_metrics menu)
{
    return menu.check_column>=16 && menu.check_column<=256 && menu.shortcut_gap<=256 &&
        menu.arrow_column>=11 && menu.arrow_column<=256 &&
        menu.plain_column>0 && menu.plain_column<=menu.arrow_column;
}

enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group,classic_scroll_metrics *scroll,classic_menu_metrics *menu)
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
            columns=(classic_menu_metrics){read_half(data),read_half(data+2),read_half(data+4),read_half(data+6)};
            if(!valid_menu(columns))return CLASSIC_STYLE_INVALID;
        }else if(flags&required_flag)return CLASSIC_STYLE_REQUIRED_RECORD;
        record+=record_size+payload;
        remaining-=record_size+payload;
    }
    if(remaining)return CLASSIC_STYLE_INVALID;
    if(!value.count)return CLASSIC_STYLE_NO_BUTTON_GROUP;
    *group=value;
    if(scroll)*scroll=metrics;
    if(menu)*menu=columns;
    return CLASSIC_STYLE_OK;
}

size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group,
    const classic_scroll_metrics *scroll,const classic_menu_metrics *menu)
{
    if(!bytes || !group || !valid_group(group) || (scroll && !valid_scroll(*scroll)) ||
       (menu && !valid_menu(*menu)))return 0;
    size_t payload=group_header_size+slot_size*(unsigned)group->count;
    size_t length=header_size+record_size+payload+(scroll?record_size+scroll_metrics_size:0)+
        (menu?record_size+menu_metrics_size:0);
    if(capacity<length)return 0;
    memcpy(bytes,"CSTY",4);
    write_half(bytes+4,1);
    write_half(bytes+6,menu?2:scroll?1:0);
    msstyles_v4_write_word(bytes+8,(uint32_t)length);
    msstyles_v4_write_word(bytes+12,1+(scroll!=NULL)+(menu!=NULL));
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
    }
    return length;
}
