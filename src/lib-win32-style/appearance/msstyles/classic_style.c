#include "classic_style.h"
#include "v4_endian.h"
#include <stdbool.h>
#include <string.h>

enum {
    header_size=16, record_size=8, group_header_size=8, slot_size=4,
    button_group_type=1, required_flag=1
};

_Static_assert(header_size+record_size+group_header_size+slot_size*CLASSIC_BUTTON_CLOSE==
    CLASSIC_STYLE_BUTTON_RESOURCE_CAPACITY,"title resource capacity");

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

enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group)
{
    if(!bytes || !group || length<header_size || memcmp(bytes,"CSTY",4) ||
       msstyles_v4_read_word(bytes+8)!=length)return CLASSIC_STYLE_INVALID;
    if(read_half(bytes+4)!=1)return CLASSIC_STYLE_VERSION;
    uint32_t count=msstyles_v4_read_word(bytes+12);
    const unsigned char *record=bytes+header_size;
    size_t remaining=length-header_size;
    classic_button_group value={0};
    for(uint32_t i=0;i<count;++i){
        if(remaining<record_size)return CLASSIC_STYLE_INVALID;
        uint16_t type=read_half(record),flags=read_half(record+2);
        size_t payload=msstyles_v4_read_word(record+4);
        if((flags&~required_flag) || payload>remaining-record_size)return CLASSIC_STYLE_INVALID;
        if(type==button_group_type){
            if(value.count || !read_group(record+record_size,payload,&value))return CLASSIC_STYLE_INVALID;
        }else if(flags&required_flag)return CLASSIC_STYLE_REQUIRED_RECORD;
        record+=record_size+payload;
        remaining-=record_size+payload;
    }
    if(remaining)return CLASSIC_STYLE_INVALID;
    if(!value.count)return CLASSIC_STYLE_NO_BUTTON_GROUP;
    *group=value;
    return CLASSIC_STYLE_OK;
}

size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group)
{
    if(!bytes || !group || !valid_group(group))return 0;
    size_t payload=group_header_size+slot_size*(unsigned)group->count;
    size_t length=header_size+record_size+payload;
    if(capacity<length)return 0;
    memcpy(bytes,"CSTY",4);
    write_half(bytes+4,1);
    write_half(bytes+6,0);
    msstyles_v4_write_word(bytes+8,(uint32_t)length);
    msstyles_v4_write_word(bytes+12,1);
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
    return length;
}
