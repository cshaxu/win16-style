#include "lib-win32/handle/duplicate.h"
#include "global_policy.h"
#include "lib-win32/process-image/image.h"
#include <stdlib.h>
#include <wchar.h>

BOOL ntw16s_global_image_allowed(const WCHAR *image,const WCHAR *windows)
{
    if(!image || !*image || !windows || !*windows)return FALSE;
    size_t length=wcslen(windows);
    if(!_wcsnicmp(image,windows,length) && image[length]==L'\\'){
        const WCHAR *base=wcsrchr(image,L'\\');base=base?base+1:image;
        static const WCHAR *const shared[]={L"explorer.exe",L"ApplicationFrameHost.exe",L"ShellExperienceHost.exe",
            L"StartMenuExperienceHost.exe",L"SearchHost.exe",L"SearchApp.exe",L"sihost.exe",L"taskhostw.exe",
            L"RuntimeBroker.exe",L"dllhost.exe"};
        for(unsigned i=0;i<ARRAYSIZE(shared);++i)if(!lstrcmpiW(base,shared[i]))return FALSE;
    }
    return TRUE;
}

static DWORD token_user(HANDLE token,TOKEN_USER **user)
{
    DWORD bytes=0;*user=NULL;
    if(GetTokenInformation(token,TokenUser,NULL,0,&bytes))return ERROR_INVALID_DATA;
    DWORD query_error=GetLastError();
    if(query_error!=ERROR_INSUFFICIENT_BUFFER)return query_error?query_error:ERROR_INVALID_DATA;
    if(bytes<sizeof(TOKEN_USER) || bytes>65536)return ERROR_INVALID_DATA;
    *user=(TOKEN_USER *)malloc(bytes);if(!*user)return ERROR_NOT_ENOUGH_MEMORY;
    if(!GetTokenInformation(token,TokenUser,*user,bytes,&bytes)){DWORD error=GetLastError();free(*user);*user=NULL;return error;}
    return ERROR_SUCCESS;
}

DWORD ntw16s_global_policy_init(ntw16s_global_policy *policy)
{
    HANDLE token=NULL;TOKEN_USER *user=NULL;DWORD error;
    if(!policy)return ERROR_INVALID_PARAMETER;
    ZeroMemory(policy,sizeof(*policy));
    if(!ProcessIdToSessionId(GetCurrentProcessId(),&policy->session))return GetLastError();
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return GetLastError();
    error=token_user(token,&user);CloseHandle(token);if(error)return error;
    if(!IsValidSid(user->User.Sid) || GetLengthSid(user->User.Sid)>sizeof(policy->sid))error=ERROR_INVALID_SID;
    else{
        policy->sid_bytes=GetLengthSid(user->User.Sid);
        if(!CopySid(sizeof(policy->sid),policy->sid,user->User.Sid))error=GetLastError();
    }
    free(user);if(error)ZeroMemory(policy,sizeof(*policy));return error;
}

static DWORD desktop_status(DWORD thread_id)
{
    HDESK a=GetThreadDesktop(GetCurrentThreadId());SetLastError(ERROR_SUCCESS);HDESK b=GetThreadDesktop(thread_id);DWORD target_error=GetLastError();
    win32_handle_compare_fn compare=win32_handle_compare_provider();
    /* Names alone do not distinguish desktops in separate window stations. */
    if(!a || !compare)return ERROR_ACCESS_DENIED;
    /* A freshly created thread may not yet be connected to a USER desktop.
     * A null handle without an error is not evidence of a different desktop. */
    if(!b)return target_error?target_error:ERROR_NOT_READY;
    return compare(a,b)?ERROR_SUCCESS:ERROR_ACCESS_DENIED;
}

