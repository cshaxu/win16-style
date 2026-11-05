#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "lib-win32/file/identity.h"
#include "lib-win32/file/native_path.h"
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FILE_TEST_FAIL line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto cleanup;} } while(0)
int main(void)
{
    WCHAR directory[MAX_PATH],first[MAX_PATH]={0},second[MAX_PATH]={0},path[32767];
    BOOL same=FALSE;DWORD before=0,after=0;int result=0;
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,MAX_PATH)>0);
    CHECK(GetTempFileNameW(directory,L"w16",0,first));
    CHECK(GetTempFileNameW(directory,L"w16",0,second));
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    CHECK(win32_file_same(first,first,&same)==ERROR_SUCCESS && same);
    CHECK(win32_file_same(first,second,&same)==ERROR_SUCCESS && !same);
    CHECK(win32_file_same(NULL,first,&same)==ERROR_INVALID_PARAMETER && !same);
    CHECK(win32_file_same(first,second,NULL)==ERROR_INVALID_PARAMETER);
    CHECK(common_native_application_path(first,path,ARRAYSIZE(path))==ERROR_SUCCESS);
    CHECK(win32_file_same(first,path,&same)==ERROR_SUCCESS && same);
    CHECK(common_native_application_path(first,path,2)==ERROR_INSUFFICIENT_BUFFER && !path[0]);
    CHECK(common_native_application_path(NULL,path,ARRAYSIZE(path))==ERROR_INVALID_PARAMETER && !path[0]);
    CHECK(common_native_application_path(L"",path,ARRAYSIZE(path))==ERROR_INVALID_PARAMETER && !path[0]);
    CHECK(common_native_application_path(L"?:\\invalid-image",path,ARRAYSIZE(path))!=ERROR_SUCCESS && !path[0]);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&after) && after==before);
cleanup:
    if(first[0])DeleteFileW(first);
    if(second[0])DeleteFileW(second);
    if(!result)puts("FILE_PRIMITIVES_PASS same=1 distinct=1 final_path=1 invalid=1 handles=1");
    return result;
}
