#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

static DWORD main_thread;
static volatile LONG seen, old_layout, hook_seen;
static BOOL cancel_mode, custom_hook, legacy_size, default_flags, dynamic_mode;
static volatile LONG dynamic_set;
static WCHAR dynamic_path[MAX_PATH];
static HWND found_dialog;
static const WCHAR title[]=L"Win16S T20 file dialog integration";
static BOOL CALLBACK inspect(HWND window, LPARAM ignored)
{
    (void)ignored;
    WCHAR text[128];
    GetWindowTextW(window,text,128);
    if (lstrcmpW(text,title)) return TRUE;
    if (!IsWindowVisible(window)) return TRUE;
    HWND folders=GetDlgItem(window,1121),drives=GetDlgItem(window,1137);
    WCHAR cf[64]=L"",cd[64]=L"";
    if(folders)GetClassNameW(folders,cf,64);
    if(drives)GetClassNameW(drives,cd,64);
    printf("CONTROLS folders=%ls drives=%ls\n",cf,cd);
    InterlockedExchange(&old_layout,folders!=NULL && drives!=NULL && !lstrcmpiW(cf,L"ListBox") && !lstrcmpiW(cd,L"ComboBox"));
    InterlockedExchange(&seen,1);
    found_dialog=window;
    return FALSE;
}
static DWORD WINAPI observer(void *unused)
{
    (void)unused;
    for (unsigned i=0;i<160;++i) {
        if (!seen) EnumThreadWindows(main_thread,inspect,0);
        if (seen && found_dialog && IsWindow(found_dialog)) {
            if(dynamic_mode && !dynamic_set){WCHAR value[MAX_PATH]=L"";
                BOOL set=SetDlgItemTextW(found_dialog,1152,dynamic_path);
                GetDlgItemTextW(found_dialog,1152,value,ARRAYSIZE(value));
                InterlockedExchange(&dynamic_set,set && !lstrcmpiW(value,dynamic_path));}
            HWND ok=GetDlgItem(found_dialog,IDOK);
            if (i>20 && i<100 && i%10==0 && ok) PostMessageW(found_dialog,WM_COMMAND,MAKEWPARAM(cancel_mode?IDCANCEL:IDOK,BN_CLICKED),(LPARAM)(cancel_mode?GetDlgItem(found_dialog,IDCANCEL):ok));
            if (i==110) PostMessageW(found_dialog,WM_CLOSE,0,0);
        }
        if (seen && !IsWindow(found_dialog)) break;
        Sleep(50);
    }
    if (!seen || IsWindow(found_dialog)) PostThreadMessageW(main_thread,WM_QUIT,0,0);
    return 0;
}
static UINT_PTR CALLBACK app_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{
    (void)window;(void)wparam;(void)lparam;
    if(message==WM_INITDIALOG)InterlockedIncrement(&hook_seen);
    return 0;
}
static void *guard_copy(const void *value,SIZE_T length,void **base)
{
    SYSTEM_INFO info;GetSystemInfo(&info);
    if(length>info.dwPageSize)return NULL;
    BYTE *memory=VirtualAlloc(NULL,(SIZE_T)info.dwPageSize*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(!memory)return NULL;
    DWORD previous;
    if(!VirtualProtect(memory+info.dwPageSize,info.dwPageSize,PAGE_NOACCESS,&previous)){
        VirtualFree(memory,0,MEM_RELEASE);return NULL;
    }
    void *bounded=memory+info.dwPageSize-length;memcpy(bounded,value,length);*base=memory;return bounded;
}
int wmain(int argc, WCHAR **argv)
{
    if (argc<4 || argc>5) return 2;
    dynamic_mode=argc==5 && !lstrcmpW(argv[4],L"oldhookdynamic");
    BOOL oldhook_accept=dynamic_mode || (argc==5 && !lstrcmpW(argv[4],L"oldhookaccept"));
    custom_hook=oldhook_accept || (argc==5 && (!lstrcmpW(argv[4],L"hook") || !lstrcmpW(argv[4],L"oldhook")));
    cancel_mode=(custom_hook && !oldhook_accept) || (argc==5 && !lstrcmpW(argv[4],L"cancel"));
    legacy_size=argc==5 && !lstrcmpW(argv[4],L"v400");
    default_flags=oldhook_accept || (argc==5 && (!lstrcmpW(argv[4],L"default") || !lstrcmpW(argv[4],L"oldhook")));
    BOOL save=argv[1][0]==L's', ansi=argv[2][0]==L'a', expected_old=argv[3][0]==L'1';
    WCHAR directory[MAX_PATH], file_w[MAX_PATH], path_w[MAX_PATH];
    char directory_a[MAX_PATH],file_a[MAX_PATH];
    if (!GetTempPathW(MAX_PATH,directory) || !GetTempFileNameW(directory,L"w16",0,path_w)) return 3;
    if (save && !DeleteFileW(path_w)) return 4;
    lstrcpynW(file_w,path_w,MAX_PATH);
    if(dynamic_mode){lstrcpynW(dynamic_path,path_w,MAX_PATH);file_w[0]=0;}
    if (!WideCharToMultiByte(CP_ACP,0,directory,-1,directory_a,MAX_PATH,NULL,NULL) ||
        !WideCharToMultiByte(CP_ACP,0,path_w,-1,file_a,MAX_PATH,NULL,NULL)) return 5;
    HMODULE bridge=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    WCHAR module[MAX_PATH]=L"(not loaded)";
    if (bridge) GetModuleFileNameW(bridge,module,MAX_PATH);
    BYTE *entry=(BYTE *)(void *)GetProcAddress(GetModuleHandleW(L"comdlg32.dll"),"GetOpenFileNameW");
    printf("MODULE %ls entry=%p bytes=%02x %02x %02x %02x %02x\n",module,(void*)entry,entry[0],entry[1],entry[2],entry[3],entry[4]);
    main_thread=GetCurrentThreadId();
    printf("START pid=%lu tid=%lu\n",GetCurrentProcessId(),main_thread); fflush(stdout);
    HANDLE thread=CreateThread(NULL,0,observer,NULL,0,NULL);
    if (!thread) return 6;
    BOOL result; WORD offset=0;
    if (ansi) {
        OPENFILENAMEA o={0};o.lStructSize=sizeof(o);o.lpstrFile=file_a;o.nMaxFile=MAX_PATH;
        o.lpstrInitialDir=directory_a;o.lpstrTitle="Win16S T20 file dialog integration";
        o.lpstrFilter="All files\0*.*\0\0";o.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(save?0:OFN_FILEMUSTEXIST);
        if(custom_hook){o.Flags|=OFN_ENABLEHOOK;o.lpfnHook=app_hook;}
        if(legacy_size)o.lStructSize=OPENFILENAME_SIZE_VERSION_400A;
        if(default_flags)o.Flags&=~OFN_EXPLORER;
        void *base=NULL;OPENFILENAMEA *passed=&o;
        if(legacy_size){passed=guard_copy(&o,o.lStructSize,&base);if(!passed)return 7;}
        result=save?GetSaveFileNameA(passed):GetOpenFileNameA(passed);
        if(base){memcpy(&o,passed,OPENFILENAME_SIZE_VERSION_400A);VirtualFree(base,0,MEM_RELEASE);}
        offset=o.nFileOffset;
        MultiByteToWideChar(CP_ACP,0,file_a,-1,file_w,MAX_PATH);
    } else {
        OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrFile=file_w;o.nMaxFile=MAX_PATH;
        o.lpstrInitialDir=directory;o.lpstrTitle=title;o.lpstrFilter=L"All files\0*.*\0\0";
        o.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(save?0:OFN_FILEMUSTEXIST);
        if(custom_hook){o.Flags|=OFN_ENABLEHOOK;o.lpfnHook=app_hook;}
        if(legacy_size)o.lStructSize=OPENFILENAME_SIZE_VERSION_400W;
        if(default_flags)o.Flags&=~OFN_EXPLORER;
        void *base=NULL;OPENFILENAMEW *passed=&o;
        if(legacy_size){passed=guard_copy(&o,o.lStructSize,&base);if(!passed)return 7;}
        result=save?GetSaveFileNameW(passed):GetOpenFileNameW(passed);
        if(base){memcpy(&o,passed,OPENFILENAME_SIZE_VERSION_400W);VirtualFree(base,0,MEM_RELEASE);}
        offset=o.nFileOffset;
    }
    DWORD extended=CommDlgExtendedError();
    WaitForSingleObject(thread,6000);CloseHandle(thread);
    BOOL same=!lstrcmpiW(path_w,file_w);
    DeleteFileW(path_w);
    if(dynamic_mode)printf("FILE_DYNAMIC bits=%u set=%ld same=%d\n",(unsigned)(sizeof(void*)*8),dynamic_set,same);
    printf("FILE_DIALOG bits=%u save=%d ansi=%d observed=%ld old=%ld expected=%d ok=%d ext=%lu same=%d offset=%u cancel=%d custom_hook=%d hook_seen=%ld\n",
        (unsigned)(sizeof(void*)*8),save,ansi,seen,old_layout,expected_old,result,extended,same,offset,cancel_mode,custom_hook,hook_seen);
    return seen && old_layout==expected_old && !extended && (!custom_hook || hook_seen) && (!dynamic_mode || dynamic_set) &&
        (cancel_mode ? !result : (result && same && offset)) ? 0:1;
}
