#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include "run16s-exe/explorer.h"

typedef BOOL (__cdecl *window_fn)(HWND);
#define CHECK(x) do { if(!(x)) { \
    fprintf(stderr,"explorer selection failure at %d: %lu\n",__LINE__,GetLastError()); \
    return 1; \
} } while(0)

static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_DESTROY)PostQuitMessage(0);
    return DefWindowProcW(window,message,wp,lp);
}

static HWND make_folder(HINSTANCE instance)
{
    return CreateWindowW(L"CabinetWClass",L"fixture",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        0,0,300,200,NULL,NULL,instance,NULL);
}

typedef struct owned_window {DWORD pid;HWND window;} owned_window;
static BOOL CALLBACK find_owned(HWND window,LPARAM parameter)
{
    owned_window *owned=(owned_window *)parameter;DWORD pid=0;
    WCHAR name[64];GetWindowThreadProcessId(window,&pid);
    if(pid==owned->pid && GetClassNameW(window,name,ARRAYSIZE(name)) && !wcscmp(name,L"CabinetWClass")){
        owned->window=window;return FALSE;
    }
    return TRUE;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"--other-module")){
        HMODULE other=LoadLibraryW(argv[2]);CHECK(other);
        WNDCLASSW child={.lpfnWndProc=window_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"CabinetWClass"};
        CHECK(RegisterClassW(&child));CHECK(make_folder(child.hInstance));
        MSG message;while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
        FreeLibrary(other);return 0;
    }
    CHECK(argc==1);
    WCHAR desktop_name[64],image[32768],directory[32768],path[32768];
    WCHAR system_explorer[MAX_PATH];
    UINT system_length=GetWindowsDirectoryW(system_explorer,ARRAYSIZE(system_explorer));
    CHECK(system_length && system_length+14<ARRAYSIZE(system_explorer));
    wcscpy(system_explorer+system_length,L"\\explorer.exe");
    CHECK(run16s_explorer_default(system_explorer,L" \t"));
    CHECK(!run16s_explorer_default(system_explorer,L"/n"));
    CHECK(!run16s_explorer_default(L"C:\\other\\explorer.exe",L""));
    CHECK(run16s_terminal_image(L"C:\\Apps\\WindowsTerminal.exe"));
    CHECK(!run16s_terminal_image(L"C:\\Apps\\not-terminal.exe"));
    CHECK(!run16s_terminal_image(NULL));
    swprintf(desktop_name,ARRAYSIZE(desktop_name),L"S3Selection-%lu",GetCurrentProcessId());
    HDESK original=GetThreadDesktop(GetCurrentThreadId());CHECK(original);
    HDESK desktop=CreateDesktopW(desktop_name,NULL,NULL,0,GENERIC_ALL,NULL);
    CHECK(desktop && SetThreadDesktop(desktop));
    DWORD length=GetModuleFileNameW(NULL,image,ARRAYSIZE(image));
    CHECK(length && length<ARRAYSIZE(image));
    wcscpy(directory,image);WCHAR *slash=wcsrchr(directory,L'\\');CHECK(slash);slash[1]=0;
    CHECK(wcslen(directory)+14<ARRAYSIZE(path));wcscpy(path,directory);wcscat(path,L"ntw16s64.dll");
    HMODULE module=LoadLibraryW(path);CHECK(module);
    window_fn attached=(window_fn)(uintptr_t)GetProcAddress(module,"Ntw16sWindowAttached");
    window_fn remove=(window_fn)(uintptr_t)GetProcAddress(module,"Ntw16sWindowRemove");
    CHECK(attached && remove);
    WNDCLASSW wc={.lpfnWndProc=window_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"CabinetWClass"};
    CHECK(RegisterClassW(&wc));
    HWND old=make_folder(wc.hInstance);CHECK(old);
    ShowWindow(old,SW_HIDE);
    run16s_explorer_snapshot *snapshot=run16s_explorer_snapshot_create();CHECK(snapshot);
    ShowWindow(old,SW_SHOW);
    HWND fresh=make_folder(wc.hInstance);CHECK(fresh);
    CHECK(run16s_explorer_attach_new(snapshot,image,directory)==ERROR_SUCCESS);
    CHECK(attached(fresh) && !attached(old));
    CHECK(remove(fresh));DestroyWindow(fresh);run16s_explorer_snapshot_free(snapshot);
    {
        WNDCLASSW terminal={.lpfnWndProc=window_proc,.hInstance=wc.hInstance,.lpszClassName=L"CASCADIA_HOSTING_WINDOW_CLASS"};
        CHECK(RegisterClassW(&terminal));
        HWND previous=CreateWindowW(terminal.lpszClassName,L"previous",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            0,0,300,200,NULL,NULL,terminal.hInstance,NULL);CHECK(previous);
        snapshot=run16s_terminal_snapshot_create();CHECK(snapshot);
        HWND created=CreateWindowW(terminal.lpszClassName,L"created",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            0,0,300,200,NULL,NULL,terminal.hInstance,NULL);CHECK(created);
        CHECK(run16s_explorer_attach_new(snapshot,image,directory)==ERROR_SUCCESS);
        CHECK(attached(created) && !attached(previous));
        CHECK(remove(created));DestroyWindow(created);DestroyWindow(previous);
        run16s_explorer_snapshot_free(snapshot);CHECK(UnregisterClassW(terminal.lpszClassName,terminal.hInstance));
    }
    snapshot=run16s_explorer_snapshot_create();CHECK(snapshot);
    HWND one=make_folder(wc.hInstance),two=make_folder(wc.hInstance);CHECK(one && two);
    CHECK(run16s_explorer_attach_new(snapshot,image,directory)==ERROR_MORE_DATA);
    CHECK(!attached(old) && !attached(one) && !attached(two));
    DestroyWindow(one);DestroyWindow(two);run16s_explorer_snapshot_free(snapshot);
    WCHAR scratch[32768],other[32768],command[32768];
    DWORD current=GetCurrentDirectoryW(ARRAYSIZE(scratch),scratch);
    CHECK(current && current+24<ARRAYSIZE(scratch));
    wcscat(scratch,L"\\other-module");CHECK(CreateDirectoryW(scratch,NULL));
    wcscpy(other,scratch);wcscat(other,L"\\ntw16s64.dll");CHECK(CopyFileW(path,other,TRUE));
    snapshot=run16s_explorer_snapshot_create();CHECK(snapshot);
    CHECK(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --other-module \"%ls\"",image,other)>0);
    STARTUPINFOW startup={.cb=sizeof(startup),.lpDesktop=desktop_name};PROCESS_INFORMATION child={0};
    CHECK(CreateProcessW(image,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child));
    CHECK(run16s_explorer_attach_new(snapshot,image,directory)==ERROR_REVISION_MISMATCH);
    owned_window target={.pid=child.dwProcessId};EnumWindows(find_owned,(LPARAM)&target);CHECK(target.window);
    CHECK(PostMessageW(target.window,WM_CLOSE,0,0));CHECK(WaitForSingleObject(child.hProcess,3000)==WAIT_OBJECT_0);
    CloseHandle(child.hThread);CloseHandle(child.hProcess);run16s_explorer_snapshot_free(snapshot);
    CHECK(DeleteFileW(other));CHECK(RemoveDirectoryW(scratch));

    /* A loaded module keeps its original loader name after the mapped file is
     * renamed and replaced. The replacement can contain identical code. */
    CHECK(CreateDirectoryW(scratch,NULL));CHECK(CopyFileW(path,other,TRUE));
    WCHAR moved[32768],package[32768];wcscpy(moved,scratch);wcscat(moved,L"\\mapped-old.dll");
    wcscpy(package,scratch);wcscat(package,L"\\");
    snapshot=run16s_explorer_snapshot_create();CHECK(snapshot);
    CHECK(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --other-module \"%ls\"",image,other)>0);
    child=(PROCESS_INFORMATION){0};CHECK(CreateProcessW(image,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child));
    target=(owned_window){.pid=child.dwProcessId};
    for(unsigned i=0;i<60 && !target.window;i++){EnumWindows(find_owned,(LPARAM)&target);if(!target.window)Sleep(50);}
    CHECK(target.window);CHECK(MoveFileW(other,moved));CHECK(CopyFileW(path,other,TRUE));
    DWORD changed=run16s_explorer_attach_new(snapshot,image,package);
    printf("renamed_mapping_result=%lu\n",changed);
    CHECK(PostMessageW(target.window,WM_CLOSE,0,0));CHECK(WaitForSingleObject(child.hProcess,3000)==WAIT_OBJECT_0);
    CloseHandle(child.hThread);CloseHandle(child.hProcess);run16s_explorer_snapshot_free(snapshot);
    CHECK(DeleteFileW(other));CHECK(DeleteFileW(moved));CHECK(RemoveDirectoryW(scratch));
    CHECK(changed==ERROR_REVISION_MISMATCH);
    DestroyWindow(old);FreeLibrary(module);
    CHECK(UnregisterClassW(wc.lpszClassName,wc.hInstance));
    CHECK(SetThreadDesktop(original));CHECK(CloseDesktop(desktop));
    puts("explorer-selection PASS");return 0;
}
