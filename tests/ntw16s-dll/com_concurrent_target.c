#define COBJMACROS
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
typedef struct {WCHAR dir[MAX_PATH],a[MAX_PATH],b[MAX_PATH],title[64];LONG selected,closed;HRESULT hr;DWORD count,options;BOOL paths;} task;
static _Thread_local task *local;static volatile LONG ready;static BOOL shared,modern,serial_close;static task *base_task;
static void CALLBACK tick(HWND w,UINT msg,UINT_PTR id,DWORD time){(void)msg;(void)id;(void)time;task *t=local;if(!t)return;HWND list=GetDlgItem(w,1120);if(modern){if(!t->selected){t->selected=1;InterlockedIncrement(&ready);}if(InterlockedCompareExchange(&ready,0,0)==2 && (!serial_close || t==base_task || InterlockedCompareExchange(&base_task->closed,0,0)))PostMessageW(w,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(w,IDOK));return;}if(!list)return;
    if(!t->selected){LRESULT n=SendMessageW(list,LB_GETCOUNT,0,0);int hits=0;for(LRESULT i=0;i<n;++i){WCHAR name[MAX_PATH]=L"";LRESULT len=SendMessageW(list,LB_GETTEXTLEN,i,0);if(len<1 || len>=MAX_PATH)continue;SendMessageW(list,LB_GETTEXT,i,(LPARAM)name);if(!lstrcmpiW(name,L"first.txt")||!lstrcmpiW(name,L"second.txt")){if(SendMessageW(list,LB_SETSEL,TRUE,i)!=LB_ERR)++hits;}}
       if(hits==2){SendMessageW(w,WM_COMMAND,MAKEWPARAM(1120,LBN_SELCHANGE),(LPARAM)list);t->selected=1;InterlockedIncrement(&ready);}}
    if(t->selected && InterlockedCompareExchange(&ready,0,0)==2 && (!serial_close || t==base_task || InterlockedCompareExchange(&base_task->closed,0,0))){PostMessageW(w,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(w,IDOK));}}
static BOOL CALLBACK locate(HWND w,LPARAM ignored)
{(void)ignored;WCHAR title[64];if(local && IsWindowVisible(w) && GetWindowTextW(w,title,ARRAYSIZE(title)) && !lstrcmpW(title,local->title)){tick(w,WM_TIMER,0,0);return FALSE;}return TRUE;}
static void CALLBACK scan_timer(HWND w,UINT message,UINT_PTR timer,DWORD now)
{(void)w;(void)message;(void)timer;(void)now;EnumThreadWindows(GetCurrentThreadId(),locate,0);}
static LRESULT CALLBACK watch(int code,WPARAM wp,LPARAM lp){if(code>=0 && local){const CWPSTRUCT *p=(const CWPSTRUCT*)lp;if(p->message==WM_SHOWWINDOW && p->wParam){WCHAR title[80];GetWindowTextW(p->hwnd,title,80);if(!lstrcmpW(title,local->title))SetTimer(p->hwnd,0x7791,60,tick);}}return CallNextHookEx(NULL,code,wp,lp);}
static DWORD WINAPI execute(void *arg){task *t=arg;local=t;PCWSTR expected_dir=shared?base_task->dir:t->dir,expected_a=shared?base_task->a:t->a,expected_b=shared?base_task->b:t->b;HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 2;IFileOpenDialog *dialog=NULL;IShellItem *folder=NULL;FILEOPENDIALOGOPTIONS flags=0;HHOOK hook=NULL;
    hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileOpenDialog,(void**)&dialog);if(SUCCEEDED(hr))hr=IFileOpenDialog_SetTitle(dialog,t->title);if(SUCCEEDED(hr))hr=IFileOpenDialog_GetOptions(dialog,&flags);if(SUCCEEDED(hr)){t->options=flags|FOS_ALLOWMULTISELECT|FOS_FORCEFILESYSTEM;hr=IFileOpenDialog_SetOptions(dialog,t->options);}if(SUCCEEDED(hr))hr=SHCreateItemFromParsingName(expected_dir,NULL,&IID_IShellItem,(void**)&folder);if(SUCCEEDED(hr))hr=IFileOpenDialog_SetFolder(dialog,folder);if(folder)IShellItem_Release(folder);
    if(SUCCEEDED(hr) && modern){WCHAR input[MAX_PATH*2+8];int n=swprintf(input,ARRAYSIZE(input),L"\"%ls\" \"%ls\"",expected_a,expected_b);if(n<0 || (size_t)n>=ARRAYSIZE(input))hr=E_FAIL;else hr=IFileOpenDialog_SetFileName(dialog,input);}
    if(SUCCEEDED(hr)){hook=SetWindowsHookExW(WH_CALLWNDPROC,watch,NULL,GetCurrentThreadId());if(!hook)hr=E_FAIL;}
    UINT_PTR scan=SetTimer(NULL,0,60,scan_timer);if(!scan)hr=E_FAIL;
    if(SUCCEEDED(hr))hr=IFileOpenDialog_Show(dialog,NULL);
    InterlockedExchange(&t->closed,1);
    if(scan)KillTimer(NULL,scan);
    t->hr=hr;IShellItemArray *array=NULL;BOOL found_a=FALSE,found_b=FALSE;
    if(SUCCEEDED(hr))hr=IFileOpenDialog_GetResults(dialog,&array);
    if(SUCCEEDED(hr))hr=IShellItemArray_GetCount(array,&t->count);
    for(DWORD i=0;SUCCEEDED(hr)&&i<t->count;++i){IShellItem *item=NULL;PWSTR path=NULL;hr=IShellItemArray_GetItemAt(array,i,&item);if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);if(path){if(!lstrcmpiW(path,expected_a))found_a=TRUE;if(!lstrcmpiW(path,expected_b))found_b=TRUE;CoTaskMemFree(path);}if(item)IShellItem_Release(item);}
    t->paths=found_a && found_b;if(array)IShellItemArray_Release(array);if(hook)UnhookWindowsHookEx(hook);if(dialog)IFileOpenDialog_Release(dialog);CoUninitialize();return SUCCEEDED(hr)&&t->count==2&&t->paths?0:1;}
