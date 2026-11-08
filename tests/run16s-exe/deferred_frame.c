#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
typedef BOOL (__cdecl *query_fn)(HWND);
static HWND destroy_during_timer,created_timer;
static LRESULT CALLBACK destroy_on_timer_create(int code,WPARAM wp,LPARAM lp)
{
    if(code==HCBT_CREATEWND && destroy_during_timer){
        WCHAR name[32]={0};GetClassNameW((HWND)wp,name,32);
        if(!lstrcmpiW(name,L"Static")){HWND root=destroy_during_timer;destroy_during_timer=NULL;created_timer=(HWND)wp;DestroyWindow(root);}
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_NCCALCSIZE)return 0;if(m==WM_ERASEBKGND)return 1;return DefWindowProcW(w,m,a,b);}
static int top(HWND w){RECT r;POINT p={0};if(!GetWindowRect(w,&r)||!ClientToScreen(w,&p))return -1;return (int)(p.y-r.top);}
static void pump(DWORD ms){ULONGLONG end=GetTickCount64()+ms;MSG m;do{while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}while(GetTickCount64()<end);}
static unsigned active_pixels(HWND w)
{
    RECT r;if(!GetWindowRect(w,&r))return 0;int width=r.right-r.left,height=r.bottom-r.top;
    HDC screen=GetDC(NULL),memory=screen?CreateCompatibleDC(screen):NULL;
    HBITMAP bitmap=memory?CreateCompatibleBitmap(screen,width,height):NULL;unsigned count=0;
    if(bitmap){HGDIOBJ old=SelectObject(memory,bitmap);SendMessageW(w,WM_PRINT,(WPARAM)memory,PRF_NONCLIENT);
        for(int y=0;y<height&&y<50;y++)for(int x=0;x<width;x++)if(GetPixel(memory,x,y)==RGB(164,200,240))count++;
        SelectObject(memory,old);DeleteObject(bitmap);}
    if(memory)DeleteDC(memory);
    if(screen)ReleaseDC(NULL,screen);
    return count;
}
int main(int argc,char **argv)
{
    WCHAR deskname[64];swprintf(deskname,64,L"Win16DeferFixture-%lu",GetCurrentProcessId());
    HDESK desk=CreateDesktopW(deskname,NULL,NULL,0,GENERIC_ALL,NULL);if(!desk||!SetThreadDesktop(desk))return 2;
    HMODULE module=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    query_fn attached=module?(query_fn)(uintptr_t)GetProcAddress(module,"Ntw16sWindowAttached"):NULL;
    query_fn remove=module?(query_fn)(uintptr_t)GetProcAddress(module,"Ntw16sWindowRemove"):NULL;if(!attached||!remove)return 3;
    WNDCLASSW root={0},bridge={0};root.lpfnWndProc=proc;root.hInstance=GetModuleHandleW(NULL);root.lpszClassName=L"Win16DeferFixture";
    bridge.lpfnWndProc=DefWindowProcW;bridge.hInstance=root.hInstance;
    bridge.lpszClassName=argc==2 && !strcmp(argv[1],"modern")?L"Microsoft.UI.Content.DesktopChildSiteBridge":L"Windows.UI.Composition.DesktopWindowContentBridge";
    if(!RegisterClassW(&root)||!RegisterClassW(&bridge))return 4;
    MONITORINFO monitor={.cbSize=sizeof(monitor)};POINT primary={0,0};
    if(!GetMonitorInfoW(MonitorFromPoint(primary,MONITOR_DEFAULTTOPRIMARY),&monitor))return 22;
    HWND w=CreateWindowW(root.lpszClassName,L"",WS_OVERLAPPEDWINDOW,monitor.rcWork.left+100,monitor.rcWork.top,400,300,NULL,NULL,root.hInstance,NULL);if(!w||!attached(w))return 5;
    HWND child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD,0,0,200,100,w,NULL,bridge.hInstance,NULL);if(!child)return 6;
    ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);pump(120);int waiting=top(w);RECT native_outer,styled_outer,restored_outer;
    if(!GetWindowRect(w,&native_outer))return 19;
    SendMessageW(w,WM_NCACTIVATE,TRUE,0);ShowWindow(child,SW_SHOWNOACTIVATE);pump(160);int ready=top(w);unsigned active=active_pixels(w);
    if(!GetWindowRect(w,&styled_outer))return 20;
    BOOL removed=remove(w);int restored=top(w);if(!GetWindowRect(w,&restored_outer))return 21;DestroyWindow(w);
    w=CreateWindowW(root.lpszClassName,L"",WS_OVERLAPPEDWINDOW,10,10,400,300,NULL,NULL,root.hInstance,NULL);if(!w||!attached(w))return 7;
    ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);pump(80);int classic=top(w);BOOL classic_removed=remove(w);DestroyWindow(w);
    w=CreateWindowW(root.lpszClassName,L"",WS_OVERLAPPEDWINDOW,10,10,400,300,NULL,NULL,root.hInstance,NULL);if(!w||!attached(w))return 9;
    child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD,0,0,200,100,w,NULL,bridge.hInstance,NULL);if(!child)return 10;
    ShowWindow(w,SW_SHOWNOACTIVATE);pump(80);ShowWindow(w,SW_HIDE);ShowWindow(child,SW_SHOWNOACTIVATE);pump(80);int hidden=top(w);
    ShowWindow(w,SW_SHOWNOACTIVATE);SendMessageW(w,WM_NCACTIVATE,TRUE,0);SendMessageW(w,WM_NCACTIVATE,FALSE,0);pump(120);int resumed=top(w);unsigned inactive=active_pixels(w);BOOL resumed_removed=remove(w);DestroyWindow(w);
    w=CreateWindowW(root.lpszClassName,L"",WS_OVERLAPPEDWINDOW,10,10,400,300,NULL,NULL,root.hInstance,NULL);if(!w||!attached(w))return 11;
    child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD,0,0,200,100,w,NULL,bridge.hInstance,NULL);if(!child)return 12;
    ShowWindow(w,SW_SHOWNOACTIVATE);pump(2200);int fallback=top(w);BOOL fallback_removed=remove(w);DestroyWindow(w);
    w=CreateWindowW(root.lpszClassName,L"",WS_OVERLAPPEDWINDOW,10,10,400,300,NULL,NULL,root.hInstance,NULL);if(!w||!attached(w))return 13;
    child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD,0,0,200,100,w,NULL,bridge.hInstance,NULL);if(!child)return 14;
    ShowWindow(w,SW_SHOWNOACTIVATE);Sleep(50);MSG queued;if(!PeekMessageW(&queued,NULL,WM_TIMER,WM_TIMER,PM_NOREMOVE))return 15;
    HWND timer=queued.hwnd;BOOL timer_before=IsWindow(timer);DestroyWindow(w);BOOL timer_after=IsWindow(timer);pump(50);
    w=CreateWindowW(root.lpszClassName,L"",WS_OVERLAPPEDWINDOW,10,10,400,300,NULL,NULL,root.hInstance,NULL);if(!w||!attached(w))return 16;
    child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD,0,0,200,100,w,NULL,bridge.hInstance,NULL);if(!child)return 17;
    destroy_during_timer=w;created_timer=NULL;HHOOK hook=SetWindowsHookExW(WH_CBT,destroy_on_timer_create,NULL,GetCurrentThreadId());if(!hook)return 18;
    ShowWindow(w,SW_SHOWNOACTIVATE);UnhookWindowsHookEx(hook);BOOL root_destroyed=!IsWindow(w),helper_destroyed=created_timer&&!IsWindow(created_timer);pump(50);
    printf("DEFER waiting=%d ready=%d active=%u restored=%d removed=%d classic=%d classic_removed=%d hidden=%d resumed=%d inactive=%u resumed_removed=%d fallback=%d fallback_removed=%d timer=%d/%d reentrant=%d/%d\n",waiting,ready,active,restored,removed,classic,classic_removed,hidden,resumed,inactive,resumed_removed,fallback,fallback_removed,timer_before,timer_after,root_destroyed,helper_destroyed);
    UnregisterClassW(bridge.lpszClassName,bridge.hInstance);UnregisterClassW(root.lpszClassName,root.hInstance);CloseDesktop(desk);
    return waiting==0&&ready>0&&active>100&&EqualRect(&native_outer,&styled_outer)&&EqualRect(&native_outer,&restored_outer)&&restored==0&&removed&&classic>0&&classic_removed&&hidden==0&&resumed>0&&!inactive&&resumed_removed&&fallback>0&&fallback_removed&&timer_before&&!timer_after&&root_destroyed&&helper_destroyed?0:8;
}
