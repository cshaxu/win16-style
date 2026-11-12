#include "system_menu.h"
#include "lib-style/common/menu_snapshot.h"
#include "lib-style/common/menu_input.h"
#include "lib-style/common/present.h"
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

typedef struct menu_scope menu_scope;
struct menu_scope { HWND owner,popup;HMENU menu;menu_scope *previous;unsigned calls;int selection;BOOL detached,painted; };
static _Thread_local menu_scope *scopes;
static _Thread_local HHOOK message_hook;
static LRESULT CALLBACK menu_popup_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static int selected_item(HMENU menu);

static void popup_paint(HWND window,menu_scope *scope,HDC target,BOOL window_coordinates)
{
    if(scope->detached || !scope->menu)return;
    HDC dc=target?target:GetDC(window);if(!dc)return;
    BOOL painted=FALSE;
    RECT client;
    if(!target && GetClientRect(window,&client)){
        style_present_buffer buffer;
        BOOL ready=style_present_buffer_begin(&buffer,dc,&client);
        if(ready)painted=win2k_system_menu_paint(window,scope->menu,buffer.memory,FALSE);
        BOOL committed=style_present_buffer_end(&buffer,ready && painted && !scope->detached);
        painted=painted && committed;
    }
    if(!painted && !scope->detached)
        painted=win2k_system_menu_paint(window,scope->menu,dc,window_coordinates);
    if(!target)ReleaseDC(window,dc);
    if(painted && !target && !scope->detached){scope->selection=selected_item(scope->menu);scope->painted=TRUE;}
}
static int selected_item(HMENU menu)
{
    int count=GetMenuItemCount(menu);
    for(int i=0;i<count;++i){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE};
        if(GetMenuItemInfoW(menu,(UINT)i,TRUE,&item) && (item.fState&MFS_HILITE))return i;}
    return -1;
}
static LRESULT CALLBACK menu_popup_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    menu_scope *scope=(menu_scope *)data;(void)id;++scope->calls;
    BOOL mouse=message==WM_MOUSEMOVE || message==WM_NCMOUSEMOVE;
    int before=mouse && scope->menu?selected_item(scope->menu):-1;
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    if(!scope->detached){
        if(message==WM_NCDESTROY){scope->popup=NULL;RemoveWindowSubclass(window,menu_popup_proc,(UINT_PTR)menu_popup_proc);}
        else if(message==WM_PRINT && (lp&PRF_CLIENT))popup_paint(window,scope,(HDC)wp,TRUE);
        else if(message==WM_PAINT || message==WM_WINDOWPOSCHANGED || message==WM_KEYDOWN ||
                (mouse && scope->menu && before!=selected_item(scope->menu)))popup_paint(window,scope,NULL,FALSE);
    }
    if(!--scope->calls && scope->detached)free(scope);
    return result;
}
static LRESULT CALLBACK menu_message(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp && scopes){
        const CWPSTRUCT *event=(const CWPSTRUCT *)lp;
        if(style_menu_popup_observation(event->message)){
            menu_scope *scope=scopes;
            if(scope->menu && !scope->popup && style_menu_popup_is(event->hwnd,scope->menu) &&
               SetWindowSubclass(event->hwnd,menu_popup_proc,(UINT_PTR)menu_popup_proc,(DWORD_PTR)scope))scope->popup=event->hwnd;
        }
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
void win2k_system_menu_begin(HWND owner)
{
    menu_scope *scope=calloc(1,sizeof(*scope));if(!scope)return;
    if(!scopes){message_hook=SetWindowsHookExW(WH_CALLWNDPROC,menu_message,NULL,GetCurrentThreadId());
        if(!message_hook){free(scope);return;}}
    scope->owner=owner;scope->previous=scopes;scopes=scope;
}
void win2k_system_menu_set(HWND owner,HMENU menu)
{
    if(scopes && scopes->owner==owner)scopes->menu=menu;
}
void win2k_system_menu_idle(HWND owner)
{
    menu_scope *scope=scopes;
    if(scope && scope->owner==owner && scope->popup && scope->menu &&
       (!scope->painted || scope->selection!=selected_item(scope->menu))){
        ++scope->calls;popup_paint(scope->popup,scope,NULL,FALSE);
        if(!--scope->calls && scope->detached)free(scope);
    }
}
void win2k_system_menu_end(HWND owner,BOOL all)
{
    menu_scope **link=&scopes;
    while(*link){menu_scope *scope=*link;
        if(scope->owner!=owner){link=&scope->previous;continue;}
        *link=scope->previous;scope->detached=TRUE;
        if(scope->popup)RemoveWindowSubclass(scope->popup,menu_popup_proc,(UINT_PTR)menu_popup_proc);
        if(!scope->calls)free(scope);
        if(!all)break;
    }
    if(!scopes && message_hook){UnhookWindowsHookEx(message_hook);message_hook=NULL;}
}
