#ifndef STYLE_CONTROL_PROGRESS_PAINT_H
#define STYLE_CONTROL_PROGRESS_PAINT_H
#include "progress_snapshot.h"
#include <stdint.h>

enum {
    STYLE_PROGRESS_DEFINED=1, STYLE_PROGRESS_SCALE_DPI=2, STYLE_PROGRESS_BORDER=4,
    STYLE_PROGRESS_ROUND=8, STYLE_PROGRESS_FULL_BLOCKS=16
};
typedef struct style_progress_layout {
    uint8_t flags,inset,clearance,gap,block_part,block_whole;
} style_progress_layout;
typedef struct style_progress_ink {
    COLORREF background,fill,border;
} style_progress_ink;

#define STYLE_PROGRESS_OUTLINED {STYLE_PROGRESS_DEFINED|STYLE_PROGRESS_SCALE_DPI|STYLE_PROGRESS_BORDER,1,0,2,3,4}
#define STYLE_PROGRESS_INSET {STYLE_PROGRESS_DEFINED|STYLE_PROGRESS_ROUND|STYLE_PROGRESS_FULL_BLOCKS,1,1,2,2,3}

BOOL style_progress_paint(HDC,const style_progress_snapshot *,DWORD,UINT,const style_progress_layout *,style_progress_ink);
#endif
