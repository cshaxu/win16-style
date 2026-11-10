#ifndef TEST_GLOBAL_TARGET_FIXTURE_H
#define TEST_GLOBAL_TARGET_FIXTURE_H
static HWND button;static unsigned clicks;
static LRESULT CALLBACK window_proc(HWND,UINT,WPARAM,LPARAM);
static LRESULT global_fixture_message(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+52){
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        return attached?(attached(w)?1:0)|(attached(button)?2:0):0;
    }
    if(m==WM_APP+53)return clicks;
    if(m==WM_COMMAND && HIWORD(a)==BN_CLICKED){++clicks;return 0;}
    if(m==WM_CLOSE){DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static int target(DWORD exstyle)
{
    WCHAR name[128];if(!GetEnvironmentVariableW(L"WIN16_ATTACH_TEST_READY",name,ARRAYSIZE(name)))return 2;
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);if(!ready)return 3;
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=window_proc,.lpszClassName=L"Win16OwnedAttachTest"};
    if(!RegisterClassW(&c))return 4;
    HWND window=CreateWindowExW(exstyle,c.lpszClassName,L"owned",WS_OVERLAPPEDWINDOW,0,0,180,120,NULL,NULL,c.hInstance,NULL);if(!window)return 5;
    button=CreateWindowW(L"BUTTON",L"Click",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,10,10,80,30,window,(HMENU)(UINT_PTR)101,c.hInstance,NULL);
    if(!button)return 6;
    ShowWindow(window,SW_SHOW);SetEvent(ready);CloseHandle(ready);
    MSG message;while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}return 0;
}
static int query(HWND window,UINT message)
{DWORD_PTR result=0;return SendMessageTimeoutW(window,message,0,0,SMTO_ABORTIFHUNG,1000,&result)?(int)result:-1;}
#endif
