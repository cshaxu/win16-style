#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include "ntw16s-dll/injection/startup_hooks.h"

static DWORD (WINAPI *native_error)(void);
static BOOL (WINAPI *native_open_a)(LPOPENFILENAMEA);
static BOOL (WINAPI *native_open_w)(LPOPENFILENAMEW);
static BOOL (WINAPI *native_color_a)(LPCHOOSECOLORA);
static BOOL (WINAPI *native_color_w)(LPCHOOSECOLORW);
static _Thread_local BOOL shadow_active;
static _Thread_local DWORD shadow_value;
static _Thread_local BOOL prototype_confirmed;
static volatile LONG naive_active;
static BOOL naive,wide,nested,gap_mode;
static HANDLE ready_event,release_event;
static DWORD worker_first,worker_second,owner;
static volatile LONG seen,nested_error,nested_seen;
static HWND dialog;
static const WCHAR normal_title[]=L"Win16S shadow ordinary";
static const WCHAR nested_title[]=L"Win16S shadow nested";
static void clear_shadow(void){shadow_active=FALSE;InterlockedExchange(&naive_active,0);}
static DWORD WINAPI hook_error(void)
{
    DWORD result=native_error();DWORD last=GetLastError();
    if(naive?InterlockedCompareExchange(&naive_active,0,0):shadow_active)result=naive?FNERR_BUFFERTOOSMALL:shadow_value;
    SetLastError(last);return result;
}
static BOOL WINAPI hook_color_a(LPCHOOSECOLORA p){clear_shadow();return native_color_a(p);}
static BOOL WINAPI hook_color_w(LPCHOOSECOLORW p){clear_shadow();return native_color_w(p);}
static BOOL WINAPI hook_open_a(LPOPENFILENAMEA p)
{
    printf("SHADOW_OPEN_ENTER ansi\n");clear_shadow();prototype_confirmed=FALSE;BOOL result=native_open_a(p);DWORD last=GetLastError();
    if(!result&&prototype_confirmed){DWORD error=native_error();if((error==0||error==CDERR_DIALOGFAILURE)&&p&&p->lpstrFile&&p->nMaxFile>=2){WORD required=80;memcpy(p->lpstrFile,&required,sizeof(required));shadow_value=FNERR_BUFFERTOOSMALL;shadow_active=TRUE;InterlockedExchange(&naive_active,1);}}
    printf("SHADOW_OPEN_EXIT ansi confirmed=%d active=%d result=%d\n",prototype_confirmed,shadow_active,result);SetLastError(last);return result;
}
static BOOL WINAPI hook_open_w(LPOPENFILENAMEW p)
{
    printf("SHADOW_OPEN_ENTER wide\n");clear_shadow();prototype_confirmed=FALSE;BOOL result=native_open_w(p);DWORD last=GetLastError();
    if(!result&&prototype_confirmed){DWORD error=native_error();if((error==0||error==CDERR_DIALOGFAILURE)&&p&&p->lpstrFile&&p->nMaxFile>=1){WORD required=80;memcpy(p->lpstrFile,&required,sizeof(required));shadow_value=FNERR_BUFFERTOOSMALL;shadow_active=TRUE;InterlockedExchange(&naive_active,1);}}
    printf("SHADOW_OPEN_EXIT wide confirmed=%d active=%d result=%d\n",prototype_confirmed,shadow_active,result);SetLastError(last);return result;
}
static BOOL bad_color(void)
{
    if(wide){CHOOSECOLORW p={0};p.lStructSize=1;return ChooseColorW(&p);}
    CHOOSECOLORA p={0};p.lStructSize=1;return ChooseColorA(&p);
}
static BOOL bad_font(void)
{
    if(wide){CHOOSEFONTW p={0};p.lStructSize=1;return ChooseFontW(&p);}
    CHOOSEFONTA p={0};p.lStructSize=1;return ChooseFontA(&p);
}
static UINT_PTR CALLBACK abort_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)wp;(void)lp;if(message==WM_INITDIALOG){printf("SHADOW_ABORT_INIT\n");prototype_confirmed=TRUE;PostMessageW(w,WM_COMMAND,IDABORT,0);}return 0;
}
static UINT_PTR CALLBACK nested_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)w;(void)wp;(void)lp;if(message==WM_INITDIALOG){BOOL r=bad_color();DWORD e=CommDlgExtendedError();if(!r){InterlockedExchange(&nested_error,(LONG)e);InterlockedIncrement(&nested_seen);}}return 0;
}
static BOOL CALLBACK find_dialog(HWND w,LPARAM unused)
{
    (void)unused;WCHAR text[128]=L"",cls[32]=L"";DWORD_PTR copied=0;
    if(!IsWindowVisible(w)||!GetClassNameW(w,cls,ARRAYSIZE(cls))||lstrcmpW(cls,L"#32770"))return TRUE;
    if(!SendMessageTimeoutW(w,WM_GETTEXT,ARRAYSIZE(text),(LPARAM)text,SMTO_ABORTIFHUNG,300,&copied)||lstrcmpW(text,nested?nested_title:normal_title))return TRUE;
    dialog=w;InterlockedExchange(&seen,1);return FALSE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;for(unsigned i=0;i<120;++i){if(!seen)EnumThreadWindows(owner,find_dialog,0);if(seen&&IsWindow(dialog)&&i>5)PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);if(seen&&!IsWindow(dialog))return 0;Sleep(50);}
    if(dialog&&IsWindow(dialog))PostMessageW(dialog,WM_CLOSE,0,0);
    return 1;
}
static BOOL open_dialog(BOOL special)
{
    if(wide){WCHAR file[8]=L"";OPENFILENAMEW p={0};p.lStructSize=sizeof(p);p.lpstrFile=file;p.nMaxFile=ARRAYSIZE(file);p.lpstrTitle=special?L"Win16S shadow special":nested?nested_title:normal_title;
        p.Flags=OFN_NOCHANGEDIR|(special?OFN_ENABLEHOOK:OFN_EXPLORER|(nested?OFN_ENABLEHOOK:0));p.lpfnHook=special?abort_hook:nested?nested_hook:NULL;
        BOOL r=GetOpenFileNameW(&p);WORD required=0;memcpy(&required,file,sizeof(required));if(special)printf("SHADOW_BUFFER required=%u\n",required);return r;}
    char file[8]={0};OPENFILENAMEA p={0};p.lStructSize=sizeof(p);p.lpstrFile=file;p.nMaxFile=ARRAYSIZE(file);p.lpstrTitle=special?"Win16S shadow special":nested?"Win16S shadow nested":"Win16S shadow ordinary";
    p.Flags=OFN_NOCHANGEDIR|(special?OFN_ENABLEHOOK:OFN_EXPLORER|(nested?OFN_ENABLEHOOK:0));p.lpfnHook=special?abort_hook:nested?nested_hook:NULL;
    BOOL r=GetOpenFileNameA(&p);WORD required=0;memcpy(&required,file,sizeof(required));if(special)printf("SHADOW_BUFFER required=%u\n",required);return r;
}
static DWORD WINAPI worker(void *unused)
{
    (void)unused;BOOL bad=bad_color();worker_first=CommDlgExtendedError();SetEvent(ready_event);
    if(WaitForSingleObject(release_event,5000)!=WAIT_OBJECT_0)return 1;
    worker_second=CommDlgExtendedError();return !bad&&worker_first==CDERR_STRUCTSIZE&&worker_second==worker_first?0:1;
}
int wmain(int argc,WCHAR **argv)
{
    setvbuf(stdout,NULL,_IONBF,0);if(argc!=2&&argc!=3)return 2;wide=!wcscmp(argv[1],L"wide");if(!wide&&wcscmp(argv[1],L"ansi"))return 2;naive=argc==3&&!wcscmp(argv[2],L"naive");gap_mode=argc==3&&!wcscmp(argv[2],L"gap");if(argc==3&&!naive&&!gap_mode)return 2;
    HMODULE module=GetModuleHandleW(L"comdlg32.dll");if(!module)return 3;
    const char *names[]={"CommDlgExtendedError","GetOpenFileNameA","GetOpenFileNameW","ChooseColorA","ChooseColorW"};
    void *slots[]={&native_error,&native_open_a,&native_open_w,&native_color_a,&native_color_w};
    for(unsigned i=0;i<ARRAYSIZE(names);++i){FARPROC address=GetProcAddress(module,names[i]);if(!address)return 3;memcpy(slots[i],&address,sizeof(address));}
    DWORD status=ntw16s_hooks_begin();if(!status)status=ntw16s_hooks_attach(&native_error,(void *)hook_error);if(!status)status=ntw16s_hooks_attach(&native_open_a,(void *)hook_open_a);if(!status)status=ntw16s_hooks_attach(&native_open_w,(void *)hook_open_w);if(!status)status=ntw16s_hooks_attach(&native_color_a,(void *)hook_color_a);if(!status)status=ntw16s_hooks_attach(&native_color_w,(void *)hook_color_w);if(!status)status=ntw16s_hooks_commit();if(status){ntw16s_hooks_abort();printf("SHADOW_HOOK_FAIL %lu\n",status);return 3;}
    BOOL special=open_dialog(TRUE);DWORD first=CommDlgExtendedError(),repeat=CommDlgExtendedError();
    if(gap_mode){BOOL font=bad_font();DWORD reported=CommDlgExtendedError(),underlying=native_error();
        printf("SHADOW_GAP bits=%u wide=%d font=%d reported=%lu native=%lu\n",(unsigned)(sizeof(void*)*8),wide,font,reported,underlying);
        return !special&&!font&&first==FNERR_BUFFERTOOSMALL&&repeat==first&&reported==FNERR_BUFFERTOOSMALL&&underlying==CDERR_STRUCTSIZE?0:1;}
    ready_event=CreateEventW(NULL,TRUE,FALSE,NULL);release_event=CreateEventW(NULL,TRUE,FALSE,NULL);if(!ready_event||!release_event)return 4;
    HANDLE thread=CreateThread(NULL,0,worker,NULL,0,NULL);if(!thread)return 5;DWORD ready=WaitForSingleObject(ready_event,5000),during=CommDlgExtendedError();SetEvent(release_event);DWORD waited=WaitForSingleObject(thread,5000),code=99;if(waited==WAIT_OBJECT_0)GetExitCodeThread(thread,&code);CloseHandle(thread);CloseHandle(ready_event);CloseHandle(release_event);
    BOOL bad=bad_color();DWORD changed=CommDlgExtendedError();
    owner=GetCurrentThreadId();seen=0;nested=FALSE;HANDLE watcher=CreateThread(NULL,0,observe,NULL,0,NULL);if(!watcher)return 6;BOOL cancel=open_dialog(FALSE);DWORD cancel_error=CommDlgExtendedError();DWORD cw=WaitForSingleObject(watcher,7000),cc=99;if(cw==WAIT_OBJECT_0)GetExitCodeThread(watcher,&cc);CloseHandle(watcher);
    seen=0;nested=TRUE;watcher=CreateThread(NULL,0,observe,NULL,0,NULL);if(!watcher)return 6;BOOL nested_cancel=open_dialog(FALSE);DWORD nested_after=CommDlgExtendedError();DWORD nw=WaitForSingleObject(watcher,7000),nc=99;if(nw==WAIT_OBJECT_0)GetExitCodeThread(watcher,&nc);CloseHandle(watcher);
    printf("SHADOW_CONTRACT bits=%u wide=%d naive=%d special=%d first=%lu repeat=%lu during=%lu worker=%lu,%lu ready=%lu wait=%lu code=%lu changed=%lu bad=%d cancel=%d cancel_error=%lu cancel_wait=%lu,%lu nested=%d nested_error=%ld nested_after=%lu nested_seen=%ld nested_wait=%lu,%lu\n",(unsigned)(sizeof(void*)*8),wide,naive,special,first,repeat,during,worker_first,worker_second,ready,waited,code,changed,bad,cancel,cancel_error,cw,cc,nested_cancel,nested_error,nested_after,nested_seen,nw,nc);
    if(naive){BOOL rejected=!special&&first==FNERR_BUFFERTOOSMALL&&during!=first&&worker_first==CDERR_STRUCTSIZE&&worker_second==worker_first;printf("SHADOW_NAIVE_REJECTED %d\n",rejected);return rejected?0:1;}
    return !special&&first==FNERR_BUFFERTOOSMALL&&repeat==first&&during==first&&worker_first==CDERR_STRUCTSIZE&&worker_second==worker_first&&ready==WAIT_OBJECT_0&&waited==WAIT_OBJECT_0&&!code&&!bad&&changed==CDERR_STRUCTSIZE&&!cancel&&!cancel_error&&cw==WAIT_OBJECT_0&&!cc&&!nested_cancel&&nested_seen==1&&nested_error==CDERR_STRUCTSIZE&&(nested_after==0||nested_after==CDERR_STRUCTSIZE)&&nw==WAIT_OBJECT_0&&!nc?0:1;
}
