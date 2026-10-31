#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
typedef BOOL (__cdecl *attachedfn)(HWND);
static HWND root;static int stage,dialog_ok,button_ok;static ULONGLONG deadline;
static attachedfn fn(void){HMODULE m=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");return m?(attachedfn)(void*)GetProcAddress(m,"Ntw16sWindowAttached"):NULL;}
static LRESULT CALLBACK proc(HWND w,UINT msg,WPARAM a,LPARAM b){
 if(msg==WM_TIMER){attachedfn f=fn();if(stage==0&&f&&f(w)){stage=1;deadline=GetTickCount64()+2500;MessageBoxW(w,L"Owned test dialog",L"Global dialog probe",MB_OK);printf("GLOBAL LATE DIALOG width=%u dialog=%d button=%d\n",(unsigned)(sizeof(void*)*8),dialog_ok,button_ok);DestroyWindow(w);return 0;}
 if(stage==1){HWND d=FindWindowW(L"#32770",L"Global dialog probe");HWND button=d?FindWindowExW(d,NULL,L"Button",NULL):NULL;if(d&&f){dialog_ok=f(d);button_ok=button&&f(button);if(dialog_ok&&button_ok)PostMessageW(d,WM_CLOSE,0,0);}if(GetTickCount64()>deadline&&d)PostMessageW(d,WM_CLOSE,0,0);}
 if(GetTickCount64()>deadline+5000){DestroyWindow(w);return 0;}return 0;}
 if(msg==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(w,msg,a,b);}
int wmain(void){WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=proc,.lpszClassName=L"GlobalDialogOwnProbe"};if(!RegisterClassW(&c))return 2;root=CreateWindowW(c.lpszClassName,L"probe root",WS_OVERLAPPEDWINDOW|WS_VISIBLE,50,50,360,240,NULL,NULL,c.hInstance,NULL);if(!root)return 3;deadline=GetTickCount64()+5000;SetTimer(root,1,50,NULL);MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return dialog_ok&&button_ok?0:1;}
