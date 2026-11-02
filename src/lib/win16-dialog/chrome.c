/* Apply presentation services to controls of system classic templates. */
#include "chrome.h"
#include <dlgs.h>
BOOL win16_dialog_color_swatch_edge(HWND dialog,HDC dc,const RECT *rect)
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
BOOL win16_dialog_is_font_template(HWND window)
{
    WCHAR cls[32];HWND first=GetDlgItem(window,1136),second=GetDlgItem(window,1137),third=GetDlgItem(window,1138);
    return first && second && third && GetClassNameW(window,cls,32) && !lstrcmpW(cls,L"#32770");
}
void win16_dialog_color_initialized(HWND window,void (*found)(HWND),void (*remove_surface)(HWND))
{
    WCHAR cls[32];HWND basic=GetDlgItem(window,720),custom=GetDlgItem(window,721);
    if(basic && custom && GetClassNameW(window,cls,32) && !lstrcmpW(cls,L"#32770")){
        found(window);
        const int ids[]={720,721,710,702,709};
        for(unsigned i=0;i<sizeof(ids)/sizeof(ids[0]);++i){
            HWND surface=GetDlgItem(window,ids[i]);
            if(surface && GetParent(surface)==window && GetClassNameW(surface,cls,32) && !lstrcmpiW(cls,L"Static"))
                remove_surface(surface);
        }
    }
}
void win16_dialog_old_chrome(HWND window,const win16_dialog_chrome_services *services)
{
    static int owner;
    const int list_ids[]={lst1,lst2};
    for(unsigned i=0;i<sizeof(list_ids)/sizeof(list_ids[0]);++i){
        HWND list=GetDlgItem(window,list_ids[i]);WCHAR cls[32];
        DWORD style=list?(DWORD)GetWindowLongPtrW(list,GWL_STYLE):0;
        if(list && GetParent(list)==window && GetClassNameW(list,cls,32) && !lstrcmpiW(cls,L"ListBox") &&
            (style&LBS_OWNERDRAWFIXED) && (style&LBS_HASSTRINGS) && !(style&LBS_NODATA) &&
            services->grid(list,&owner) && !services->scroll(list))services->remove(list,&owner);
    }
    HWND drives=GetDlgItem(window,cmb2);
    if(drives && GetParent(drives)==window)(void)services->combo(drives,&owner);
}
void win16_dialog_font_chrome(HWND window,const win16_dialog_chrome_services *services)
{
    static int owner;
    for(int id=cmb1;id<=cmb5;++id){
        HWND combo=GetDlgItem(window,id);
        if(combo && GetParent(combo)==window)(void)services->combo(combo,&owner);
    }
}
