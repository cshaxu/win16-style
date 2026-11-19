#include "v4_tables.h"
#include "v4_endian.h"
#include <string.h>

static bool class_span(msstyles_v4_name name,size_t available,size_t *span)
{
    if(!name.bytes || name.units<2 || name.units>available/2)return false;
    size_t length=(size_t)name.units*2;
    size_t pad=(-length)&7u;
    if(pad>available-length || name.bytes[length-2] || name.bytes[length-1])return false;
    *span=length+pad;
    return true;
}

bool msstyles_v4_class_read(const unsigned char *bytes,size_t available,
    msstyles_v4_name *name,size_t *used)
{
    if(!bytes || !name || !used)return false;
    size_t units=0,limit=available/2;
    while(units<limit && (bytes[units*2] || bytes[units*2+1]))++units;
    if(units==limit || units>=UINT32_MAX)return false;
    msstyles_v4_name value={bytes,(uint32_t)(units+1)};
    size_t span;
    if(!class_span(value,available,&span))return false;
    for(size_t pos=(size_t)value.units*2;pos<span;++pos)
        if(bytes[pos])return false;
    *name=value;
    *used=span;
    return true;
}

bool msstyles_v4_class_write(unsigned char *bytes,size_t available,
    msstyles_v4_name name,size_t *used)
{
    size_t span;
    if(!bytes || !used || !class_span(name,available,&span))return false;
    size_t length=(size_t)name.units*2;
    for(size_t pos=0;pos<length-2;pos+=2)
        if(!name.bytes[pos] && !name.bytes[pos+1])return false;
    memcpy(bytes,name.bytes,length);
    memset(bytes+length,0,span-length);
    *used=span;
    return true;
}

static bool name_span(msstyles_v4_name name,size_t available,size_t *span)
{
    if(available<4 || name.units<2 || name.units>(available-4)/2 || !name.bytes)return false;
    size_t length=(size_t)name.units*2;
    size_t pad=length&2u;
    if(pad>available-4-length || name.bytes[length-2] || name.bytes[length-1])return false;
    for(size_t pos=0;pos<length-2;pos+=2)
        if(!name.bytes[pos] && !name.bytes[pos+1])return false;
    *span=4+length+pad;
    return true;
}

bool msstyles_v4_variant_read(const unsigned char *bytes,size_t available,
    msstyles_v4_variant *variant,size_t *used)
{
    if(!bytes || !variant || !used)return false;
    msstyles_v4_variant value;
    msstyles_v4_name *names[]={&value.record,&value.size,&value.color};
    size_t pos=0;
    for(size_t i=0;i<3;++i){
        if(available-pos<4)return false;
        msstyles_v4_name name={bytes+pos+4,msstyles_v4_read_word(bytes+pos)};
        size_t span;
        if(!name_span(name,available-pos,&span))return false;
        *names[i]=name;
        pos+=span;
    }
    *variant=value;
    *used=pos;
    return true;
}

bool msstyles_v4_variant_write(unsigned char *bytes,size_t available,
    const msstyles_v4_variant *variant,size_t *used)
{
    if(!bytes || !variant || !used)return false;
    const msstyles_v4_name names[]={variant->record,variant->size,variant->color};
    size_t spans[3],total=0;
    for(size_t i=0;i<3;++i){
        if(!name_span(names[i],available-total,&spans[i]))return false;
        total+=spans[i];
    }
    size_t pos=0;
    for(size_t i=0;i<3;++i){
        size_t length=(size_t)names[i].units*2;
        msstyles_v4_write_word(bytes+pos,names[i].units);
        memcpy(bytes+pos+4,names[i].bytes,length);
        memset(bytes+pos+4+length,0,spans[i]-4-length);
        pos+=spans[i];
    }
    *used=total;
    return true;
}
