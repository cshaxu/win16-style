#define _WIN32_WINNT 0x0A00
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
static unsigned draws,idles;
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data){
 (void)id;(void)data;
 if(m==WM_DRAWITEM && b && ((DRAWITEMSTRUCT*)b)->CtlType==ODT_MENU)++draws;
 if(m==WM_ENTERIDLE && a==MSGF_MENU){++idles;EndMenu();return 0;}
 return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b){
 if(m==WM_EXITMENULOOP){PostQuitMessage(0);return 0;}
 if(m==WM_TIMER){KillTimer(w,1);EndMenu();PostQuitMessage(0);return 0;}
 return DefWindowProcW(w,m,a,b);
}
int wmain(void){
 HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.hInstance=module,.lpfnWndProc=owner,.lpszClassName=L"MenuFirstPaintWitness"};
 if(!RegisterClassW(&cls))return 2;
 HMENU bar=CreateMenu(),popup=CreatePopupMenu();if(!bar||!popup)return 3;
 if(!AppendMenuW(popup,MF_STRING,101,L"&First")||!AppendMenuW(popup,MF_STRING,102,L"&Second")||!AppendMenuW(popup,MF_STRING,103,L"&Third")||!AppendMenuW(bar,MF_POPUP,(UINT_PTR)popup,L"&File"))return 4;
 HWND w=CreateWindowW(cls.lpszClassName,L"First paint",WS_OVERLAPPEDWINDOW,80,80,400,260,NULL,bar,module,NULL);if(!w)return 5;
 ShowWindow(w,SW_SHOW);UpdateWindow(w);if(!SetWindowSubclass(w,observe,1,0))return 6;
 RECT r;if(!GetMenuItemRect(w,bar,0,&r)||!SetTimer(w,1,3000,NULL))return 7;
 if(!PostMessageW(w,WM_NCLBUTTONDOWN,HTMENU,MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2)))return 8;
 MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 KillTimer(w,1);printf("MENU_INITIAL bits=%u draws=%u idle=%u\n",(unsigned)(sizeof(void*)*8),draws,idles);
 DestroyWindow(w);return idles&&draws>=3?0:1;
}
