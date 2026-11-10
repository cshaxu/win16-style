#include <windows.h>
#include <stdio.h>

int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 1;
    SetLastError(ERROR_SUCCESS);
    HMODULE normal=LoadLibraryW(argv[1]);DWORD first=GetLastError();
    if(normal)FreeLibrary(normal);
    HMODULE scoped=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    DWORD second=GetLastError();
    printf("SHARED LOAD native=%d error=%lu scoped=%d error=%lu\n",normal!=NULL,first,scoped!=NULL,second);
    if(scoped)FreeLibrary(scoped);
    return !normal && first==ERROR_MOD_NOT_FOUND && scoped?0:2;
}
