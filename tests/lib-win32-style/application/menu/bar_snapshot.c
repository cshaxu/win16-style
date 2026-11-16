#include "lib-win32-style/application/menu/bar_snapshot.h"
#include <stdio.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"BAR_SNAPSHOT line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static unsigned measured,drawn;
static BOOL replace_on_measure;
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_MEASUREITEM){
        MEASUREITEMSTRUCT *item=(MEASUREITEMSTRUCT *)b;
        ++measured;item->itemWidth=50;item->itemHeight=20;
        if(replace_on_measure){MENUITEMINFOW value={.cbSize=sizeof(value),.fMask=MIIM_DATA,.dwItemData=99};SetMenuItemInfoW(GetMenu(w),1,TRUE,&value);}
        return TRUE;
    }
    if(m==WM_DRAWITEM){
        DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT *)b;
        if(item->itemData!=77)return FALSE;
        ++drawn;SetTextColor(item->hDC,RGB(5,6,7));SetViewportOrgEx(item->hDC,100,200,NULL);return TRUE;
    }
    return DefWindowProcW(w,m,a,b);
}
static BOOL measure(void *context,HDC dc,const MENUITEMINFOW *item,SIZE *size)
{(void)dc;(void)item;*size=(SIZE){*(int *)context,20};return TRUE;}
static style_menu_bar_inclusion include(const MENUITEMINFOW *item,UINT *flags)
{(void)flags;return item->wID==3?STYLE_MENU_BAR_SKIP:STYLE_MENU_BAR_INCLUDE;}
int main(void)
{
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicBarSnapshot",.lpfnWndProc=procedure};CHECK(RegisterClassW(&cls));
    HMENU menu=CreateMenu();CHECK(menu);
    CHECK(AppendMenuW(menu,MF_STRING,1,L"&File"));
    CHECK(AppendMenuW(menu,MF_OWNERDRAW,2,(LPCWSTR)(ULONG_PTR)77));
    CHECK(AppendMenuW(menu,MF_STRING,3,L"Skipped"));
    CHECK(AppendMenuW(menu,MF_STRING|MF_GRAYED,4,L"&Disabled"));
    HWND w=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,30,30,400,200,NULL,menu,cls.hInstance,NULL);CHECK(w);
    int width=40;style_menu_bar_profile custom={include,measure,FALSE};style_menu_bar_snapshot bar;
    CHECK(style_menu_bar_read(w,menu,(POINT){5,10},300,&custom,&width,&bar));
    CHECK(bar.count==3 && bar.height==20 && bar.rects[0].left==5 && bar.rects[1].left==45 && bar.items[2].position==3);
    CHECK(bar.items[1].data==77 && style_menu_bar_item_current(&bar,1) && measured>0);
    CHECK(style_menu_bar_mnemonic(&bar,L'f',TRUE)==0 && style_menu_bar_mnemonic(&bar,L'd',TRUE)==-1);
    MENUITEMINFOW text={.cbSize=sizeof(text),.fMask=MIIM_STRING,.dwTypeData=L"Changed"};CHECK(SetMenuItemInfoW(menu,0,TRUE,&text));
    CHECK(!lstrcmpW(bar.items[0].text,L"&File"));
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);SetTextColor(dc,RGB(1,2,3));POINT origin;CHECK(GetViewportOrgEx(dc,&origin));
    CHECK(style_menu_bar_draw_owned(&bar,1,dc) && drawn==1 && GetTextColor(dc)==RGB(1,2,3));
    POINT after;CHECK(GetViewportOrgEx(dc,&after) && after.x==origin.x && after.y==origin.y);DeleteDC(dc);
    style_menu_bar_free(&bar);style_menu_bar_free(&bar);CHECK(!bar.items && !bar.rects && !bar.count);
    style_menu_bar_profile native={.include=include,.native_rects=TRUE};unsigned prior=measured;
    CHECK(style_menu_bar_read(w,menu,(POINT){999,999},0,&native,NULL,&bar));
    RECT real;CHECK(GetMenuItemRect(w,menu,0,&real) && EqualRect(&real,&bar.rects[0]) && prior==measured);style_menu_bar_free(&bar);
    replace_on_measure=TRUE;
    CHECK(!style_menu_bar_read(w,menu,(POINT){0,0},300,&custom,&width,&bar));CHECK(!bar.items && !bar.rects && !bar.count);
    MENUITEMINFOW data={.cbSize=sizeof(data),.fMask=MIIM_DATA};CHECK(GetMenuItemInfoW(menu,1,TRUE,&data) && data.dwItemData==99);
    CHECK(DestroyWindow(w));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("BAR_SNAPSHOT copied/native geometry, borrowed data, owner callbacks, DC isolation and failed identity cleanup PASS");return 0;
}
