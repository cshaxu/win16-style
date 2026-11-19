#include "v4_property.h"
#include <string.h>

enum { property_header_size=32 };

static uint32_t read_word(const unsigned char *bytes)
{
    return (uint32_t)bytes[0]|((uint32_t)bytes[1]<<8)|
        ((uint32_t)bytes[2]<<16)|((uint32_t)bytes[3]<<24);
}

static void write_word(unsigned char *bytes,uint32_t word)
{
    for(unsigned i=0;i<4;++i)bytes[i]=(unsigned char)(word>>(8*i));
}

static bool property_span(size_t available,size_t payload,size_t *span)
{
    if(available<property_header_size)return false;
    size_t remaining=available-property_header_size;
    if(payload>remaining)return false;
    size_t pad=(-payload)&7u;
    if(pad>remaining-payload)return false;
    *span=property_header_size+payload+pad;
    return true;
}

bool msstyles_v4_property_read(const unsigned char *bytes,size_t available,
    size_t class_count,msstyles_v4_property *property,size_t *used)
{
    if(!bytes || !property || !used || available<property_header_size)return false;
    msstyles_v4_property item={
        .name_id=read_word(bytes),.type_id=read_word(bytes+4),
        .class_id=read_word(bytes+8),.part_id=read_word(bytes+12),.state_id=read_word(bytes+16),
        .short_value=read_word(bytes+20),.declared_size=read_word(bytes+28)
    };
    size_t stored=item.short_value?0:item.declared_size;
    size_t span;
    if(read_word(bytes+24)!=0 || item.class_id>=class_count ||
        !property_span(available,stored,&span))return false;

    if(stored)item.data=bytes+property_header_size;
    *property=item;
    *used=span;
    return true;
}

bool msstyles_v4_property_write(unsigned char *bytes,size_t available,
    size_t class_count,const msstyles_v4_property *property,size_t *used)
{
    if(!bytes || !property || !used || property->class_id>=class_count)return false;
    size_t stored=property->short_value?0:property->declared_size;
    size_t span;
    if((stored && !property->data) || !property_span(available,stored,&span))return false;

    const uint32_t words[]={property->name_id,property->type_id,property->class_id,
        property->part_id,property->state_id,property->short_value,0,property->declared_size};
    for(unsigned i=0;i<sizeof(words)/sizeof(words[0]);++i)write_word(bytes+4*i,words[i]);
    if(stored)memcpy(bytes+property_header_size,property->data,stored);
    memset(bytes+property_header_size+stored,0,span-property_header_size-stored);
    *used=span;
    return true;
}