int wmain(int argc,WCHAR **argv){task t[2]={0};WCHAR root[MAX_PATH],initial_cwd[MAX_PATH]=L"",final_cwd[MAX_PATH]=L"";if(argc>3)return 2;if(argc==3 && (!SetEnvironmentVariableW(L"TEMP",argv[2]) || !SetEnvironmentVariableW(L"TMP",argv[2])))return 2;GetCurrentDirectoryW(MAX_PATH,initial_cwd);shared=argc>=2 && !lstrcmpW(argv[1],L"shared");modern=argc>=2 && (!lstrcmpW(argv[1],L"native") || !lstrcmpW(argv[1],L"native-serial"));serial_close=argc>=2 && (!lstrcmpW(argv[1],L"serial") || !lstrcmpW(argv[1],L"native-serial"));base_task=&t[0];if(!GetTempPathW(MAX_PATH,root))return 2;for(int i=0;i<2;++i){if(!GetTempFileNameW(root,L"v16",0,t[i].dir)||!DeleteFileW(t[i].dir)||!CreateDirectoryW(t[i].dir,NULL))return 3;swprintf(t[i].a,MAX_PATH,L"%ls\\first.txt",t[i].dir);swprintf(t[i].b,MAX_PATH,L"%ls\\second.txt",t[i].dir);swprintf(t[i].title,64,L"Concurrent %d",i);HANDLE f=CreateFileW(t[i].a,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(f==INVALID_HANDLE_VALUE)return 4;CloseHandle(f);f=CreateFileW(t[i].b,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(f==INVALID_HANDLE_VALUE)return 4;CloseHandle(f);}
    printf("P285_EXPECT first=[%ls] second=[%ls] distinct=%d\n",t[0].dir,t[1].dir,lstrcmpiW(t[0].dir,t[1].dir)!=0);
    DWORD tids[2]={0};HANDLE th[2]={CreateThread(NULL,0,execute,&t[0],0,&tids[0]),CreateThread(NULL,0,execute,&t[1],0,&tids[1])};if(!th[0] || !th[1])return 5;DWORD wait=WaitForMultipleObjects(2,th,TRUE,7000),code[2]={99,99};DWORD stopped=wait;if(wait!=WAIT_OBJECT_0){PostThreadMessageW(tids[0],WM_QUIT,0,0);PostThreadMessageW(tids[1],WM_QUIT,0,0);stopped=WaitForMultipleObjects(2,th,TRUE,5000);}if(stopped==WAIT_OBJECT_0){GetExitCodeThread(th[0],&code[0]);GetExitCodeThread(th[1],&code[1]);}CloseHandle(th[0]);CloseHandle(th[1]);BOOL clean=stopped==WAIT_OBJECT_0;GetCurrentDirectoryW(MAX_PATH,final_cwd);printf("CONCURRENT_CWD initial=[%ls] final=[%ls]\n",initial_cwd,final_cwd);if(stopped==WAIT_OBJECT_0 && !SetCurrentDirectoryW(root))clean=FALSE;if(stopped==WAIT_OBJECT_0)for(int i=0;i<2;++i){if(!DeleteFileW(t[i].a)||!DeleteFileW(t[i].b)||!RemoveDirectoryW(t[i].dir))clean=FALSE;}
    BOOL cwd_ok=initial_cwd[0] && final_cwd[0] && !lstrcmpiW(initial_cwd,final_cwd);
    printf("CONCURRENT bits=%u wait=%lu ready=%ld codes=%lu,%lu counts=%lu,%lu paths=%d,%d cleanup=%d stopped=%lu options=%08lx,%08lx cwd_ok=%d\n",(unsigned)(sizeof(void*)*8),wait,ready,code[0],code[1],t[0].count,t[1].count,t[0].paths,t[1].paths,clean,stopped,t[0].options,t[1].options,cwd_ok);return wait==WAIT_OBJECT_0 && !code[0] && !code[1] && clean && cwd_ok?0:1;}
