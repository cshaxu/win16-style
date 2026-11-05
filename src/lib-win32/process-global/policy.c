#include "policy.h"
#include "identity.h"
#include "lib-win32/process-image/image.h"
#include <stdlib.h>
#include <wchar.h>

DWORD win32_process_global_policy_init(win32_process_global_policy *policy)
{
    HANDLE token=NULL;TOKEN_USER *user=NULL;DWORD error;
    if(!policy)return ERROR_INVALID_PARAMETER;
    ZeroMemory(policy,sizeof(*policy));
    if(!ProcessIdToSessionId(GetCurrentProcessId(),&policy->session))return GetLastError();
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return GetLastError();
    error=win32_process_global_token_user(token,&user);CloseHandle(token);if(error)return error;
    if(!IsValidSid(user->User.Sid) || GetLengthSid(user->User.Sid)>sizeof(policy->sid))error=ERROR_INVALID_SID;
    else{
        policy->sid_bytes=GetLengthSid(user->User.Sid);
        if(!CopySid(sizeof(policy->sid),policy->sid,user->User.Sid))error=GetLastError();
    }
    free(user);if(error)ZeroMemory(policy,sizeof(*policy));return error;
}

static DWORD inspect(const win32_process_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                   WORD expected_machine,win32_process_global_identity *identity,win32_process_global_image_filter allowed)
{
    HANDLE process=NULL,thread=NULL,token=NULL;TOKEN_USER *user=NULL;DWORD error=ERROR_SUCCESS,actual=0,session=0;
    FILETIME created,exited,kernel,user_time;WORD machine=0;DWORD appcontainer=0,bytes=0;
    WCHAR *image=NULL,windows[MAX_PATH];DWORD length=32752;UINT windows_length;
    if(identity)ZeroMemory(identity,sizeof(*identity));
    if(!policy || !policy->sid_bytes || !identity || !allowed || !pid || !thread_id || pid==GetCurrentProcessId() ||
       (expected_machine!=IMAGE_FILE_MACHINE_I386 && expected_machine!=IMAGE_FILE_MACHINE_AMD64))return ERROR_INVALID_PARAMETER;
    if(policy->sid_bytes>sizeof(policy->sid) || !IsValidSid((PSID)policy->sid) ||
       GetLengthSid((PSID)policy->sid)!=policy->sid_bytes)return ERROR_INVALID_SID;
    if(window && (GetWindowThreadProcessId(window,&actual)!=thread_id || actual!=pid || !IsWindowVisible(window)))return ERROR_INVALID_WINDOW_HANDLE;
    if(!window){error=win32_process_global_desktop_status(thread_id);if(error && error!=ERROR_NOT_READY)return error;}
    if(!ProcessIdToSessionId(pid,&session))return GetLastError();
    if(session!=policy->session)return ERROR_ACCESS_DENIED;
    process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);
    if(!process)return GetLastError();
    if(WaitForSingleObject(process,0)!=WAIT_TIMEOUT){error=ERROR_PROCESS_ABORTED;goto done;}
    if(!win32_process_global_birth(process,&identity->process_birth_low,&identity->process_birth_high)){error=GetLastError();goto done;}
    error=win32_process_image_machine(process,&machine);if(error)goto done;
    identity->machine=machine;
    if(identity->machine!=expected_machine){error=ERROR_BAD_EXE_FORMAT;goto done;}
    if(!OpenProcessToken(process,TOKEN_QUERY,&token)){error=GetLastError();goto done;}
    error=win32_process_global_token_user(token,&user);if(error)goto done;
    if(!IsValidSid(user->User.Sid) || !EqualSid((PSID)policy->sid,user->User.Sid)){error=ERROR_ACCESS_DENIED;goto done;}
    if(!GetTokenInformation(token,TokenIsAppContainer,&appcontainer,sizeof(appcontainer),&bytes)){error=GetLastError();goto done;}
    if(appcontainer){error=ERROR_ACCESS_DENIED;goto done;}
    image=(WCHAR *)calloc(length,sizeof(WCHAR));if(!image){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    windows_length=GetWindowsDirectoryW(windows,ARRAYSIZE(windows));
    if(!windows_length){error=GetLastError();if(!error)error=ERROR_INVALID_DATA;goto done;}
    if(windows_length>=ARRAYSIZE(windows)){error=ERROR_INSUFFICIENT_BUFFER;goto done;}
    if(!QueryFullProcessImageNameW(process,0,image,&length)){error=GetLastError();goto done;}
    if(!allowed(image,windows)){error=ERROR_ACCESS_DENIED;goto done;}
    thread=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,thread_id);
    if(!thread){error=GetLastError();goto done;}
    if(GetProcessIdOfThread(thread)!=pid || WaitForSingleObject(thread,0)!=WAIT_TIMEOUT){error=ERROR_INVALID_THREAD_ID;goto done;}
    if(!GetThreadTimes(thread,&created,&exited,&kernel,&user_time)){error=GetLastError();goto done;}
    identity->thread_birth_low=created.dwLowDateTime;identity->thread_birth_high=created.dwHighDateTime;
    if(!window){error=win32_process_global_desktop_status(thread_id);if(error)goto done;}
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

