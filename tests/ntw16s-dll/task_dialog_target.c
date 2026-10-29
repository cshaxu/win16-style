#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
static BOOL (__cdecl *attached)(HWND);static int created,destroyed;static BOOL frame;
static void CALLBACK choose(HWND window,UINT message,UINT_PTR timer,DWORD ticks)
{(void)message;(void)ticks;KillTimer(window,timer);frame=attached&&attached(window);SendMessageW(window,TDM_CLICK_BUTTON,IDOK,0);}
static HRESULT CALLBACK dialog_callback(HWND window,UINT notification,WPARAM wparam,LPARAM lparam,LONG_PTR data)
{(void)wparam;(void)lparam;(void)data;if(notification==TDN_CREATED){++created;SetTimer(window,917,100,choose);}if(notification==TDN_DESTROYED)++destroyed;return S_OK;}
int wmain(void)
{
    HMODULE m=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    attached=m?(BOOL (__cdecl *)(HWND))(void *)GetProcAddress(m,"Ntw16sWindowAttached"):NULL;
    TASKDIALOGCONFIG config={0};config.cbSize=sizeof(config);config.dwCommonButtons=TDCBF_OK_BUTTON;config.pszWindowTitle=L"Task dialog probe";config.pszContent=L"Owned test";config.pfCallback=dialog_callback;
    int button=0;HRESULT hr=TaskDialogIndirect(&config,&button,NULL,NULL);
    printf("TASK_DIALOG bits=%u hr=%08lx button=%d created=%d destroyed=%d frame=%d\n",(unsigned)(sizeof(void*)*8),(DWORD)hr,button,created,destroyed,frame);
    return hr==S_OK && button==IDOK && created==1 && destroyed==1 && frame?0:1;
}