static DWORD inspect(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                   WORD expected_machine,ntw16s_global_identity *identity)
{
    HANDLE process=NULL,thread=NULL,token=NULL;TOKEN_USER *user=NULL;DWORD error=ERROR_SUCCESS,actual=0,session=0;
    FILETIME created,exited,kernel,user_time;WORD machine=0;DWORD appcontainer=0,bytes=0;
    WCHAR *image=NULL,windows[MAX_PATH];DWORD length=32752;UINT windows_length;
    if(identity)ZeroMemory(identity,sizeof(*identity));
    if(!policy || !policy->sid_bytes || !identity || !pid || !thread_id || pid==GetCurrentProcessId() ||
       (expected_machine!=IMAGE_FILE_MACHINE_I386 && expected_machine!=IMAGE_FILE_MACHINE_AMD64))return ERROR_INVALID_PARAMETER;
    if(policy->sid_bytes>sizeof(policy->sid) || !IsValidSid((PSID)policy->sid) ||
       GetLengthSid((PSID)policy->sid)!=policy->sid_bytes)return ERROR_INVALID_SID;
    if(window && (GetWindowThreadProcessId(window,&actual)!=thread_id || actual!=pid || !IsWindowVisible(window)))return ERROR_INVALID_WINDOW_HANDLE;
    if(!window){error=desktop_status(thread_id);if(error && error!=ERROR_NOT_READY)return error;}
    if(!ProcessIdToSessionId(pid,&session))return GetLastError();
    if(session!=policy->session)return ERROR_ACCESS_DENIED;
    process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);
    if(!process)return GetLastError();
    if(WaitForSingleObject(process,0)!=WAIT_TIMEOUT){error=ERROR_PROCESS_ABORTED;goto done;}
    if(!GetProcessTimes(process,&created,&exited,&kernel,&user_time)){error=GetLastError();goto done;}
    identity->process_birth_low=created.dwLowDateTime;identity->process_birth_high=created.dwHighDateTime;
    error=win32_process_image_machine(process,&machine);if(error)goto done;
    identity->machine=machine;
    if(identity->machine!=expected_machine){error=ERROR_BAD_EXE_FORMAT;goto done;}
    if(!OpenProcessToken(process,TOKEN_QUERY,&token)){error=GetLastError();goto done;}
    error=token_user(token,&user);if(error)goto done;
    if(!IsValidSid(user->User.Sid) || !EqualSid((PSID)policy->sid,user->User.Sid)){error=ERROR_ACCESS_DENIED;goto done;}
    if(!GetTokenInformation(token,TokenIsAppContainer,&appcontainer,sizeof(appcontainer),&bytes)){error=GetLastError();goto done;}
    if(appcontainer){error=ERROR_ACCESS_DENIED;goto done;}
    image=(WCHAR *)calloc(length,sizeof(WCHAR));if(!image){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    windows_length=GetWindowsDirectoryW(windows,ARRAYSIZE(windows));
    if(!windows_length){error=GetLastError();if(!error)error=ERROR_INVALID_DATA;goto done;}
    if(windows_length>=ARRAYSIZE(windows)){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    if(!QueryFullProcessImageNameW(process,0,image,&length)){error=GetLastError();goto done;}
    if(!ntw16s_global_image_allowed(image,windows)){error=ERROR_ACCESS_DENIED;goto done;}
    thread=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,thread_id);
    if(!thread){error=GetLastError();goto done;}
    if(GetProcessIdOfThread(thread)!=pid || WaitForSingleObject(thread,0)!=WAIT_TIMEOUT){error=ERROR_INVALID_THREAD_ID;goto done;}
    if(!GetThreadTimes(thread,&created,&exited,&kernel,&user_time)){error=GetLastError();goto done;}
    identity->thread_birth_low=created.dwLowDateTime;identity->thread_birth_high=created.dwHighDateTime;
    if(!window){error=desktop_status(thread_id);if(error)goto done;}
    if((window && (GetWindowThreadProcessId(window,&actual)!=thread_id || actual!=pid || !IsWindowVisible(window))) ||
       WaitForSingleObject(process,0)!=WAIT_TIMEOUT || WaitForSingleObject(thread,0)!=WAIT_TIMEOUT){
        error=ERROR_INVALID_WINDOW_HANDLE;goto done;
    }
    identity->pid=pid;identity->thread_id=thread_id;
done:
    free(image);free(user);if(token)CloseHandle(token);if(thread)CloseHandle(thread);if(process)CloseHandle(process);
    if(error)ZeroMemory(identity,sizeof(*identity));
    return error;
}

static DWORD revalidate(HANDLE process,HWND window,const ntw16s_global_identity *identity)
{
    HANDLE thread=NULL;FILETIME created,exited,kernel,user_time;DWORD actual=0,error=ERROR_SUCCESS;
    if(!process || !identity || !identity->pid || !identity->thread_id ||
       GetProcessId(process)!=identity->pid)return ERROR_INVALID_PARAMETER;
    if(WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return ERROR_PROCESS_ABORTED;
    if(!GetProcessTimes(process,&created,&exited,&kernel,&user_time))return GetLastError();
    if(created.dwLowDateTime!=identity->process_birth_low || created.dwHighDateTime!=identity->process_birth_high)
        return ERROR_INVALID_STATE;
    thread=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,identity->thread_id);
    if(!thread)return GetLastError();
    if(GetProcessIdOfThread(thread)!=identity->pid || WaitForSingleObject(thread,0)!=WAIT_TIMEOUT){error=ERROR_INVALID_THREAD_ID;goto done;}
    if(!GetThreadTimes(thread,&created,&exited,&kernel,&user_time)){error=GetLastError();goto done;}
    if(created.dwLowDateTime!=identity->thread_birth_low || created.dwHighDateTime!=identity->thread_birth_high){error=ERROR_INVALID_STATE;goto done;}
    if((window && (GetWindowThreadProcessId(window,&actual)!=identity->thread_id || actual!=identity->pid || !IsWindowVisible(window))) ||
       (!window && desktop_status(identity->thread_id)!=ERROR_SUCCESS)){
        error=ERROR_INVALID_WINDOW_HANDLE;goto done;
    }
    if(WaitForSingleObject(process,0)!=WAIT_TIMEOUT || WaitForSingleObject(thread,0)!=WAIT_TIMEOUT)error=ERROR_INVALID_STATE;
done:
    CloseHandle(thread);return error;
}

DWORD ntw16s_global_policy_inspect(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                   WORD machine,ntw16s_global_identity *identity)
{return window?inspect(policy,window,pid,thread_id,machine,identity):ERROR_INVALID_PARAMETER;}
DWORD ntw16s_global_policy_inspect_thread(const ntw16s_global_policy *policy,DWORD pid,DWORD thread_id,
                                          WORD machine,ntw16s_global_identity *identity)
{return inspect(policy,NULL,pid,thread_id,machine,identity);}
DWORD ntw16s_global_policy_revalidate(HANDLE process,HWND window,const ntw16s_global_identity *identity)
{return window?revalidate(process,window,identity):ERROR_INVALID_PARAMETER;}
DWORD ntw16s_global_policy_revalidate_thread(HANDLE process,const ntw16s_global_identity *identity)
{return revalidate(process,NULL,identity);}
