#ifndef STYLE_PIXEL_GRID_H
#define STYLE_PIXEL_GRID_H
#include <windows.h>

/* Rows use ASCII palette indices; NULL colors retains the current DC brush.
 * The transparent index may be -1. Scale ratios apply independently per axis. */
typedef struct style_pixel_grid {
    const char *pixels;
    int width,height,stride,transparent;
    const COLORREF *colors;
    POINT origin;
    SIZE scale,divisor;
} style_pixel_grid;

static inline void style_paint_pixel_grid(HDC dc,const style_pixel_grid *grid)
{
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    for(int y=0;y<grid->height;++y)for(int x=0;x<grid->width;++x){
        int index=grid->pixels[y*grid->stride+x]-'0';
        if(index==grid->transparent)continue;
        if(grid->colors)SetDCBrushColor(dc,grid->colors[index]);
        RECT pixel={grid->origin.x+MulDiv(x,grid->scale.cx,grid->divisor.cx),
                    grid->origin.y+MulDiv(y,grid->scale.cy,grid->divisor.cy),
                    grid->origin.x+MulDiv(x+1,grid->scale.cx,grid->divisor.cx),
                    grid->origin.y+MulDiv(y+1,grid->scale.cy,grid->divisor.cy)};
        FillRect(dc,&pixel,brush);
    }
}
#endif
