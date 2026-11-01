#include "global_attach.h"
#include "global_session.h"
#include "global_remote.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tlhelp32.h>

#define ATTACH_LOADING 1u
#define ATTACH_STARTING 2u
#define ATTACH_ACTIVE 3u

DWORD ntw16s_global_attach_close(ntw16s_global_attach *ticket)
{
    DWORD error;
    if(!ticket)return ERROR_INVALID_PARAMETER;
    error=ntw16s_global_start_close(&ticket->start);if(error)return error;
    error=ntw16s_global_load_discard(&ticket->load);if(error)return error;
    if(ticket->process)CloseHandle(ticket->process);
    free(ticket->dll_path);ZeroMemory(ticket,sizeof(*ticket));return ERROR_SUCCESS;
}

static DWORD revalidate(ntw16s_global_attach *ticket)
{return ticket->window?ntw16s_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity):
                       ntw16s_global_policy_revalidate_thread(ticket->process,&ticket->identity);}

/* A file may have been replaced since the target mapped it. Check the mapped
 * image header as well as comparing the files before reusing an instance. */
static DWORD mapped_identity(HANDLE process,DWORD pid,HMODULE remote,const WCHAR *name,const WCHAR *path)
{
    HMODULE local=GetModuleHandleW(name);IMAGE_DOS_HEADER rd,ld;IMAGE_NT_HEADERS rn,ln;SIZE_T n=0;
    BYTE local_code[16],remote_code[16];void *remote_worker=NULL;FARPROC local_worker;
    if(!local || !remote)return ERROR_MOD_NOT_FOUND;
    if(!ReadProcessMemory(process,remote,&rd,sizeof(rd),&n) || n!=sizeof(rd) || rd.e_magic!=IMAGE_DOS_SIGNATURE || rd.e_lfanew<64 || rd.e_lfanew>1024*1024)return ERROR_BAD_EXE_FORMAT;
    memcpy(&ld,local,sizeof(ld));
    if(ld.e_magic!=IMAGE_DOS_SIGNATURE || ld.e_lfanew<64 || ld.e_lfanew>1024*1024)return ERROR_BAD_EXE_FORMAT;
    if(!ReadProcessMemory(process,(BYTE *)remote+rd.e_lfanew,&rn,sizeof(rn),&n) || n!=sizeof(rn))return ERROR_PARTIAL_COPY;
    memcpy(&ln,(BYTE *)local+ld.e_lfanew,sizeof(ln));
    if(rn.Signature!=IMAGE_NT_SIGNATURE || ln.Signature!=IMAGE_NT_SIGNATURE ||
       rn.FileHeader.Machine!=ln.FileHeader.Machine || rn.FileHeader.TimeDateStamp!=ln.FileHeader.TimeDateStamp ||
       rn.OptionalHeader.Magic!=ln.OptionalHeader.Magic || rn.OptionalHeader.SizeOfImage!=ln.OptionalHeader.SizeOfImage ||
       rn.OptionalHeader.CheckSum!=ln.OptionalHeader.CheckSum)return ERROR_REVISION_MISMATCH;
    local_worker=GetProcAddress(local,"Ntw16sGlobalThread");if(!local_worker)return ERROR_PROC_NOT_FOUND;
    DWORD error=ntw16s_remote_global_thread(process,pid,path,&remote_worker);if(error)return error;
    if(!ReadProcessMemory(GetCurrentProcess(),(const void *)local_worker,local_code,sizeof(local_code),&n) || n!=sizeof(local_code) ||
       !ReadProcessMemory(process,remote_worker,remote_code,sizeof(remote_code),&n) || n!=sizeof(remote_code))return ERROR_PARTIAL_COPY;
    if(memcmp(local_code,remote_code,sizeof(local_code)))return ERROR_REVISION_MISMATCH;
    return ERROR_SUCCESS;
}

/* Reuse the first same-named module already in the target. The target's
 * GetModuleHandle uses this same load order when the global hook fires. */
