#include <windows.h>
#include <stdio.h>
typedef void *(__cdecl *begin_fn)(BOOL);
typedef BOOL (__cdecl *end_fn)(void *,void *);
typedef BOOL (__cdecl *attached_fn)(HWND);
typedef struct result {DWORD installed,unsupported,failure;} result;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NESTED CREATION FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2);HMODULE dll=LoadLibraryW(argv[1]);CHECK(dll);
    begin_fn begin=(begin_fn)(void *)GetProcAddress(dll,"Ntw16sCreationBegin");
    end_fn end=(end_fn)(void *)GetProcAddress(dll,"Ntw16sCreationEnd");
    attached_fn attached=(attached_fn)(void *)GetProcAddress(dll,"Ntw16sWindowAttached");
    CHECK(begin && end && attached);
    void *outer=begin(TRUE);CHECK(outer);
    HWND first=CreateWindowW(L"STATIC",L"outer",WS_OVERLAPPEDWINDOW,0,0,180,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(first && attached(first));
    void *inner=begin(TRUE);CHECK(inner);
    HWND second=CreateWindowW(L"STATIC",L"inner",WS_OVERLAPPEDWINDOW,0,0,180,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(second && attached(second));
    result o={0},i={0};SetLastError(0);CHECK(!end(outer,&o) && GetLastError()==ERROR_INVALID_STATE);
    CHECK(end(inner,&i) && !i.failure && i.installed==1 && attached(second));
    HWND third=CreateWindowW(L"STATIC",L"outer again",WS_OVERLAPPEDWINDOW,0,0,180,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(third && attached(third));
    CHECK(end(outer,&o) && !o.failure && o.installed==2 && attached(first) && attached(third));
    CHECK(DestroyWindow(third) && DestroyWindow(second) && DestroyWindow(first));FreeLibrary(dll);
    printf("NESTED CREATION PASS bits=%u outer=2 inner=1 invalid_order=1\n",(unsigned)(sizeof(void *)*8));return 0;
}
