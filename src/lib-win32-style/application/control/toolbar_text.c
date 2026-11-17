#include "toolbar_text.h"
#include "control_protocol.h"
#include <wchar.h>
BOOL style_toolbar_text_read(HWND w,int index,style_toolbar_text_item *item,BOOL (*accept)(HWND,const TBBUTTON *))
{
    TBBUTTONINFOW info={0};int capacity=ARRAYSIZE(item->text);
    ZeroMemory(&item->button,sizeof(item->button));
    if(!SendMessageW(w,TB_GETBUTTON,index,(LPARAM)&item->button) || !accept(w,&item->button))return FALSE;
    /* No unbounded TB_GETBUTTONTEXT preflight: text may change reentrantly. */
    info.cbSize=sizeof(info);info.dwMask=TBIF_TEXT;info.pszText=item->text;info.cchText=capacity;
    item->text[0]=0;item->text[capacity-1]=0;
    if(SendMessageW(w,TB_GETBUTTONINFOW,item->button.idCommand,(LPARAM)&info)<0 || !item->text[0] ||
        wcsnlen(item->text,(size_t)capacity)>=(size_t)(capacity-1) || !SendMessageW(w,TB_GETITEMRECT,index,(LPARAM)&item->rect) ||
        item->rect.right<=item->rect.left || item->rect.bottom<=item->rect.top)return FALSE;
    return TRUE;
}
BOOL style_toolbar_text_supported(HWND w,BOOL (*accept)(HWND,const TBBUTTON *))
{
    LRESULT count=SendMessageW(w,TB_BUTTONCOUNT,0,0);style_toolbar_text_item item;
    if(count<0 || count>1024 || (style_control_protocol_runtime_toolbar(w) && count>64))return FALSE;
    for(int i=0;i<count;++i)if(!style_toolbar_text_read(w,i,&item,accept) ||
        (style_control_protocol_runtime_toolbar(w) && SendMessageW(w,TB_COMMANDTOINDEX,item.button.idCommand,0)!=i))return FALSE;
    return TRUE;
}
BOOL style_toolbar_text_find(HWND w,DWORD_PTR command,style_toolbar_text_item *item,BOOL (*accept)(HWND,const TBBUTTON *))
{
    LRESULT count=SendMessageW(w,TB_BUTTONCOUNT,0,0);
    for(int i=0;i<count;++i){
        if(!style_toolbar_text_read(w,i,item,accept))return FALSE;
        if((DWORD_PTR)item->button.idCommand==command)return TRUE;
    }
    return FALSE;
}
