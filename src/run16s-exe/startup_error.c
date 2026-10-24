#include "startup_error.h"
#include <objbase.h>

BOOL run16s_private_console(void)
{
    DWORD processes[2];STARTUPINFOW startup={.cb=sizeof(startup)};
    GetStartupInfoW(&startup);
    /* Snapshot before any target is created. No console, failed query, or a
     * console shared with a shell stays on stderr, including headless jobs.
     * A sole attached process has no parent shell to preserve the message. */
    /* CREATE_NO_WINDOW still has a console process list but no HWND. The
     * console HWND may belong to another desktop, so IsWindowVisible is not
     * an attachment test. Respect an explicitly hidden startup as headless. */
    return GetConsoleWindow() &&
        (!(startup.dwFlags&STARTF_USESHOWWINDOW) || startup.wShowWindow!=SW_HIDE) &&
        GetConsoleProcessList(processes,2)==1 && processes[0]==GetCurrentProcessId();
}
BOOL run16s_legacy_image(PCWSTR image)
{
    HANDLE file=CreateFileW(image,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,
        NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE)return FALSE;
    IMAGE_DOS_HEADER dos;DWORD read,signature=0;BOOL legacy=FALSE;
    if(ReadFile(file,&dos,sizeof(dos),&read,NULL) && read==sizeof(dos) &&
            dos.e_magic==IMAGE_DOS_SIGNATURE){
        legacy=TRUE;
        if(dos.e_lfanew>=0){
            LARGE_INTEGER offset;offset.QuadPart=dos.e_lfanew;
            if(!SetFilePointerEx(file,offset,NULL,FILE_BEGIN) ||
                    !ReadFile(file,&signature,sizeof(signature),&read,NULL))legacy=FALSE;
            else if(read==sizeof(signature) && signature==IMAGE_NT_SIGNATURE)legacy=FALSE;
        }
    }
    CloseHandle(file);return legacy;
}
DWORD run16s_system_error(PCWSTR image,PCWSTR arguments,
    BOOL (WINAPI *execute)(SHELLEXECUTEINFOW *))
{
    HRESULT apartment=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
    if(FAILED(apartment) && apartment!=RPC_E_CHANGED_MODE)return (DWORD)apartment;
    /* Only a rejected legacy image with a private console reaches this path.
     * Validation has not launched it. Shell owns the one actual open request
     * and its native error UI; never add another application error dialog. */
    SHELLEXECUTEINFOW info={.cbSize=sizeof(info),
        .fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOASYNC,
        .lpFile=image,.lpParameters=arguments,.nShow=SW_SHOWNORMAL};
    DWORD error=ERROR_SUCCESS;
    if(!execute(&info)){error=GetLastError();if(!error)error=ERROR_GEN_FAILURE;}
    if(info.hProcess)CloseHandle(info.hProcess);
    if(SUCCEEDED(apartment))CoUninitialize();
    return error;
}
