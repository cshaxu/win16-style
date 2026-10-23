#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
static WCHAR marker[MAX_PATH];
static void fail(int line,const char *text)
{
    FILE *file=NULL;DWORD error=GetLastError();fprintf(stderr,"TREE FAIL %d error=%lu: %s\n",line,error,text);
    if(marker[0] && !_wfopen_s(&file,marker,L"at")){fprintf(file,"FAIL %d error=%lu %s\n",line,error,text);fclose(file);}ExitProcess(1);
}
#define CHECK(x) do{if(!(x))fail(__LINE__,#x);}while(0)
static void report(const char *phase,DWORD value)
{FILE *file=NULL;CHECK(!_wfopen_s(&file,marker,L"at"));fprintf(file,"%s %lu\n",phase,value);CHECK(!fclose(file));}
static DWORD wait_gui(HANDLE process)
{
    ULONGLONG deadline=GetTickCount64()+60000;
    for(;;){ULONGLONG now=GetTickCount64();MSG message;if(now>=deadline)return WAIT_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjectsEx(1,&process,(DWORD)(deadline-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result!=WAIT_OBJECT_0+1)return result;
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){CHECK(message.message!=WM_QUIT);TranslateMessage(&message);DispatchMessageW(&message);}}
}
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
static void window_check(void)
{
    WNDCLASSW cls={0};HWND root,button;LOGFONTW font;
    typedef BOOL (__cdecl *attached_fn)(HWND);
    HMODULE dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    attached_fn attached=(attached_fn)(void *)GetProcAddress(dll,"Ntw16sWindowAttached");CHECK(attached);
    cls.lpfnWndProc=owner;cls.lpszClassName=L"PropagatedNativeWindow";cls.hInstance=GetModuleHandleW(NULL);CHECK(RegisterClassW(&cls));
    root=CreateWindowW(cls.lpszClassName,L"Inherited native window",WS_OVERLAPPEDWINDOW,0,0,360,240,NULL,NULL,cls.hInstance,NULL);CHECK(root && attached(root));
    button=CreateWindowW(L"BUTTON",L"Native child",WS_CHILD|BS_AUTOCHECKBOX,10,10,150,30,root,NULL,cls.hInstance,NULL);CHECK(button && attached(button));
    HFONT value=(HFONT)SendMessageW(button,WM_GETFONT,0,0);CHECK(value && GetObjectW(value,sizeof(font),&font)==sizeof(font));
    CHECK(!wcsncmp(font.lfFaceName,L"Win16 System20 ",15));
    SendMessageW(button,BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(button,BM_GETCHECK,0,0)==BST_CHECKED);
    CHECK(DestroyWindow(root));report("window-attached",GetCurrentProcessId());
}
int wmain(int argc,WCHAR **argv)
{
    WCHAR directory[MAX_PATH];int depth,ansi,hold;CHECK(argc==7);
    depth=_wtoi(argv[1]);ansi=_wtoi(argv[4]);hold=_wtoi(argv[5]);CHECK(depth>=0 && depth<=2 && (ansi==0 || ansi==1) && (hold==0 || hold==1));
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,MAX_PATH));
    CHECK(swprintf_s(marker,MAX_PATH,L"%ls\\tree-%ls-g%d-b%u.txt",directory,argv[6],depth,(unsigned)(sizeof(void *)*8))>0);
    report("entry",GetCurrentProcessId());CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));window_check();
    if(depth){
        WCHAR command[32768];STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};DWORD code;
        DWORD flags=hold?CREATE_SUSPENDED:0;
        CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" %d \"%ls\" \"%ls\" %d %d \"%ls\"",argv[2],depth-1,argv[3],argv[3],!ansi,hold,argv[6])>0);
        if(ansi){char application_a[4096],command_a[32768];STARTUPINFOA a={.cb=sizeof(a)};
            CHECK(WideCharToMultiByte(CP_ACP,0,argv[2],-1,application_a,sizeof(application_a),NULL,NULL));
            CHECK(WideCharToMultiByte(CP_ACP,0,command,-1,command_a,sizeof(command_a),NULL,NULL));
            CHECK(CreateProcessA(application_a,command_a,NULL,NULL,FALSE,flags,NULL,NULL,&a,&child));
        }else CHECK(CreateProcessW(argv[2],command,NULL,NULL,FALSE,flags,NULL,NULL,&startup,&child));
        report("child",child.dwProcessId);
        if(hold){
            CHECK(SuspendThread(child.hThread)==1 && ResumeThread(child.hThread)==2);
            CHECK(WaitForSingleObject(child.hProcess,0)==WAIT_TIMEOUT);
            CHECK(ResumeThread(child.hThread)==1);
        }
        DWORD wait=wait_gui(child.hProcess);if(wait!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
        CHECK(wait==WAIT_OBJECT_0 && GetExitCodeProcess(child.hProcess,&code));CloseHandle(child.hThread);CloseHandle(child.hProcess);CHECK(!code);
    }
    {WCHAR missing[MAX_PATH],line[8]=L"";STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
        CHECK(swprintf_s(missing,MAX_PATH,L"%ls\\missing-win16style-app.exe",directory)>0);
        SetLastError(0);CHECK(!CreateProcessW(missing,line,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child) && GetLastError()==ERROR_FILE_NOT_FOUND);}
    report("complete",GetCurrentProcessId());return 0;
}
