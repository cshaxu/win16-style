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
#define DECLARE_BOOL_HOOK(name,type) static BOOL (WINAPI *native_##name)(type *);static BOOL WINAPI hook_##name(type *p){clear_shadow();return native_##name(p);}
#define DECLARE_HWND_HOOK(name,type) static HWND (WINAPI *native_##name)(type *);static HWND WINAPI hook_##name(type *p){clear_shadow();return native_##name(p);}
static _Thread_local BOOL shadow_active;
static _Thread_local DWORD shadow_value;
static _Thread_local BOOL prototype_confirmed;
static volatile LONG naive_active;
static BOOL naive,wide,nested,gap_mode,extended,nested_open_mode;
static HANDLE ready_event,release_event;
static DWORD worker_first,worker_second,owner;
static volatile LONG seen,nested_error,nested_seen;
static HWND dialog;
static const WCHAR normal_title[]=L"Win16S shadow ordinary";
static const WCHAR nested_title[]=L"Win16S shadow nested";
static void clear_shadow(void){shadow_active=FALSE;InterlockedExchange(&naive_active,0);}
DECLARE_BOOL_HOOK(SaveA,OPENFILENAMEA)
DECLARE_BOOL_HOOK(SaveW,OPENFILENAMEW)
DECLARE_BOOL_HOOK(FontA,CHOOSEFONTA)
DECLARE_BOOL_HOOK(FontW,CHOOSEFONTW)
DECLARE_BOOL_HOOK(PrintA,PRINTDLGA)
DECLARE_BOOL_HOOK(PrintW,PRINTDLGW)
DECLARE_BOOL_HOOK(PageA,PAGESETUPDLGA)
DECLARE_BOOL_HOOK(PageW,PAGESETUPDLGW)
DECLARE_HWND_HOOK(FindA,FINDREPLACEA)
DECLARE_HWND_HOOK(FindW,FINDREPLACEW)
DECLARE_HWND_HOOK(ReplaceA,FINDREPLACEA)
DECLARE_HWND_HOOK(ReplaceW,FINDREPLACEW)
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
    printf("SHADOW_OPEN_ENTER ansi\n");clear_shadow();BOOL previous=prototype_confirmed;prototype_confirmed=FALSE;BOOL result=native_open_a(p);DWORD last=GetLastError();BOOL confirmed=prototype_confirmed;if(extended)prototype_confirmed=previous;
    if(!result&&confirmed){DWORD error=native_error();if((error==0||error==CDERR_DIALOGFAILURE||(extended&&nested_open_mode&&error==CDERR_STRUCTSIZE))&&p&&p->lpstrFile&&p->nMaxFile>=2){WORD required=80;memcpy(p->lpstrFile,&required,sizeof(required));shadow_value=FNERR_BUFFERTOOSMALL;shadow_active=TRUE;InterlockedExchange(&naive_active,1);}}
    printf("SHADOW_OPEN_EXIT ansi confirmed=%d active=%d result=%d\n",confirmed,shadow_active,result);SetLastError(last);return result;
}
static BOOL WINAPI hook_open_w(LPOPENFILENAMEW p)
{
    printf("SHADOW_OPEN_ENTER wide\n");clear_shadow();BOOL previous=prototype_confirmed;prototype_confirmed=FALSE;BOOL result=native_open_w(p);DWORD last=GetLastError();BOOL confirmed=prototype_confirmed;if(extended)prototype_confirmed=previous;
    if(!result&&confirmed){DWORD error=native_error();if((error==0||error==CDERR_DIALOGFAILURE||(extended&&nested_open_mode&&error==CDERR_STRUCTSIZE))&&p&&p->lpstrFile&&p->nMaxFile>=1){WORD required=80;memcpy(p->lpstrFile,&required,sizeof(required));shadow_value=FNERR_BUFFERTOOSMALL;shadow_active=TRUE;InterlockedExchange(&naive_active,1);}}
    printf("SHADOW_OPEN_EXIT wide confirmed=%d active=%d result=%d\n",confirmed,shadow_active,result);SetLastError(last);return result;
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
static BOOL bad_common(unsigned index)
{
    if(index==0)return bad_font();
    if(index==1){if(wide){OPENFILENAMEW p={0};p.lStructSize=1;return GetSaveFileNameW(&p);}OPENFILENAMEA p={0};p.lStructSize=1;return GetSaveFileNameA(&p);}
    if(index==2){if(wide){PRINTDLGW p={0};p.lStructSize=1;return PrintDlgW(&p);}PRINTDLGA p={0};p.lStructSize=1;return PrintDlgA(&p);}
    if(index==3){if(wide){PAGESETUPDLGW p={0};p.lStructSize=1;return PageSetupDlgW(&p);}PAGESETUPDLGA p={0};p.lStructSize=1;return PageSetupDlgA(&p);}
    if(index==4){if(wide){FINDREPLACEW p={0};p.lStructSize=1;return FindTextW(&p)!=NULL;}FINDREPLACEA p={0};p.lStructSize=1;return FindTextA(&p)!=NULL;}
    if(index==5){if(wide){FINDREPLACEW p={0};p.lStructSize=1;return ReplaceTextW(&p)!=NULL;}FINDREPLACEA p={0};p.lStructSize=1;return ReplaceTextA(&p)!=NULL;}
    if(index==6)return bad_color();
    if(wide){OPENFILENAMEW p={0};p.lStructSize=1;return GetOpenFileNameW(&p);}
    OPENFILENAMEA p={0};p.lStructSize=1;return GetOpenFileNameA(&p);
}
static UINT_PTR CALLBACK abort_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)wp;(void)lp;if(message==WM_INITDIALOG){printf("SHADOW_ABORT_INIT\n");prototype_confirmed=TRUE;
        if(nested_open_mode){if(wide){OPENFILENAMEW inner={0};inner.lStructSize=1;(void)GetOpenFileNameW(&inner);}else{OPENFILENAMEA inner={0};inner.lStructSize=1;(void)GetOpenFileNameA(&inner);}}
        if(!PostMessageW(w,WM_COMMAND,IDABORT,0))prototype_confirmed=FALSE;}return 0;
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
    setvbuf(stdout,NULL,_IONBF,0);if(argc!=2&&argc!=3)return 2;wide=!wcscmp(argv[1],L"wide");if(!wide&&wcscmp(argv[1],L"ansi"))return 2;naive=argc==3&&!wcscmp(argv[2],L"naive");gap_mode=argc==3&&!wcscmp(argv[2],L"gap");
    BOOL matrix=argc==3&&!wcscmp(argv[2],L"matrix"),nested_legacy=argc==3&&!wcscmp(argv[2],L"nestedlegacy"),nested_fixed=argc==3&&!wcscmp(argv[2],L"nestedfixed");
    extended=matrix||nested_fixed;nested_open_mode=nested_legacy||nested_fixed;if(argc==3&&!naive&&!gap_mode&&!matrix&&!nested_open_mode)return 2;
    HMODULE module=GetModuleHandleW(L"comdlg32.dll");if(!module)return 3;
    const char *names[]={"CommDlgExtendedError","GetOpenFileNameA","GetOpenFileNameW","ChooseColorA","ChooseColorW"};
    void *slots[]={&native_error,&native_open_a,&native_open_w,&native_color_a,&native_color_w};
    for(unsigned i=0;i<ARRAYSIZE(names);++i){FARPROC address=GetProcAddress(module,names[i]);if(!address)return 3;memcpy(slots[i],&address,sizeof(address));}
    const char *extra_names[]={"GetSaveFileNameA","GetSaveFileNameW","ChooseFontA","ChooseFontW","PrintDlgA","PrintDlgW","PageSetupDlgA","PageSetupDlgW","FindTextA","FindTextW","ReplaceTextA","ReplaceTextW"};
    void *extra_slots[]={&native_SaveA,&native_SaveW,&native_FontA,&native_FontW,&native_PrintA,&native_PrintW,&native_PageA,&native_PageW,&native_FindA,&native_FindW,&native_ReplaceA,&native_ReplaceW};
    void *extra_hooks[]={(void *)hook_SaveA,(void *)hook_SaveW,(void *)hook_FontA,(void *)hook_FontW,(void *)hook_PrintA,(void *)hook_PrintW,(void *)hook_PageA,(void *)hook_PageW,(void *)hook_FindA,(void *)hook_FindW,(void *)hook_ReplaceA,(void *)hook_ReplaceW};
    if(extended)for(unsigned i=0;i<ARRAYSIZE(extra_names);++i){FARPROC address=GetProcAddress(module,extra_names[i]);if(!address)return 3;memcpy(extra_slots[i],&address,sizeof(address));}
    DWORD status=ntw16s_hooks_begin();if(!status)status=ntw16s_hooks_attach(&native_error,(void *)hook_error);if(!status)status=ntw16s_hooks_attach(&native_open_a,(void *)hook_open_a);if(!status)status=ntw16s_hooks_attach(&native_open_w,(void *)hook_open_w);if(!status)status=ntw16s_hooks_attach(&native_color_a,(void *)hook_color_a);if(!status)status=ntw16s_hooks_attach(&native_color_w,(void *)hook_color_w);
    if(extended)for(unsigned i=0;!status && i<ARRAYSIZE(extra_names);++i)status=ntw16s_hooks_attach(extra_slots[i],extra_hooks[i]);
    if(!status)status=ntw16s_hooks_commit();
    if(status){ntw16s_hooks_abort();printf("SHADOW_HOOK_FAIL %lu\n",status);return 3;}
    BOOL special=open_dialog(TRUE);DWORD first=CommDlgExtendedError(),repeat=CommDlgExtendedError();
    if(nested_open_mode){printf("SHADOW_NESTED fixed=%d first=%lu repeat=%lu\n",extended,first,repeat);return !special&&first==repeat&&(extended?first==FNERR_BUFFERTOOSMALL:first!=FNERR_BUFFERTOOSMALL)?0:1;}
    if(matrix){unsigned passed=0;for(unsigned i=0;i<8;++i){if(i){special=open_dialog(TRUE);first=CommDlgExtendedError();}
            BOOL failed=bad_common(i);DWORD reported=CommDlgExtendedError(),underlying=native_error();
            BOOL ok=!special&&first==FNERR_BUFFERTOOSMALL&&!failed&&reported==underlying&&reported!=FNERR_BUFFERTOOSMALL;
            printf("SHADOW_MATRIX index=%u result=%d reported=%lu native=%lu valid=%d\n",i,failed,reported,underlying,ok);if(ok)++passed;}
        special=open_dialog(TRUE);first=CommDlgExtendedError();DWORD before_ex=native_error();HRESULT ex;
        if(wide){PRINTDLGEXW p={0};p.lStructSize=1;ex=PrintDlgExW(&p);}else{PRINTDLGEXA p={0};p.lStructSize=1;ex=PrintDlgExA(&p);}
        DWORD after_ex=native_error(),reported_ex=CommDlgExtendedError();
        printf("SHADOW_MATRIX_EX hr=%08lx before=%lu after=%lu reported=%lu\n",(unsigned long)ex,before_ex,after_ex,reported_ex);
        printf("SHADOW_MATRIX_RESULT bits=%u wide=%d passed=%u total=8\n",(unsigned)(sizeof(void*)*8),wide,passed);return !special&&first==FNERR_BUFFERTOOSMALL&&FAILED(ex)&&passed==8?0:1;}
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