static DWORD select_path(HANDLE process,DWORD pid,const WCHAR *requested,WCHAR *selected,DWORD capacity)
{
    typedef DWORD (WINAPI *module_path_fn)(HANDLE,HMODULE,LPWSTR,DWORD);
    module_path_fn module_path=(module_path_fn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"K32GetModuleFileNameExW");
    const WCHAR *base=wcsrchr(requested,L'\\');HANDLE snapshot=INVALID_HANDLE_VALUE;MODULEENTRY32W e={.dwSize=sizeof(e)};
    DWORD error=ERROR_SUCCESS;BOOL found=FALSE;size_t length;HMODULE found_module=NULL;
    if(!base || !base[1] || !module_path)return ERROR_INVALID_PARAMETER;
    for(unsigned i=0;i<8;++i){snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);if(snapshot!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;}
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    if(!Module32FirstW(snapshot,&e)){error=GetLastError();goto done;}
    do{
        if(_wcsicmp(e.szModule,base+1))continue;
        DWORD n=module_path(process,e.hModule,selected,capacity);
        if(!n || n>=capacity){error=n?ERROR_INSUFFICIENT_BUFFER:GetLastError();goto done;}
        found=TRUE;found_module=e.hModule;break;
    }while(Module32NextW(snapshot,&e));
    if(!found){
        if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
        length=wcslen(requested);if(length>=capacity){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
        memcpy(selected,requested,(length+1)*sizeof(WCHAR));
    }else if(_wcsicmp(selected,requested)){
        if(!ntw16s_global_files_equal(selected,requested))error=ERROR_REVISION_MISMATCH;
        else error=mapped_identity(process,pid,found_module,base+1,selected);
    }
done:
    CloseHandle(snapshot);return error;
}

static DWORD begin(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket)
{
    DWORD error;size_t length;
    if(!policy || !pid || !thread_id || !cookie || !dll_path || !*dll_path || !ticket)return ERROR_INVALID_PARAMETER;
    if(ticket->process || ticket->dll_path || ticket->phase)return ERROR_BUSY;
    error=window?ntw16s_global_policy_inspect(policy,window,pid,thread_id,machine,&ticket->identity):
                 ntw16s_global_policy_inspect_thread(policy,pid,thread_id,machine,&ticket->identity);if(error)return error;
    ticket->process=OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|
                                PROCESS_VM_WRITE|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid);
    if(!ticket->process){error=GetLastError();goto fail;}
    ticket->window=window;error=revalidate(ticket);if(error)goto fail;
    length=wcslen(dll_path);if(length>=32752){error=ERROR_FILENAME_EXCED_RANGE;goto fail;}
    ticket->dll_path=(WCHAR *)calloc(32752,sizeof(WCHAR));if(!ticket->dll_path){error=ERROR_NOT_ENOUGH_MEMORY;goto fail;}
    error=select_path(ticket->process,pid,dll_path,ticket->dll_path,32752);if(error)goto fail;
    ticket->window=window;ticket->cookie=cookie;
    error=ntw16s_global_load_begin(ticket->process,pid,ticket->dll_path,&ticket->load);if(error)goto fail;
    ticket->phase=ATTACH_LOADING;return ERROR_SUCCESS;
fail:
    {DWORD release=ntw16s_global_attach_close(ticket);return release?release:error;}
}

DWORD ntw16s_global_attach_begin(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket)
{return window?begin(policy,window,pid,thread_id,machine,cookie,dll_path,ticket):ERROR_INVALID_PARAMETER;}
DWORD ntw16s_global_attach_begin_thread(const ntw16s_global_policy *policy,DWORD pid,DWORD thread_id,
                                       WORD machine,DWORD cookie,const WCHAR *dll_path,ntw16s_global_attach *ticket)
{return begin(policy,NULL,pid,thread_id,machine,cookie,dll_path,ticket);}

static DWORD control(ntw16s_global_attach *ticket,LPARAM action)
{
    DWORD error=ntw16s_global_policy_revalidate(ticket->process,ticket->window,&ticket->identity);
    if(error)return error;
    UINT message=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);if(!message)return GetLastError();
    DWORD_PTR ignored=0;
    if(!SendMessageTimeoutW(ticket->window,message,ticket->cookie,action,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,100,&ignored)){
        error=GetLastError();return error?error:ERROR_TIMEOUT;
    }
    if(action==NTW16S_GLOBAL_APPLY){
        DWORD_PTR ack=0;
        if(!SendMessageTimeoutW(ticket->window,message,ticket->cookie,NTW16S_GLOBAL_QUERY,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,100,&ack)){
            error=GetLastError();return error?error:ERROR_TIMEOUT;
        }
        if(ack!=NTW16S_GLOBAL_ATTACHED)return ERROR_NOT_READY;
    }
    return ERROR_SUCCESS;
}

DWORD ntw16s_global_attach_advance(ntw16s_global_attach *ticket,SECURITY_ATTRIBUTES *security)
{
    DWORD error;void *worker=NULL;
    if(!ticket || !ticket->process || !ticket->dll_path || !ticket->phase)return ERROR_INVALID_PARAMETER;
    if(ticket->phase==ATTACH_LOADING){
        error=ntw16s_global_load_finish(&ticket->load,0,&worker);
        if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;
        if(error)return error;
        error=revalidate(ticket);if(error)return error;
        error=ntw16s_global_start_begin(ticket->process,ticket->identity.pid,ticket->identity.thread_id,ticket->cookie,
                                        ticket->dll_path,security,&ticket->start);
        if(error)return error;
        ticket->phase=ATTACH_STARTING;
    }
    if(ticket->phase==ATTACH_STARTING){
        error=ntw16s_global_start_ready(&ticket->start,0);
        if(error==ERROR_TIMEOUT)return ERROR_IO_PENDING;
        if(error)return error;
        ticket->phase=ATTACH_ACTIVE;
        if(!ticket->window)return ERROR_SUCCESS;
        ticket->control_error=control(ticket,NTW16S_GLOBAL_APPLY);
        return ticket->control_error;
    }
    return ticket->phase==ATTACH_ACTIVE?ticket->control_error:ERROR_INVALID_STATE;
}

DWORD ntw16s_global_attach_remove(ntw16s_global_attach *ticket)
{
    if(!ticket || ticket->phase!=ATTACH_ACTIVE)return ERROR_INVALID_STATE;
    return control(ticket,NTW16S_GLOBAL_REMOVE);
}

DWORD ntw16s_global_attach_apply(ntw16s_global_attach *ticket)
{
    if(!ticket || ticket->phase!=ATTACH_ACTIVE)return ERROR_INVALID_STATE;
    ticket->control_error=control(ticket,NTW16S_GLOBAL_APPLY);
    return ticket->control_error;
}
