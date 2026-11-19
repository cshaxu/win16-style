#include "caption_slots.h"
#include <limits.h>
#include <string.h>

BOOL style_caption_slots(const classic_button_group *group,const RECT *band,UINT dpi,
    SIZE dimensions,unsigned present,BOOL clip_right,RECT bounds[CLASSIC_BUTTON_CLOSE+1])
{
    if(!group || !band || !bounds || group->count>CLASSIC_BUTTON_CLOSE || !dpi || dpi>INT_MAX ||
       dimensions.cx<=0 || dimensions.cy<=0 || (int64_t)band->bottom-band->top<dimensions.cy)
        return FALSE;
    RECT result[CLASSIC_BUTTON_CLOSE+1]={0};
    int64_t top=band->top+((int64_t)band->bottom-band->top-dimensions.cy)/2;
    int left_margin=MulDiv(group->left_margin,(int)dpi,96),right_margin=MulDiv(group->right_margin,(int)dpi,96);
    if(left_margin<0 || right_margin<0)return FALSE;
    int64_t left=(int64_t)band->left+left_margin,right=(int64_t)band->right-right_margin;
    unsigned placed=0;
    for(unsigned side=CLASSIC_BUTTON_LEFT;side<=CLASSIC_BUTTON_RIGHT;++side){
        BOOL previous=FALSE;
        int gap_after=0;
        for(unsigned index=0;index<group->count;++index){
            const classic_button_slot slot=group->slots[side==CLASSIC_BUTTON_LEFT?
                index:group->count-1-index];
            if(slot.role<CLASSIC_BUTTON_SYSTEM || slot.role>CLASSIC_BUTTON_CLOSE ||
               slot.side>CLASSIC_BUTTON_RIGHT)return FALSE;
            unsigned bit=1u<<slot.role;
            if(slot.side!=side || !(present&bit))continue;
            if(placed&bit)return FALSE;
            int gap=MulDiv(slot.gap_before,(int)dpi,96);
            if(gap<0)return FALSE;
            int64_t begin,end;
            if(side==CLASSIC_BUTTON_LEFT){
                begin=left+(previous?gap:0);
                end=begin+dimensions.cx;
                left=end;
            }else{
                end=right-gap_after;
                if(clip_right)end=max(end,band->left);
                begin=end-dimensions.cx;
                if(begin<left && !clip_right)return FALSE;
                begin=max(begin,band->left);
                right=begin;
                gap_after=gap;
            }
            if(begin<LONG_MIN || end>LONG_MAX ||
               (!clip_right && end>band->right))return FALSE;
            result[slot.role]=(RECT){(LONG)begin,(LONG)top,(LONG)end,(LONG)(top+dimensions.cy)};
            placed|=bit;
            previous=TRUE;
        }
    }
    if(placed!=present)return FALSE;
    memcpy(bounds,result,sizeof(result));
    return TRUE;
}
