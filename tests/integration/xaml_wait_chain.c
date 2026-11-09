#include <windows.h>
#include <tlhelp32.h>
#include <wct.h>
#include <stdio.h>
#include <stdlib.h>

static BOOL CALLBACK report_window(HWND window,LPARAM parameter)
{
    DWORD pid=0,tid=GetWindowThreadProcessId(window,&pid);
    if (pid==(DWORD)(UINT_PTR)parameter && IsWindowVisible(window)) {
        WCHAR name[128]={0}; GetClassNameW(window,name,ARRAYSIZE(name));
        printf("visible_window_thread=%lu class=%ls\n",(unsigned long)tid,name);
    }
    return TRUE;
}

int wmain(int argc,wchar_t **argv)
{
    if (argc!=2) return 2;
    DWORD pid=wcstoul(argv[1],NULL,10); if (!pid) return 2;
    EnumWindows(report_window,(LPARAM)(UINT_PTR)pid);
    HWCT session=OpenThreadWaitChainSession(0,NULL);
    if (!session) { printf("session_error=%lu\n",(unsigned long)GetLastError()); return 3; }
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if (snapshot==INVALID_HANDLE_VALUE) { CloseThreadWaitChainSession(session); return 3; }
    THREADENTRY32 thread={.dwSize=sizeof(thread)}; unsigned int examined=0,success=0;
    WCHAR prefix[100]; DWORD prefix_length=GetEnvironmentVariableW(L"WIN16_XAML_THREAD_TAG",prefix,ARRAYSIZE(prefix));
    WCHAR callback_prefix[100]; DWORD callback_length=GetEnvironmentVariableW(L"WIN16_XAML_CALLBACK_TAG",callback_prefix,ARRAYSIZE(callback_prefix));
    if (Thread32First(snapshot,&thread)) do {
        if (thread.th32OwnerProcessID!=pid) continue;
        BOOL is_site=FALSE;
        if (prefix_length && prefix_length<ARRAYSIZE(prefix)) {
            WCHAR name[144]; swprintf(name,ARRAYSIZE(name),L"%ls-%lu",prefix,(unsigned long)thread.th32ThreadID);
            HANDLE tag=OpenEventW(SYNCHRONIZE,FALSE,name);
            if (tag) { is_site=TRUE; printf("site_thread=%lu\n",(unsigned long)thread.th32ThreadID); CloseHandle(tag); }
        }
        if (callback_length && callback_length<ARRAYSIZE(callback_prefix)) {
            WCHAR name[144]; swprintf(name,ARRAYSIZE(name),L"%ls-%lu",callback_prefix,(unsigned long)thread.th32ThreadID);
            HANDLE tag=OpenEventW(SYNCHRONIZE,FALSE,name);
            if (tag) { printf("callback_thread=%lu\n",(unsigned long)thread.th32ThreadID); CloseHandle(tag); }
        }
        WAITCHAIN_NODE_INFO nodes[16]={0}; DWORD count=ARRAYSIZE(nodes); BOOL cycle=FALSE;
        ++examined;
        if (GetThreadWaitChain(session,0,0,thread.th32ThreadID,&count,nodes,&cycle)) {
            ++success;
            if (count>1 || cycle || is_site) {
                printf("thread=%lu count=%lu cycle=%d",(unsigned long)thread.th32ThreadID,(unsigned long)count,cycle);
                for (DWORD i=0;i<count;i++) {
                    printf(" type=%d status=%d",nodes[i].ObjectType,nodes[i].ObjectStatus);
                    if (nodes[i].ObjectType==WctThreadType)
                        printf(" pid=%lu tid=%lu",(unsigned long)nodes[i].ThreadObject.ProcessId,(unsigned long)nodes[i].ThreadObject.ThreadId);
                }
                putchar('\n');
            }
        }
    } while (examined<64 && Thread32Next(snapshot,&thread));
    CloseHandle(snapshot); CloseThreadWaitChainSession(session);
    printf("examined=%u resolved=%u\n",examined,success);
    return success ? 0 : 77;
}
