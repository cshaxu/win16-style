#include "dib_strip.h"
#include <string.h>

BOOL style_dib_strip_read(const void *data,DWORD bytes,unsigned count,BOOL horizontal,style_dib_strip *output)
{
    BITMAPINFOHEADER h;
    if(!data || bytes<sizeof(h) || !output || !count)return FALSE;
    memcpy(&h,data,sizeof(h));
    if(h.biSize!=sizeof(h) || h.biPlanes!=1 || h.biCompression!=BI_RGB ||
       h.biWidth<=0 || h.biWidth>8192 || !h.biHeight || h.biHeight<-8192 || h.biHeight>8192)
        return FALSE;
    unsigned depth=h.biBitCount;
    if(depth!=4 && depth!=8 && depth!=24 && depth!=32)return FALSE;
    unsigned width=(unsigned)h.biWidth,height=(unsigned)(h.biHeight<0?-h.biHeight:h.biHeight);
    if((horizontal?width:height)%count)return FALSE;
    unsigned capacity=depth<=8?1u<<depth:256u;
    unsigned colors=depth<=8 && !h.biClrUsed?capacity:h.biClrUsed;
    if(colors>capacity)return FALSE;
    unsigned offset=sizeof(h)+colors*sizeof(RGBQUAD),stride=((width*depth+31u)/32u)*4u;
    if(offset>bytes || height>(bytes-offset)/stride)return FALSE;
    const BYTE *start=data;
    *output=(style_dib_strip){.bits=start+offset,.palette=(const RGBQUAD *)(start+sizeof(h)),
        .colors=colors,.width=horizontal?width/count:width,.height=horizontal?height:height/count,
        .stride=stride,.count=count,.depth=depth,.top_down=h.biHeight<0,.horizontal=horizontal};
    return TRUE;
}
