#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <stdio.h>
#include <wchar.h>

static BOOL wide,nested_mode;
static HANDLE ready_event,release_event;
static DWORD thread_first,thread_second,thread_id,gui_thread;
static volatile LONG seen;
static volatile LONG nested_seen,nested_error;
static HWND dialog;
static const WCHAR title[]=L"Win16S extended error probe";

static BOOL bad_structure(void)
{
    if(wide){OPENFILENAMEW o={0};o.lStructSize=1;return GetOpenFileNameW(&o);}
    OPENFILENAMEA o={0};o.lStructSize=1;return GetOpenFileNameA(&o);
}
static BOOL bad_hook(void)
{
    if(wide){WCHAR file[32]=L"";OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);o.Flags=OFN_ENABLEHOOK;return GetOpenFileNameW(&o);}
    char file[32]={0};OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);o.Flags=OFN_ENABLEHOOK;return GetOpenFileNameA(&o);
}
static BOOL bad_color(void)
{
    if(wide){CHOOSECOLORW c={0};c.lStructSize=1;return ChooseColorW(&c);}
    CHOOSECOLORA c={0};c.lStructSize=1;return ChooseColorA(&c);
}
static UINT_PTR CALLBACK nested_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)w;(void)wp;(void)lp;
    if(message==WM_INITDIALOG){BOOL result=bad_color();DWORD error=CommDlgExtendedError();
        if(!result){InterlockedExchange(&nested_error,(LONG)error);InterlockedIncrement(&nested_seen);}}
    return 0;
}
static DWORD WINAPI other_thread(void *unused)
{
    (void)unused;thread_id=GetCurrentThreadId();BOOL result=bad_hook();thread_first=CommDlgExtendedError();
    printf("ERROR_THREAD tid=%lu hook_result=%d first=%lu\n",thread_id,result,thread_first);
    SetEvent(ready_event);if(WaitForSingleObject(release_event,5000)!=WAIT_OBJECT_0)return 1;
    thread_second=CommDlgExtendedError();return 0;
}
static BOOL CALLBACK find_dialog(HWND w,LPARAM unused)
{
    (void)unused;WCHAR text[128]=L"",cls[32]=L"";DWORD_PTR copied=0;
    if(!IsWindowVisible(w)||!GetClassNameW(w,cls,ARRAYSIZE(cls))||lstrcmpW(cls,L"#32770"))return TRUE;
    if(!SendMessageTimeoutW(w,WM_GETTEXT,ARRAYSIZE(text),(LPARAM)text,SMTO_ABORTIFHUNG,300,&copied)||lstrcmpW(text,title))return TRUE;
    dialog=w;InterlockedExchange(&seen,1);return FALSE;
}
static DWORD WINAPI observer(void *unused)
{
    (void)unused;for(unsigned i=0;i<100;++i){if(!seen)EnumThreadWindows(gui_thread,find_dialog,0);
        if(seen && IsWindow(dialog) && i>5)PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);
        if(seen&&!IsWindow(dialog))return 0;
        Sleep(50);}
    if(dialog&&IsWindow(dialog))PostMessageW(dialog,WM_CLOSE,0,0);
    return 1;
}
static BOOL cancel_dialog(void)
{
    if(wide){WCHAR file[256]=L"";OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);o.lpstrTitle=title;o.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|(nested_mode?OFN_ENABLEHOOK:0);if(nested_mode)o.lpfnHook=nested_hook;return GetOpenFileNameW(&o);}
    char file[256]={0};OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);o.lpstrTitle="Win16S extended error probe";o.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|(nested_mode?OFN_ENABLEHOOK:0);if(nested_mode)o.lpfnHook=nested_hook;return GetOpenFileNameA(&o);
}
int wmain(int argc,WCHAR **argv)
{
    setvbuf(stdout,NULL,_IONBF,0);if(argc!=2&&argc!=3)return 2;wide=!wcscmp(argv[1],L"wide");if(!wide&&wcscmp(argv[1],L"ansi"))return 2;nested_mode=argc==3&&!wcscmp(argv[2],L"nested");if(argc==3&&!nested_mode)return 2;
    DWORD main_id=GetCurrentThreadId();BOOL open_bad=bad_structure();DWORD first=CommDlgExtendedError(),repeat=CommDlgExtendedError();
    SetLastError(0x12345678);DWORD third=CommDlgExtendedError(),last_after=GetLastError();
    BOOL hook_bad=bad_hook();DWORD hook_error=CommDlgExtendedError();BOOL color_bad=bad_color();DWORD color_error=CommDlgExtendedError();
    ready_event=CreateEventW(NULL,TRUE,FALSE,NULL);release_event=CreateEventW(NULL,TRUE,FALSE,NULL);if(!ready_event||!release_event)return 3;
    BOOL reset=bad_structure();DWORD main_before=CommDlgExtendedError();HANDLE thread=CreateThread(NULL,0,other_thread,NULL,0,NULL);if(!thread)return 4;
    DWORD ready=WaitForSingleObject(ready_event,5000);DWORD main_during=CommDlgExtendedError();SetEvent(release_event);
    DWORD wait=WaitForSingleObject(thread,5000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(thread,&code);CloseHandle(thread);CloseHandle(ready_event);CloseHandle(release_event);
    gui_thread=GetCurrentThreadId();HANDLE watcher=CreateThread(NULL,0,observer,NULL,0,NULL);if(!watcher)return 5;
    BOOL cancel=cancel_dialog();DWORD cancel_error=CommDlgExtendedError(),cancel_repeat=CommDlgExtendedError();DWORD watched=WaitForSingleObject(watcher,7000),watch_code=99;if(watched==WAIT_OBJECT_0)GetExitCodeThread(watcher,&watch_code);CloseHandle(watcher);
    printf("ERROR_CONTRACT bits=%u wide=%d main_tid=%lu worker_tid=%lu open=%d first=%lu repeat=%lu third=%lu last=%08lx hook=%d hook_error=%lu color=%d color_error=%lu reset=%d before=%lu during=%lu worker_first=%lu worker_second=%lu ready=%lu wait=%lu code=%lu\n",(unsigned)(sizeof(void*)*8),wide,main_id,thread_id,open_bad,first,repeat,third,last_after,hook_bad,hook_error,color_bad,color_error,reset,main_before,main_during,thread_first,thread_second,ready,wait,code);
    printf("ERROR_CANCEL seen=%ld result=%d error=%lu repeat=%lu wait=%lu worker=%lu nested_seen=%ld nested_error=%ld\n",seen,cancel,cancel_error,cancel_repeat,watched,watch_code,nested_seen,nested_error);
    BOOL cancel_ok=nested_mode?((cancel_error==0||cancel_error==CDERR_STRUCTSIZE)&&cancel_repeat==cancel_error&&nested_seen==1&&nested_error==CDERR_STRUCTSIZE):(cancel_error==0&&cancel_repeat==0&&!nested_seen&&!nested_error);
    return !open_bad&&!hook_bad&&!color_bad&&!reset&&!cancel&&first==CDERR_STRUCTSIZE&&repeat==first&&third==first&&hook_error==CDERR_NOHOOK&&color_error==CDERR_STRUCTSIZE&&main_before==CDERR_STRUCTSIZE&&main_during==main_before&&thread_first==CDERR_NOHOOK&&thread_second==thread_first&&cancel_ok&&seen&&ready==WAIT_OBJECT_0&&wait==WAIT_OBJECT_0&&!code&&watched==WAIT_OBJECT_0&&!watch_code?0:1;
}
