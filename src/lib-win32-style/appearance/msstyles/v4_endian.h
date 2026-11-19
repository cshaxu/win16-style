#ifndef STYLE_MSSTYLES_V4_ENDIAN_H
#define STYLE_MSSTYLES_V4_ENDIAN_H

#include <stdint.h>

static inline uint32_t msstyles_v4_read_word(const unsigned char *bytes)
{
    return (uint32_t)bytes[0]|((uint32_t)bytes[1]<<8)|
        ((uint32_t)bytes[2]<<16)|((uint32_t)bytes[3]<<24);
}

static inline void msstyles_v4_write_word(unsigned char *bytes,uint32_t word)
{
    for(unsigned i=0;i<4;++i)bytes[i]=(unsigned char)(word>>(8*i));
}

#endif
