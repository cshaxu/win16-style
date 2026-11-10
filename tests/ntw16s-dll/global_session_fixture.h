#ifndef TEST_GLOBAL_SESSION_FIXTURE_H
#define TEST_GLOBAL_SESSION_FIXTURE_H

static int global_test_session(WCHAR *name,WCHAR *stop_name,HANDLE *map,ntw16s_global_session **session,DWORD cookie)
{
    WCHAR desktop[128];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 7;
    if(!ntw16s_global_name(name,NTW16S_GLOBAL_NAME_CAP) ||
       swprintf(stop_name,NTW16S_GLOBAL_NAME_CAP+16,L"%ls.stop",name)<0)return 8;
    *map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!*map || GetLastError()==ERROR_ALREADY_EXISTS)return 9;
    *session=(ntw16s_global_session *)MapViewOfFile(*map,FILE_MAP_ALL_ACCESS,0,0,sizeof(**session));if(!*session)return 10;
    ZeroMemory(*session,sizeof(**session));(*session)->magic=NTW16S_GLOBAL_MAGIC;(*session)->version=NTW16S_GLOBAL_VERSION;
    (*session)->owner_pid=GetCurrentProcessId();(*session)->cookie=cookie;(*session)->phase=NTW16S_GLOBAL_ACTIVE;
    if(!win32_process_global_birth(GetCurrentProcess(),&(*session)->owner_birth_low,&(*session)->owner_birth_high))return 11;
    return 0;
}
#endif
