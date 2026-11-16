#include "classic_dialog_controls.h"
#include <dlgs.h>
BOOL style_classic_color_template(HWND window)
{
    WCHAR cls[32];
    return GetDlgItem(window,720) && GetDlgItem(window,721) &&
        GetClassNameW(window,cls,32) && !lstrcmpW(cls,L"#32770");
}
BOOL style_classic_color_swatch(HWND dialog,HDC dc,const RECT *rect)
{
    if(!dialog || !dc || !rect || WindowFromDC(dc)!=dialog)return FALSE;
    for(int id=720;id<=721;++id){HWND grid=GetDlgItem(dialog,id);RECT bounds;
        if(grid && GetParent(grid)==dialog && IsWindowVisible(grid) && GetWindowRect(grid,&bounds)){
            MapWindowPoints(NULL,dialog,(POINT *)&bounds,2);
            if(rect->left>=bounds.left && rect->top>=bounds.top && rect->right<=bounds.right && rect->bottom<=bounds.bottom &&
               rect->right>rect->left+2 && rect->bottom>rect->top+2)return TRUE;
        }
    }
    return FALSE;
}
void style_classic_color_controls(HWND window,const int *ids,unsigned count,void (*visit)(HWND))
{
    WCHAR cls[32];
    for(unsigned i=0;i<count;++i){
        HWND surface=GetDlgItem(window,ids[i]);
        if(surface && GetParent(surface)==window && GetClassNameW(surface,cls,32) && !lstrcmpiW(cls,L"Static"))visit(surface);
    }
}
BOOL style_classic_font_template(HWND window)
{
    WCHAR cls[32];HWND first=GetDlgItem(window,1136),second=GetDlgItem(window,1137),third=GetDlgItem(window,1138);
    return first && second && third && GetClassNameW(window,cls,32) && !lstrcmpW(cls,L"#32770");
}
void style_classic_file_controls(HWND window,const void *owner,const style_classic_dialog_controls *services)
{
    const int list_ids[]={lst1,lst2};
    for(unsigned i=0;i<sizeof(list_ids)/sizeof(list_ids[0]);++i){
        HWND list=GetDlgItem(window,list_ids[i]);WCHAR cls[32];
        DWORD style=list?(DWORD)GetWindowLongPtrW(list,GWL_STYLE):0;
        if(list && GetParent(list)==window && GetClassNameW(list,cls,32) && !lstrcmpiW(cls,L"ListBox") &&
            (style&LBS_OWNERDRAWFIXED) && (style&LBS_HASSTRINGS) && !(style&LBS_NODATA) &&
            services->grid(list,owner) && !services->scroll(list))services->remove(list,owner);
    }
    HWND drives=GetDlgItem(window,cmb2);
    if(drives && GetParent(drives)==window)(void)services->combo(drives,owner);
}
void style_classic_font_controls(HWND window,const void *owner,const style_classic_dialog_controls *services)
{
    for(int id=cmb1;id<=cmb5;++id){
        HWND combo=GetDlgItem(window,id);
        if(combo && GetParent(combo)==window)(void)services->combo(combo,owner);
    }
}
