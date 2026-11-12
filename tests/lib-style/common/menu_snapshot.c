#include "lib-style/common/menu_snapshot.h"
#include <stdio.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"menu snapshot line %d error %lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
static HMENU menu;
static DWORD start;
static BOOL checked,passed;
static void CALLBACK poll_menu(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)window;(void)message;(void)timer;
    style_menu_row row={0};BOOL cancelled=FALSE;
    if(style_menu_read_row(menu,0,&row,&cancelled)){
        passed=!lstrcmpW(row.text,L"&File") && row.rect.right>row.rect.left && !(row.type&MFT_OWNERDRAW) &&
            !style_menu_read_row(menu,1,&row,NULL);
        cancelled=TRUE;passed=passed && !style_menu_read_row(menu,0,&row,&cancelled);
        checked=TRUE;EndMenu();
    }else if(time-start>2000)EndMenu();
}
int main(void)
{
    menu=CreatePopupMenu();CHECK(menu);
    CHECK(AppendMenuW(menu,MF_STRING,1,L"&File"));
    CHECK(AppendMenuW(menu,MF_OWNERDRAW,2,(LPCWSTR)(UINT_PTR)1));
    HWND window=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,20,20,240,160,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(window);
    ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);start=GetTickCount();
    CHECK(SetTimer(window,1,50,poll_menu));
    TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,40,40,0,window,NULL);
    KillTimer(window,1);DestroyMenu(menu);DestroyWindow(window);CHECK(checked && passed);
    puts("MENU_SNAPSHOT_OK text=1 geometry=1 ownerdraw=1 cancel=1");return 0;
}
