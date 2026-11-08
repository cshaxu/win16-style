#ifndef UNICODE
#define UNICODE
#endif
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
int wmain(int argc, wchar_t **argv) {
    WCHAR self[32768], params[32768];
    if (!GetModuleFileNameW(NULL, self, 32768)) return 2;
    if (argc == 2 && !wcscmp(argv[1], L"--handle-child")) return 37;
    if (argc == 2 && !wcscmp(argv[1], L"--handle-control")) {
        SHELLEXECUTEINFOW request = {.cbSize=sizeof(request), .fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_NO_UI,
            .lpVerb=L"open", .lpFile=self, .lpParameters=L"--handle-child", .nShow=SW_HIDE};
        if(!ShellExecuteExW(&request) || !request.hProcess)return 18;
        DWORD wait=WaitForSingleObject(request.hProcess,5000),code=STILL_ACTIVE;
        GetExitCodeProcess(request.hProcess,&code);
        if(wait==WAIT_TIMEOUT){TerminateProcess(request.hProcess,ERROR_TIMEOUT);WaitForSingleObject(request.hProcess,2000);}
        CloseHandle(request.hProcess);
        wprintf(L"wait=%lu exit=%lu\n",wait,code);
        return wait==WAIT_OBJECT_0 && code==37?0:19;
    }
    if (argc == 2 && (!wcscmp(argv[1], L"--invalid") || !wcscmp(argv[1], L"--invalid-handle"))) {
        BOOL handle_case=!wcscmp(argv[1], L"--invalid-handle");
        SHELLEXECUTEINFOW invalid = {.cbSize=sizeof(invalid), .fMask=SEE_MASK_FLAG_NO_UI|(handle_case?SEE_MASK_NOCLOSEPROCESS:0),
            .lpVerb=L"runas", .lpFile=L"Z:\\win16style-nonexistent\\missing.exe", .nShow=SW_HIDE};
        SetLastError(0); BOOL result=ShellExecuteExW(&invalid); DWORD error=GetLastError();
        wprintf(L"result=%d error=%lu handle=%d\n",result,error,invalid.hProcess!=NULL);
        return result ? 17 : 0;
    }
    if (argc == 3 && !wcscmp(argv[1], L"--child")) {
        HMODULE dll = GetModuleHandleW(sizeof(void*) == 8 ? L"ntw16s64.dll" : L"ntw16s32.dll");
        WCHAR path[32768] = L""; HANDLE token = NULL; TOKEN_ELEVATION e = {0}; DWORD size = 0;
        if (dll) GetModuleFileNameW(dll, path, 32768);
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return 3;
        BOOL ok = GetTokenInformation(token, TokenElevation, &e, sizeof(e), &size); CloseHandle(token);
        WNDCLASSW cls = {.lpfnWndProc=DefWindowProcW, .hInstance=GetModuleHandleW(NULL), .lpszClassName=L"Win16SelfElevateProbe"};
        if (!RegisterClassW(&cls)) return 13;
        HWND window = CreateWindowExW(0, cls.lpszClassName, L"Self elevation probe", WS_OVERLAPPEDWINDOW,
            0, 0, 320, 200, NULL, NULL, cls.hInstance, NULL);
        if (!window) return 14;
        typedef BOOL (__cdecl *attached_fn)(HWND);
        FARPROC symbol = dll ? GetProcAddress(dll, "Ntw16sWindowAttached") : NULL;
        attached_fn attached = NULL; memcpy(&attached, &symbol, sizeof(attached));
        BOOL styled = attached && attached(window);
        FILE *f = _wfopen(argv[2], L"w, ccs=UTF-8"); if (!f) return 4;
        fwprintf(f, L"pid=%lu elevated=%lu dll=%ls attached=%d\n", GetCurrentProcessId(), e.TokenIsElevated, path, styled); fclose(f);
        DestroyWindow(window); UnregisterClassW(cls.lpszClassName, cls.hInstance);
        return ok && e.TokenIsElevated && dll && styled ? 0 : 5;
    }
    BOOL nohandle = argc == 3 && !wcscmp(argv[1], L"--nohandle");
    PCWSTR report = nohandle ? argv[2] : argc == 2 ? argv[1] : NULL;
    if (report) {
        WCHAR parent_report[32768], dll_path[32768] = L"";
        HMODULE dll = GetModuleHandleW(sizeof(void*) == 8 ? L"ntw16s64.dll" : L"ntw16s32.dll");
        if (dll) GetModuleFileNameW(dll, dll_path, 32768);
        WNDCLASSW cls = {.lpfnWndProc=DefWindowProcW, .hInstance=GetModuleHandleW(NULL), .lpszClassName=L"Win16SelfElevateParent"};
        if (!RegisterClassW(&cls)) return 15;
        HWND window = CreateWindowExW(0, cls.lpszClassName, L"Self elevation parent", WS_OVERLAPPEDWINDOW,
            0, 0, 320, 200, NULL, NULL, cls.hInstance, NULL);
        if (!window) return 16;
        typedef BOOL (__cdecl *attached_fn)(HWND);
        FARPROC symbol = dll ? GetProcAddress(dll, "Ntw16sWindowAttached") : NULL;
        attached_fn attached = NULL; memcpy(&attached, &symbol, sizeof(attached));
        BOOL styled = attached && attached(window);
        if (swprintf(parent_report, 32768, L"%ls.parent", report) < 0) return 10;
        FILE *f = _wfopen(parent_report, L"w, ccs=UTF-8"); if (!f) return 11;
        fwprintf(f, L"pid=%lu dll=%ls attached=%d\n", GetCurrentProcessId(), dll_path, styled); fclose(f);
        DestroyWindow(window); UnregisterClassW(cls.lpszClassName, cls.hInstance);
        if (!dll || !dll_path[0] || !styled) return 12;
    }
    if (!report || swprintf(params, 32768, L"--child \"%ls\"", report) < 0) return 6;
    SHELLEXECUTEINFOW info = {.cbSize=sizeof(info), .fMask=nohandle?SEE_MASK_FLAG_NO_UI:SEE_MASK_NOCLOSEPROCESS, .lpVerb=L"runas", .lpFile=self, .lpParameters=params, .nShow=SW_HIDE};
    if (!ShellExecuteExW(&info)) { fwprintf(stderr,L"shell_error=%lu\n",GetLastError()); return 7; }
    if (nohandle) return 0;
    if (!info.hProcess) return 8;
    DWORD wait=WaitForSingleObject(info.hProcess, 90000), code=STILL_ACTIVE;
    if (wait == WAIT_TIMEOUT) {
        TerminateProcess(info.hProcess, ERROR_TIMEOUT);
        WaitForSingleObject(info.hProcess, 5000);
    }
    GetExitCodeProcess(info.hProcess,&code); CloseHandle(info.hProcess);
    wprintf(L"wait=%lu child_exit=%lu\n",wait,code); return wait==WAIT_OBJECT_0 ? (int)code : 9;
}
