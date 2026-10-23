/* Native held workload: reports readiness, releases only on a test-owned event. */
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"WAIT TARGET FAIL %d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int _dowildcard=0;
static int target(int argc,WCHAR **argv)
{
    CHECK(argc==4);
    CHECK(GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll"));
    DWORD self=GetCurrentProcessId(),parent=0,ids[64],count=GetConsoleProcessList(ids,ARRAYSIZE(ids));
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);CHECK(snapshot!=INVALID_HANDLE_VALUE);
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};CHECK(Process32FirstW(snapshot,&entry));
    do{if(entry.th32ProcessID==self){parent=entry.th32ParentProcessID;break;}}while(Process32NextW(snapshot,&entry));
    CloseHandle(snapshot);CHECK(parent);
    FILE *file=NULL;CHECK(!_wfopen_s(&file,argv[1],L"wt"));
    fprintf(file,"%lu %lu %llu %lu\n",self,parent,(unsigned long long)(uintptr_t)GetConsoleWindow(),count);
    for(DWORD i=0;i<count && i<ARRAYSIZE(ids);++i)fprintf(file,"%lu\n",ids[i]);
    CHECK(!fclose(file));
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,argv[2]),release=OpenEventW(SYNCHRONIZE,FALSE,argv[3]);
    CHECK(ready && release && SetEvent(ready));
    DWORD result=WaitForSingleObject(release,20000);CloseHandle(ready);CloseHandle(release);
    CHECK(result==WAIT_OBJECT_0);return 42;
}
#ifdef WAIT_GUI
int WINAPI wWinMain(HINSTANCE a,HINSTANCE b,PWSTR c,int d)
{(void)a;(void)b;(void)c;(void)d;return target(__argc,__wargv);}
#else
int wmain(int argc,WCHAR **argv){return target(argc,argv);}
#endif
