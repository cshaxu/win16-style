#include "legacy_shell.h"
#include <objbase.h>

BOOL run16s_legacy_image(PCWSTR image)
{
    HANDLE file=CreateFileW(image,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,
        NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE)return FALSE;
    IMAGE_DOS_HEADER dos;DWORD read,signature=0;BOOL legacy=FALSE;
    if(ReadFile(file,&dos,sizeof(dos),&read,NULL) && read==sizeof(dos) &&
            dos.e_magic==IMAGE_DOS_SIGNATURE){
        /* DOS has no new header; NE/LE and other non-PE MZ images remain
         * Windows Shell's decision. Never shell-launch an unstyled PE. */
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
DWORD run16s_legacy_shell(PCWSTR image,PCWSTR arguments,run16s_shell_execute execute)
{
    HRESULT apartment=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
    if(FAILED(apartment) && apartment!=RPC_E_CHANGED_MODE)return (DWORD)apartment;
    /* Match a direct Shell open: do not suppress Windows' own error/security
     * UI, invent a dialog, change the verb, or bypass zone checks. */
    SHELLEXECUTEINFOW info={.cbSize=sizeof(info),
        .fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOASYNC,
        .lpFile=image,.lpParameters=arguments,.nShow=SW_SHOWNORMAL};
    DWORD error=ERROR_SUCCESS;
    if(!execute(&info)){error=GetLastError();if(!error)error=ERROR_GEN_FAILURE;}
    if(info.hProcess)CloseHandle(info.hProcess);
    if(SUCCEEDED(apartment))CoUninitialize();
    return error;
}