static DWORD revalidate(HANDLE process,HWND window,const win32_process_global_identity *identity)
{
    HANDLE thread=NULL;FILETIME created,exited,kernel,user_time;DWORD actual=0,error=ERROR_SUCCESS;
    if(!process || !identity || !identity->pid || !identity->thread_id ||
       GetProcessId(process)!=identity->pid)return ERROR_INVALID_PARAMETER;
    if(WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return ERROR_PROCESS_ABORTED;
    DWORD birth_low=0,birth_high=0;
    if(!win32_process_global_birth(process,&birth_low,&birth_high))return GetLastError();
    if(birth_low!=identity->process_birth_low || birth_high!=identity->process_birth_high)
        return ERROR_INVALID_STATE;
    thread=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,identity->thread_id);
    if(!thread)return GetLastError();
    if(GetProcessIdOfThread(thread)!=identity->pid || WaitForSingleObject(thread,0)!=WAIT_TIMEOUT){error=ERROR_INVALID_THREAD_ID;goto done;}
    if(!GetThreadTimes(thread,&created,&exited,&kernel,&user_time)){error=GetLastError();goto done;}
    if(created.dwLowDateTime!=identity->thread_birth_low || created.dwHighDateTime!=identity->thread_birth_high){error=ERROR_INVALID_STATE;goto done;}
    if((window && (GetWindowThreadProcessId(window,&actual)!=identity->thread_id || actual!=identity->pid || !IsWindowVisible(window))) ||
       (!window && win32_process_global_desktop_status(identity->thread_id)!=ERROR_SUCCESS)){
        error=ERROR_INVALID_WINDOW_HANDLE;goto done;
    }
    if(WaitForSingleObject(process,0)!=WAIT_TIMEOUT || WaitForSingleObject(thread,0)!=WAIT_TIMEOUT)error=ERROR_INVALID_STATE;
done:
    CloseHandle(thread);return error;
}

DWORD win32_process_global_policy_inspect(const win32_process_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                   WORD machine,win32_process_global_identity *identity,win32_process_global_image_filter allowed)
{return window?inspect(policy,window,pid,thread_id,machine,identity,allowed):ERROR_INVALID_PARAMETER;}
DWORD win32_process_global_policy_inspect_thread(const win32_process_global_policy *policy,DWORD pid,DWORD thread_id,
                                          WORD machine,win32_process_global_identity *identity,win32_process_global_image_filter allowed)
{return inspect(policy,NULL,pid,thread_id,machine,identity,allowed);}
DWORD win32_process_global_policy_revalidate(HANDLE process,HWND window,const win32_process_global_identity *identity)
{return window?revalidate(process,window,identity):ERROR_INVALID_PARAMETER;}
DWORD win32_process_global_policy_revalidate_thread(HANDLE process,const win32_process_global_identity *identity)
{return revalidate(process,NULL,identity);}
