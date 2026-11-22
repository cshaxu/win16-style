#include "dib_strip.h"
#include "bitmap.h"
#include <stdint.h>

static BOOL alpha_present(const style_dib_strip *strip)
{
    if(strip->depth!=32)return FALSE;
    unsigned width=strip->horizontal?strip->width*strip->count:strip->width;
    unsigned height=strip->horizontal?strip->height:strip->height*strip->count;
    for(unsigned y=0;y<height;++y)
        for(unsigned x=0;x<width;++x)
            if(strip->bits[(size_t)y*strip->stride+x*4u+3])return TRUE;
    return FALSE;
}

static BOOL decode(const style_dib_strip *strip,unsigned state,BOOL alpha,style_bitmap *bitmap)
{
    unsigned height=strip->horizontal?strip->height:strip->height*strip->count;
    unsigned left=strip->horizontal?state*strip->width:0,top=strip->horizontal?0:state*strip->height;
    for(unsigned y=0;y<strip->height;++y){
        unsigned row=strip->top_down?top+y:height-1-top-y;
        const BYTE *source=strip->bits+(size_t)row*strip->stride+left*(strip->depth/8u);
        BYTE *target=bitmap->pixels+(size_t)y*strip->width*4u;
        for(unsigned x=0;x<strip->width;++x){
            RGBQUAD color;
            if(strip->depth==8){
                if(*source>=strip->colors)return FALSE;
                color=strip->palette[*source++];
            }else{
                color=(RGBQUAD){source[0],source[1],source[2],alpha?source[3]:255};
                source+=strip->depth/8u;
            }
            unsigned opacity=alpha?color.rgbReserved:255;
            if(strip->transparent && RGB(color.rgbRed,color.rgbGreen,color.rgbBlue)==strip->color_key)
                opacity=0;
            *target++=(BYTE)((color.rgbBlue*opacity+127u)/255u);
            *target++=(BYTE)((color.rgbGreen*opacity+127u)/255u);
            *target++=(BYTE)((color.rgbRed*opacity+127u)/255u);
            *target++=(BYTE)opacity;
        }
    }
    return TRUE;
}

static void slice(int width,int low,int high,int positions[4])
{
    if(low+high>width){
        low=MulDiv(low,width,low+high);
        high=width-low;
    }
    positions[0]=0;positions[1]=low;positions[2]=width-high;positions[3]=width;
}

BOOL style_dib_strip_draw(HDC dc,RECT bounds,const style_dib_strip *strip,unsigned state,UINT dpi)
{
    if(!dc || !strip || !strip->bits || state>=strip->count || !dpi || IsRectEmpty(&bounds))return FALSE;
    int w=(int)strip->width,h=(int)strip->height;
    RECT m=strip->margins;
    if(m.left<0 || m.right<0 || m.top<0 || m.bottom<0 ||
       (int64_t)m.left+m.right>w || (int64_t)m.top+m.bottom>h)return FALSE;
    style_bitmap bitmap={0};
    if(!style_bitmap_ensure(&bitmap,dc,(SIZE){w,h}))return FALSE;
    BOOL alpha=alpha_present(strip),ok=decode(strip,state,alpha,&bitmap);
    int x[4],y[4],dx[4],dy[4];
    slice(w,m.left,m.right,x);slice(h,m.top,m.bottom,y);
    slice(bounds.right-bounds.left,MulDiv(m.left,(int)dpi,96),MulDiv(m.right,(int)dpi,96),dx);
    slice(bounds.bottom-bounds.top,MulDiv(m.top,(int)dpi,96),MulDiv(m.bottom,(int)dpi,96),dy);
    BLENDFUNCTION blend={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    for(unsigned j=0;ok && j<3;++j){
        for(unsigned i=0;ok && i<3;++i){
            int sw=x[i+1]-x[i],sh=y[j+1]-y[j],dw=dx[i+1]-dx[i],dh=dy[j+1]-dy[j];
            if(!sw || !sh || !dw || !dh)continue;
            int left=bounds.left+dx[i],top=bounds.top+dy[j];
            ok=alpha || strip->transparent?
                AlphaBlend(dc,left,top,dw,dh,bitmap.dc,x[i],y[j],sw,sh,blend):
                StretchBlt(dc,left,top,dw,dh,bitmap.dc,x[i],y[j],sw,sh,SRCCOPY);
        }
    }
    style_bitmap_dispose(&bitmap);
    return ok;
}