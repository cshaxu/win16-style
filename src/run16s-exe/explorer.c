#include "explorer.h"
#include "ntw16s-dll/window_thread.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <wchar.h>

/* A pre-launch snapshot ensures that an already open folder, the desktop and
 * the taskbar are never attached. Ambiguous concurrent creations are rejected. */
typedef struct folder { HWND window; DWORD pid,thread; } folder;
struct run16s_explorer_snapshot { folder items[256]; unsigned count; BOOL overflow; };

static BOOL CALLBACK collect(HWND w,LPARAM data)
{
    run16s_explorer_snapshot *s=(run16s_explorer_snapshot *)data;
    WCHAR name[64];DWORD pid=0;
    if(!GetClassNameW(w,name,ARRAYSIZE(name)) || wcscmp(name,L"CabinetWClass"))return TRUE;
    if(s->count==ARRAYSIZE(s->items)){s->overflow=TRUE;return FALSE;}
    DWORD thread=GetWindowThreadProcessId(w,&pid);
    if(thread && pid)s->items[s->count++]=(folder){w,pid,thread};
    return TRUE;
}
static BOOL listed(const run16s_explorer_snapshot *s,const folder *f)
{
    for(unsigned i=0;i<s->count;i++)
        if(s->items[i].window==f->window && s->items[i].pid==f->pid && s->items[i].thread==f->thread)return TRUE;
    return FALSE;
}
static BOOL image_matches(DWORD pid,PCWSTR expected)
{
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
    if(!process)return FALSE;
    WCHAR image[32768];DWORD length=ARRAYSIZE(image);
    BOOL match=QueryFullProcessImageNameW(process,0,image,&length) && !_wcsicmp(image,expected);
    CloseHandle(process);return match;
}
BOOL run16s_explorer_default(PCWSTR image,PCWSTR arguments)
{
    if(!image || !arguments)return FALSE;
    while(*arguments==L' ' || *arguments==L'\t')arguments++;
    if(*arguments)return FALSE;
    WCHAR windows[MAX_PATH];UINT length=GetWindowsDirectoryW(windows,ARRAYSIZE(windows));
    if(!length || length+14>=ARRAYSIZE(windows))return FALSE;
    wcscpy(windows+length,L"\\explorer.exe");return !_wcsicmp(image,windows);
}
run16s_explorer_snapshot *run16s_explorer_snapshot_create(void)
{
    run16s_explorer_snapshot *s=calloc(1,sizeof(*s));
    if(!s)return NULL;
    if(!EnumWindows(collect,(LPARAM)s) || s->overflow){free(s);return NULL;}
    return s;
}
void run16s_explorer_snapshot_free(run16s_explorer_snapshot *s){free(s);}

/* SetWindowsHookEx uses the DLL's code offset in the target. If an older DLL
 * is already mapped there, the same offset need not be the same function. */
static DWORD compatible_module(DWORD pid,HMODULE module,HOOKPROC callback,PCWSTR path)
{
    HANDLE list=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
    if(list==INVALID_HANDLE_VALUE)return GetLastError();
    MODULEENTRY32W entry={.dwSize=sizeof(entry)};DWORD error=0;
    if(!Module32FirstW(list,&entry)){error=GetLastError();CloseHandle(list);return error;}
    do {
        if(_wcsicmp(entry.szModule,L"ntw16s64.dll"))continue;
        if(_wcsicmp(entry.szExePath,path)){error=ERROR_REVISION_MISMATCH;break;}
        HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,FALSE,pid);
        if(!process){error=GetLastError();break;}
        BYTE remote[64];SIZE_T received=0;uintptr_t offset=(uintptr_t)callback-(uintptr_t)module;
        if(offset+sizeof(remote)>entry.modBaseSize ||
           !ReadProcessMemory(process,entry.modBaseAddr+offset,remote,sizeof(remote),&received) ||
           received!=sizeof(remote) || memcmp(remote,(const void *)(uintptr_t)callback,sizeof(remote)))
            error=ERROR_REVISION_MISMATCH;
        CloseHandle(process);break;
    }while(Module32NextW(list,&entry));
    CloseHandle(list);return error;
}

static DWORD attach(const folder *f,PCWSTR directory)
{
    WCHAR path[32768];size_t length=wcslen(directory);
    if(length+14>=ARRAYSIZE(path))return ERROR_FILENAME_EXCED_RANGE;
    wcscpy(path,directory);wcscat(path,L"ntw16s64.dll");
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module)return GetLastError();
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sWindowThreadHook");
    DWORD error=callback?compatible_module(f->pid,module,callback,path):ERROR_PROC_NOT_FOUND;
    HHOOK hook=!error?SetWindowsHookExW(WH_CALLWNDPROC,callback,module,f->thread):NULL;
    if(!hook && !error)error=GetLastError();
    if(!hook && !error)error=ERROR_PROC_NOT_FOUND;
    if(hook){
        DWORD pid=0,thread=GetWindowThreadProcessId(f->window,&pid);DWORD_PTR ignored=0;
        UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
        if(pid!=f->pid || thread!=f->thread || !message)error=ERROR_INVALID_WINDOW_HANDLE;
        else {
            RemovePropW(f->window,NTW16S_WINDOW_THREAD_RESULT);
            if(!SendMessageTimeoutW(f->window,message,NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)f->window,
                 SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,3000,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
            else {uintptr_t result=(uintptr_t)GetPropW(f->window,NTW16S_WINDOW_THREAD_RESULT);
                error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
            RemovePropW(f->window,NTW16S_WINDOW_THREAD_RESULT);
        }
        if(!UnhookWindowsHookEx(hook) && !error)error=GetLastError();
    }
    FreeLibrary(module);return error;
}
DWORD run16s_explorer_attach_new(run16s_explorer_snapshot *before,PCWSTR image,PCWSTR directory)
{
    if(!before || !image || !directory)return ERROR_INVALID_PARAMETER;
    ULONGLONG deadline=GetTickCount64()+5000;
    ULONGLONG stable_since=0;folder previous={0};
    do {
        run16s_explorer_snapshot now={0};folder candidate={0};unsigned count=0;
        if(!EnumWindows(collect,(LPARAM)&now) || now.overflow)return ERROR_MORE_DATA;
        for(unsigned i=0;i<now.count;i++){
            folder *f=&now.items[i];
            if(!listed(before,f) && IsWindowVisible(f->window) && image_matches(f->pid,image)){candidate=*f;count++;}
        }
        if(count>1)return ERROR_MORE_DATA;
        if(count==1){
            if(candidate.window!=previous.window || candidate.pid!=previous.pid || candidate.thread!=previous.thread){
                previous=candidate;stable_since=GetTickCount64();
            }else if(GetTickCount64()-stable_since>=200)return attach(&candidate,directory);
        }else {previous=(folder){0};stable_since=0;}
        Sleep(50);
    }while(GetTickCount64()<deadline);
    return ERROR_NOT_FOUND;
}
