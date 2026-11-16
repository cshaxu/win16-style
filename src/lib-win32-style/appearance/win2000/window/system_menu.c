#include "system_menu.h"
#include "lib-win32-style/application/menu/menu_snapshot.h"
#include "lib-win32-style/application/menu/popup.h"
#include "lib-win32-style/application/graphics/present.h"
#include <oleacc.h>
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>

static void system_glyph(HDC dc,const RECT *row,HBITMAP bitmap,int unit,COLORREF color)
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

BOOL win2k_system_menu_paint(HWND popup,HMENU menu,HDC dc,BOOL window_coordinates)
{
    MENUBARINFO info={.cbSize=sizeof(info)};
    if(!popup || !menu || !dc || !GetMenuBarInfo(popup,OBJID_CLIENT,0,&info) || info.hMenu!=menu)return FALSE;
    int count=GetMenuItemCount(menu);
    if(count<1 || count>64)return FALSE;
    style_menu_row *rows=calloc((size_t)count,sizeof(*rows));if(!rows)return FALSE;
    BOOL valid=TRUE;
    for(int i=0;i<count;++i){
        if(!style_menu_read_row(menu,(UINT)i,&rows[i],NULL)){valid=FALSE;break;}
    }
    int saved=valid?SaveDC(dc):0;
    if(!saved){free(rows);return FALSE;}
    UINT dpi=GetDpiForWindow(popup);if(!dpi)dpi=96;
    int unit=max(1,MulDiv(1,(int)dpi,96));
    HFONT font=CreateFontW(-MulDiv(8,(int)dpi,72),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    HFONT bold=CreateFontW(-MulDiv(8,(int)dpi,72),0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    HFONT normal=font?font:(HFONT)GetCurrentObject(dc,OBJ_FONT);
    if(normal)SelectObject(dc,normal);
    SetBkMode(dc,TRANSPARENT);
    POINT origin={0,0};
    if(!style_menu_client_offset(popup,window_coordinates,&origin))valid=FALSE;
    for(int i=0;valid && i<count;++i){
        RECT r=rows[i].rect;MapWindowPoints(NULL,popup,(POINT *)&r,2);OffsetRect(&r,origin.x,origin.y);
        BOOL selected=(rows[i].state&MFS_HILITE)!=0;
        HBRUSH background=CreateSolidBrush(selected?RGB(0,0,128):RGB(212,208,200));
        if(!background){valid=FALSE;break;}
        FillRect(dc,&r,background);DeleteObject(background);
        if(rows[i].type&MFT_SEPARATOR){
            RECT line={r.left+unit,r.top+(r.bottom-r.top)/2,r.right-unit,r.top+(r.bottom-r.top)/2+unit};
            HBRUSH shadow=CreateSolidBrush(RGB(128,128,128));
            if(shadow){FillRect(dc,&line,shadow);DeleteObject(shadow);}continue;
        }
        if((rows[i].state&MFS_DEFAULT) && bold)SelectObject(dc,bold);
        else if(normal)SelectObject(dc,normal);
        SetTextColor(dc,(rows[i].state&MFS_DISABLED)?RGB(128,128,128):selected?RGB(255,255,255):RGB(0,0,0));
        system_glyph(dc,&r,rows[i].bitmap,unit,(rows[i].state&MFS_DISABLED)?RGB(128,128,128):selected?RGB(255,255,255):RGB(0,0,0));
        RECT label=r;label.left+=MulDiv(22,(int)dpi,96);label.right-=MulDiv(16,(int)dpi,96);
        WCHAR *tab=wcschr(rows[i].text,L'\t');
        if(tab){*tab=0;RECT shortcut=label;DrawTextW(dc,tab+1,-1,&shortcut,DT_SINGLELINE|DT_VCENTER|DT_RIGHT|DT_NOPREFIX);}
        DrawTextW(dc,rows[i].text,-1,&label,DT_SINGLELINE|DT_VCENTER|DT_LEFT);
        if(rows[i].state&MFS_CHECKED){RECT mark=r;mark.left+=unit;mark.right=mark.left+MulDiv(13,(int)dpi,96);
            DrawFrameControl(dc,&mark,DFC_MENU,DFCS_MENUCHECK);}
        if(rows[i].submenu){RECT arrow=r;arrow.left=arrow.right-MulDiv(13,(int)dpi,96);arrow.right-=unit;
            DrawFrameControl(dc,&arrow,DFC_MENU,DFCS_MENUARROW);}
    }
    RestoreDC(dc,saved);if(bold)DeleteObject(bold);if(font)DeleteObject(font);free(rows);return valid;
}

typedef struct menu_presentation { HMENU menu; } menu_presentation;
static BOOL accept_popup(void *context,HWND window)
{
    menu_presentation *s=context;
    return s->menu && style_menu_popup_is(window,s->menu);
}
static BOOL selected_item(style_menu_popup *popup,ULONGLONG *selected)
{
    menu_presentation *s=style_menu_popup_context(popup);int count=GetMenuItemCount(s->menu);
    if(count<0)return FALSE;
    *selected=0;
    for(int i=0;i<count;++i){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE};
        if(GetMenuItemInfoW(s->menu,(UINT)i,TRUE,&item) && (item.fState&MFS_HILITE)){
            *selected=(ULONGLONG)i+1;break;}}
    return TRUE;
}
static BOOL popup_paint(style_menu_popup *popup,HDC target,BOOL window_coordinates)
{
    HWND window=style_menu_popup_window(popup);menu_presentation *s=style_menu_popup_context(popup);
    if(!s->menu)return FALSE;
    HDC dc=target?target:GetDC(window);if(!dc)return FALSE;
    BOOL painted=FALSE;RECT client;
    if(!target && GetClientRect(window,&client)){
        style_present_buffer buffer;
        BOOL ready=style_present_buffer_begin(&buffer,dc,&client,TRUE);
        if(ready)painted=win2k_system_menu_paint(window,s->menu,buffer.memory,FALSE);
        BOOL committed=style_present_buffer_end(&buffer,ready && painted && style_menu_popup_live(popup));
        painted=painted && committed;
    }
    if(!painted && style_menu_popup_live(popup))painted=win2k_system_menu_paint(window,s->menu,dc,window_coordinates);
    if(!target)ReleaseDC(window,dc);
    return painted;
}
static const style_menu_popup_ops popup_ops={accept_popup,NULL,popup_paint,selected_item,free,FALSE,TRUE};
static style_menu_popup_scope *begin_menu(void *context,HWND owner)
{
    (void)context;menu_presentation *s=calloc(1,sizeof(*s));if(!s)return NULL;
    style_menu_popup_scope *scope=style_menu_popup_begin(owner,&popup_ops,s);
    if(!scope)free(s);
    return scope;
}
static void set_system(void *context,style_menu_popup_scope *scope,HMENU menu)
{
    (void)context;menu_presentation *s=style_menu_popup_scope_context(scope);
    if(s)s->menu=menu;
}
const style_menu_owner_ops win2k_system_menu_operations={.begin=begin_menu,.system=set_system};
