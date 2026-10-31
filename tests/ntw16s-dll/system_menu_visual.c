#define _WIN32_WINNT 0x0A00
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
static HMENU system_menu;static HWND popup;static UINT before_type,seen_type;static ULONG_PTR before_data,seen_data;static BOOL seen,checked,selected,custom;static COLORREF sample=CLR_INVALID;static unsigned idles,custom_draws;
static BOOL CALLBACK find_popup(HWND w,LPARAM unused){(void)unused;WCHAR cls[32];MENUBARINFO mbi={.cbSize=sizeof(mbi)};if(IsWindowVisible(w)&&GetClassNameW(w,cls,32)&&!lstrcmpW(cls,L"#32768")&&GetMenuBarInfo(w,OBJID_CLIENT,0,&mbi)&&mbi.hMenu==system_menu){popup=w;return FALSE;}return TRUE;}
static BOOL sample_menu(void){
    if(!EnumThreadWindows(GetCurrentThreadId(),find_popup,0)&&!popup)return FALSE;
    int count=GetMenuItemCount(system_menu),selected_index=-1;RECT row,window;
    for(int i=0;i<count;++i){MENUITEMINFOW m={.cbSize=sizeof(m),.fMask=MIIM_STATE};if(GetMenuItemInfoW(system_menu,(UINT)i,TRUE,&m)&&(m.fState&MFS_HILITE)){selected_index=i;break;}}
    if(selected_index<0||!GetMenuItemRect(NULL,system_menu,(UINT)selected_index,&row)||!GetWindowRect(popup,&window))return FALSE;
    int width=window.right-window.left,height=window.bottom-window.top;if(width<=0||height<=0||width>2048||height>2048)return FALSE;
    HDC screen=GetDC(NULL),dc=screen?CreateCompatibleDC(screen):NULL;HBITMAP bitmap=dc?CreateCompatibleBitmap(screen,width,height):NULL;HGDIOBJ prev=bitmap?SelectObject(dc,bitmap):NULL;
    if(screen)ReleaseDC(NULL,screen);
    if(!prev||prev==HGDI_ERROR){if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);return FALSE;}
    SendMessageW(popup,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
    sample=GetPixel(dc,row.left-window.left+4,(row.top+row.bottom)/2-window.top);
    SelectObject(dc,prev);DeleteObject(bitmap);DeleteDC(dc);selected=TRUE;
    MENUITEMINFOW m={.cbSize=sizeof(m),.fMask=MIIM_FTYPE|MIIM_DATA};BOOL read=GetMenuItemInfoW(system_menu,0,TRUE,&m);checked=read&&m.fType==before_type&&m.dwItemData==before_data;
    printf("SYSTEM_DURING read=%d type=%x expected=%x data=%llu expecteddata=%llu\n",read,m.fType,before_type,(unsigned long long)m.dwItemData,(unsigned long long)before_data);
    return sample!=CLR_INVALID;
}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b){
    if(m==WM_INITMENUPOPUP&&HIWORD(b)){MENUITEMINFOW i={.cbSize=sizeof(i),.fMask=MIIM_FTYPE|MIIM_DATA};if(GetMenuItemInfoW((HMENU)a,0,TRUE,&i)){seen=TRUE;seen_type=i.fType;seen_data=i.dwItemData;}}
    if(m==WM_ENTERIDLE&&a==MSGF_MENU){++idles;if(sample_menu()){EndMenu();return 0;}if(idles==1)PostMessageW(w,WM_KEYDOWN,VK_DOWN,1);}
    if(m==WM_TIMER&&a==1){KillTimer(w,1);EndMenu();return 0;}
    if(m==WM_EXITMENULOOP)PostQuitMessage(0);
    if(m==WM_MEASUREITEM&&custom){MEASUREITEMSTRUCT *x=(MEASUREITEMSTRUCT *)b;if(x&&x->CtlType==ODT_MENU&&x->itemID==12345){x->itemWidth=80;x->itemHeight=20;return TRUE;}}
    if(m==WM_DRAWITEM&&custom){DRAWITEMSTRUCT *x=(DRAWITEMSTRUCT *)b;if(x&&x->CtlType==ODT_MENU&&x->itemID==12345){++custom_draws;FillRect(x->hDC,&x->rcItem,GetSysColorBrush(COLOR_MENU));return TRUE;}}
    return DefWindowProcW(w,m,a,b);
}
int wmain(int argc,WCHAR **argv){
    if(argc!=2)return 2;
    custom=!lstrcmpW(argv[1],L"custom");if(!custom&&lstrcmpW(argv[1],L"normal"))return 2;
    WNDCLASSW cls={.lpfnWndProc=proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"Win16SysMenuWitness"};if(!RegisterClassW(&cls))return 4;
    HMENU bar=CreateMenu(),sub=CreatePopupMenu();if(!bar||!sub||!AppendMenuW(sub,MF_STRING,42,L"Item")||!AppendMenuW(bar,MF_POPUP,(UINT_PTR)sub,L"&File"))return 5;
    HWND w=CreateWindowW(cls.lpszClassName,L"System menu witness",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,30,30,360,200,NULL,bar,cls.hInstance,NULL);if(!w)return 5;
    ShowWindow(w,SW_SHOWNORMAL);UpdateWindow(w);
    system_menu=GetSystemMenu(w,FALSE);if(!system_menu)return 6;if(custom&&!AppendMenuW(system_menu,MF_OWNERDRAW,12345,0))return 7;
    MENUITEMINFOW i={.cbSize=sizeof(i),.fMask=MIIM_FTYPE|MIIM_DATA};if(!GetMenuItemInfoW(system_menu,0,TRUE,&i))return 8;before_type=i.fType;before_data=i.dwItemData;
    SetActiveWindow(w);SetFocus(w);if(!SetTimer(w,1,3000,NULL)||!PostMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' '))return 9;
    MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}KillTimer(w,1);
    BOOL intact=GetMenuItemInfoW(system_menu,0,TRUE,&i)&&i.fType==before_type&&i.dwItemData==before_data;
    BOOL style=sample==RGB(164,200,240);printf("SYSTEM_MENU bits=%u custom=%d seen=%d unchanged=%d checked=%d selected=%d blue=%d sample=%06lx idles=%u custom_draws=%u\n",(unsigned)(sizeof(void*)*8),custom,seen,seen_type==before_type&&seen_data==before_data&&intact,checked,selected,style,(DWORD)sample,idles,custom_draws);
    DestroyWindow(w);return seen&&seen_type==before_type&&seen_data==before_data&&intact&&selected&&(custom?custom_draws>0:style)?0:1;
}

