#include "explorer.h"
#include "window_target.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* A pre-launch snapshot ensures that an already open folder, the desktop and
 * the taskbar are never attached. Ambiguous concurrent creations are rejected. */
typedef struct folder { HWND window; DWORD pid,thread; } folder;
struct run16s_explorer_snapshot { folder items[256]; PCWSTR class_name; unsigned count; BOOL overflow; };

static BOOL CALLBACK collect(HWND w,LPARAM data)
{
    run16s_explorer_snapshot *s=(run16s_explorer_snapshot *)data;
    WCHAR name[64];DWORD pid=0;
    if(!GetClassNameW(w,name,ARRAYSIZE(name)) || wcscmp(name,s->class_name))return TRUE;
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
BOOL run16s_terminal_image(PCWSTR image)
{
    const WCHAR *base=image?wcsrchr(image,L'\\'):NULL;
    return base && !_wcsicmp(base+1,L"WindowsTerminal.exe");
}
static run16s_explorer_snapshot *snapshot_create(PCWSTR class_name)
{
    run16s_explorer_snapshot *s=calloc(1,sizeof(*s));
    if(!s)return NULL;
    s->class_name=class_name;
    if(!EnumWindows(collect,(LPARAM)s) || s->overflow){free(s);return NULL;}
    return s;
}
run16s_explorer_snapshot *run16s_explorer_snapshot_create(void)
{return snapshot_create(L"CabinetWClass");}
run16s_explorer_snapshot *run16s_terminal_snapshot_create(void)
{return snapshot_create(L"CASCADIA_HOSTING_WINDOW_CLASS");}
void run16s_explorer_snapshot_free(run16s_explorer_snapshot *s){free(s);}

DWORD run16s_explorer_attach_new(run16s_explorer_snapshot *before,PCWSTR image,PCWSTR directory)
{
    if(!before || !image || !directory)return ERROR_INVALID_PARAMETER;
    ULONGLONG deadline=GetTickCount64()+5000;
    ULONGLONG stable_since=0;folder previous={0};
    do {
        run16s_explorer_snapshot now={.class_name=before->class_name};folder candidate={0};unsigned count=0;
        if(!EnumWindows(collect,(LPARAM)&now) || now.overflow)return ERROR_MORE_DATA;
        for(unsigned i=0;i<now.count;i++){
            folder *f=&now.items[i];
            if(!listed(before,f) && IsWindowVisible(f->window) && image_matches(f->pid,image)){candidate=*f;count++;}
        }
        if(count>1)return ERROR_MORE_DATA;
        if(count==1){
            if(candidate.window!=previous.window || candidate.pid!=previous.pid || candidate.thread!=previous.thread){
                previous=candidate;stable_since=GetTickCount64();
            }else if(GetTickCount64()-stable_since>=200)return run16s_window_attach(candidate.window,candidate.pid,candidate.thread,directory);
        }else {previous=(folder){0};stable_since=0;}
        Sleep(50);
    }while(GetTickCount64()<deadline);
    return ERROR_NOT_FOUND;
}
