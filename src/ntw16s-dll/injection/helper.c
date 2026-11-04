/* Product policy for the generic opposite-width helper. */
#include "staging.h"
#include <wchar.h>
static DWORD apply(HANDLE child,const void *packet,DWORD bytes,PCWSTR dll,void *unused)
{
    typedef struct helper_work {ntw16s_context context;WCHAR expected[NTW16S_PATH_CAP];} helper_work;
    WORD machine;DWORD error;helper_work *work;(void)unused;
    error=ntw16s_process_machine(child,&machine);if(error)return error;
    if(machine!=(sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386))return ERROR_BAD_EXE_FORMAT;
    work=HeapAlloc(GetProcessHeap(),0,sizeof(*work));if(!work)return ERROR_NOT_ENOUGH_MEMORY;
    error=ntw16s_packet_parse(packet,bytes,machine,&work->context);
    if(!error){
        SIZE_T chars=wcslen(work->context.directory);
        if(swprintf_s(work->expected,NTW16S_PATH_CAP,L"%ls%lsntw16s%u.dll",work->context.directory,
            work->context.directory[chars-1]==L'\\'?L"":L"\\",(unsigned)(sizeof(void *)*8))<0)error=ERROR_FILENAME_EXCED_RANGE;
        else if(_wcsicmp(work->expected,dll))error=ERROR_INVALID_NAME;
        else error=ntw16s_install(child,&work->context,CreateProcessW);
    }
    HeapFree(GetProcessHeap(),0,work);return error;
}
void ntw16s_run_helper(void)
{ExitProcess(process_hook_run_helper(apply,NULL));}
