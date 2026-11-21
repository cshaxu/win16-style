#include "menu_art.h"

void style_menu_draw_mark(HDC dc,const RECT *row,BOOL submenu,UINT dpi,COLORREF color,const CHAR *mask)
{
    if(!mask){
        int unit=max(1,MulDiv(1,(int)dpi,96)),extent=MulDiv(13,(int)dpi,96);
        RECT bounds=*row;
        if(submenu){
            bounds.left=bounds.right-extent;
            bounds.right-=unit;
        }else{
            bounds.left+=unit;
            bounds.right=bounds.left+extent;
        }
        DrawFrameControl(dc,&bounds,DFC_MENU,submenu?DFCS_MENUARROW:DFCS_MENUCHECK);
        return;
    }
    int width=submenu?7:14,height=submenu?11:14;
    int left=submenu?row->right-MulDiv(11,(int)dpi,96):row->left+MulDiv(2,(int)dpi,96);
    int top=(row->top+row->bottom-MulDiv(height,(int)dpi,96))/2;
    SetDCBrushColor(dc,color);
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    for(int y=0;y<height;++y)for(int x=0;x<width;++x)if(mask[y*(width+1)+x]=='0'){
        RECT pixel={left+MulDiv(x,(int)dpi,96),top+MulDiv(y,(int)dpi,96),
            left+MulDiv(x+1,(int)dpi,96),top+MulDiv(y+1,(int)dpi,96)};
        FillRect(dc,&pixel,brush);
    }
}

void style_menu_draw_command(HDC dc,const RECT *row,HBITMAP bitmap,int unit,COLORREF color)
{
    if(!bitmap)return;
    HPEN pen=CreatePen(PS_SOLID,unit,color);if(!pen)return;
    HGDIOBJ old_pen=SelectObject(dc,pen),old_brush=SelectObject(dc,GetStockObject(NULL_BRUSH));
    int x=row->left+5*unit,y=(row->top+row->bottom-8*unit)/2;
    if(bitmap==HBMMENU_POPUP_CLOSE){
        MoveToEx(dc,x,y,NULL);LineTo(dc,x+8*unit,y+8*unit);
        MoveToEx(dc,x+7*unit,y,NULL);LineTo(dc,x-unit,y+8*unit);
    }else if(bitmap==HBMMENU_POPUP_MINIMIZE){
        MoveToEx(dc,x,y+6*unit,NULL);LineTo(dc,x+8*unit,y+6*unit);
        MoveToEx(dc,x,y+7*unit,NULL);LineTo(dc,x+8*unit,y+7*unit);
    }else if(bitmap==HBMMENU_POPUP_MAXIMIZE){
        Rectangle(dc,x,y,x+8*unit,y+8*unit);
        MoveToEx(dc,x+unit,y+2*unit,NULL);LineTo(dc,x+7*unit,y+2*unit);
    }else if(bitmap==HBMMENU_POPUP_RESTORE){
        Rectangle(dc,x+2*unit,y,x+8*unit,y+6*unit);
        Rectangle(dc,x,y+2*unit,x+6*unit,y+8*unit);
    }
    if(old_brush && old_brush!=HGDI_ERROR)SelectObject(dc,old_brush);
    if(old_pen && old_pen!=HGDI_ERROR)SelectObject(dc,old_pen);
    DeleteObject(pen);
}

void style_menu_popup_marks(HDC dc,const RECT *row,UINT dpi,COLORREF ink,
    BOOL checked,BOOL submenu,const style_menu_art *art)
{
    if(art && !art->available)art=NULL;
    if(checked)style_menu_draw_mark(dc,row,FALSE,dpi,ink,art?&art->check[0][0]:NULL);
    if(submenu)style_menu_draw_mark(dc,row,TRUE,dpi,ink,art?&art->arrow[0][0]:NULL);
}
