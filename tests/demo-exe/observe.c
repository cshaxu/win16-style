#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <string.h>
#include "../ntw16s-dll/isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DEMO OBSERVER FAIL %d error=%lu: %s\n",__LINE__,GetLastError(),#x);ExitProcess(1);}}while(0)
static HANDLE tree,ready;
static HWND observed;
static void CALLBACK shown(HWINEVENTHOOK hook,DWORD event,HWND window,LONG object,LONG child,DWORD thread,DWORD time)
{
    WCHAR name[64];DWORD pid;BOOL owned=FALSE;(void)hook;(void)event;(void)thread;(void)time;
    if(object!=OBJID_WINDOW || child || !GetClassNameW(window,name,ARRAYSIZE(name)) || wcscmp(name,L"Win16StyleDemo"))return;
    GetWindowThreadProcessId(window,&pid);HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
    if(process){IsProcessInJob(process,tree,&owned);CloseHandle(process);}
    if(owned){observed=window;SetEvent(ready);}
}
static DWORD pump_wait(HANDLE object,DWORD timeout)
{
    ULONGLONG deadline=GetTickCount64()+timeout;
    for(;;){MSG message;ULONGLONG now=GetTickCount64();if(now>=deadline)return WAIT_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjectsEx(1,&object,(DWORD)(deadline-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result!=WAIT_OBJECT_0+1)return result;
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
}
static void text(HWND window,int id,PCWSTR expected)
{WCHAR actual[300]={0};DWORD_PTR result=0;
 BOOL sent=(BOOL)SendMessageTimeoutW(id?GetDlgItem(window,id):window,WM_GETTEXT,ARRAYSIZE(actual),(LPARAM)actual,SMTO_ABORTIFHUNG,5000,&result);
 if(!sent || !result || wcscmp(actual,expected))fwprintf(stderr,L"TEXT id=%d actual=[%ls] expected=[%ls] sent=%d result=%llu\n",id,actual,expected,sent,(unsigned long long)result);
 CHECK(sent && result && !wcscmp(actual,expected));}
static void modules(DWORD pid,BOOL styled,PCWSTR package,PCWSTR image)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);CHECK(snapshot!=INVALID_HANDLE_VALUE);
    MODULEENTRY32W entry={.dwSize=sizeof(entry)};BOOL found=FALSE,main=FALSE;WCHAR wanted[MAX_PATH];
    CHECK(swprintf_s(wanted,MAX_PATH,L"%ls\\ntw16s64.dll",package)>0 && Module32FirstW(snapshot,&entry));
    do{
        if(!_wcsicmp(entry.szModule,L"ntw16s32.dll"))CHECK(FALSE);
        if(!_wcsicmp(entry.szModule,L"ntw16s64.dll")){CHECK(styled && !_wcsicmp(entry.szExePath,wanted));found=TRUE;}
        if(!_wcsicmp(entry.szExePath,image))main=TRUE;
    }while(Module32NextW(snapshot,&entry));
    CHECK(found==styled && main);CloseHandle(snapshot);
}
typedef struct pixels {uint64_t hash;unsigned caption;} pixels;
static pixels capture(HWND window,PCWSTR file)
{
    RECT r;CHECK(GetWindowRect(window,&r));int width=r.right-r.left,height=r.bottom-r.top;CHECK(width>0 && height>0);
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    HDC screen=GetDC(window),dc=CreateCompatibleDC(screen);void *data=NULL;
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&data,NULL,0);CHECK(screen && dc && bitmap && data);
    HGDIOBJ previous=SelectObject(dc,bitmap);memset(data,0xff,(size_t)width*height*4);
    CHECK(PrintWindow(window,dc,0));
    pixels value={14695981039346656037ull,0};BYTE *bytes=data;
    for(size_t i=0;i<(size_t)width*height*4;++i)value.hash=(value.hash^bytes[i])*1099511628211ull;
    for(size_t i=0;i<(size_t)width*height;++i)if(bytes[i*4]==240 && bytes[i*4+1]==200 && bytes[i*4+2]==164)++value.caption;
    BITMAPFILEHEADER header={0};header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);
    DWORD size=(DWORD)((size_t)width*height*4),written;header.bfSize=header.bfOffBits+size;
    HANDLE output=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(output!=INVALID_HANDLE_VALUE);
    CHECK(WriteFile(output,&header,sizeof(header),&written,NULL) && written==sizeof(header));
    CHECK(WriteFile(output,&info.bmiHeader,sizeof(info.bmiHeader),&written,NULL) && written==sizeof(info.bmiHeader));
    CHECK(WriteFile(output,data,size,&written,NULL) && written==size && CloseHandle(output));
    SelectObject(dc,previous);CHECK(DeleteObject(bitmap) && DeleteDC(dc));ReleaseDC(window,screen);return value;
}
static int matrix(int argc,WCHAR **argv)
{
    WCHAR scratch[MAX_PATH],package[MAX_PATH],launcher[MAX_PATH],image[MAX_PATH],path[MAX_PATH],command[32768];
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    CHECK(argc==5 && GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    CHECK(swprintf_s(package,MAX_PATH,L"%ls\\demo package",scratch)>0 && CreateDirectoryW(package,NULL));
    CHECK(swprintf_s(launcher,MAX_PATH,L"%ls\\run16s.exe",package)>0 && CopyFileW(argv[1],launcher,FALSE));
    CHECK(swprintf_s(path,MAX_PATH,L"%ls\\ntw16s64.dll",package)>0 && CopyFileW(argv[2],path,FALSE));
    CHECK(swprintf_s(path,MAX_PATH,L"%ls\\ntw16s32.dll",package)>0 && CopyFileW(argv[3],path,FALSE));
    CHECK(swprintf_s(image,MAX_PATH,L"%ls\\demo.exe",package)>0 && CopyFileW(argv[4],image,FALSE));
    ready=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(ready);
    HWINEVENTHOOK events=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,shown,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);CHECK(events);
    pixels screenshots[2];int advances[2];
    for(unsigned styled=0;styled<2;++styled){
        tree=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(tree && SetInformationJobObject(tree,JobObjectExtendedLimitInformation,&limits,sizeof(limits)) && ResetEvent(ready));observed=NULL;
        if(styled)CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\"",launcher,image)>0);
        else CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\"",image)>0);
        STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};
        startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=SW_SHOWNORMAL;
        CHECK(CreateProcessW(styled?launcher:image,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&process));
        if(!AssignProcessToJobObject(tree,process.hProcess)){TerminateProcess(process.hProcess,1);CHECK(FALSE);}
        CHECK(ResumeThread(process.hThread)==1 && pump_wait(ready,30000)==WAIT_OBJECT_0 && observed);
        HWND w=observed;DWORD pid;GetWindowThreadProcessId(w,&pid);modules(pid,styled,package,image);text(w,0,L"Win16 Style Demo");
        static const WCHAR *classes[]={L"BUTTON",L"EDIT",L"BUTTON",L"BUTTON",L"BUTTON",L"LISTBOX",L"COMBOBOX",L"SCROLLBAR",L"STATIC",L"STATIC",L"BUTTON"};
        for(int i=0;i<11;++i){WCHAR actual[40];HWND control=GetDlgItem(w,100+i);CHECK(control && GetClassNameW(control,actual,ARRAYSIZE(actual)) && !_wcsicmp(actual,classes[i]));}
        text(w,101,L"Native text");text(w,108,L"Ready");CHECK(IsDlgButtonChecked(w,102)==BST_CHECKED && IsDlgButtonChecked(w,103)==BST_CHECKED);
        CHECK(SendDlgItemMessageW(w,105,LB_GETCOUNT,0,0)==6 && SendDlgItemMessageW(w,106,CB_GETCOUNT,0,0)==6);
        CHECK(SendDlgItemMessageW(w,105,LB_GETCURSEL,0,0)==1 && SendDlgItemMessageW(w,106,CB_GETCURSEL,0,0)==1);
        CHECK(GetMenuItemCount(GetMenu(w))==2 && GetMenuItemCount(GetSubMenu(GetMenu(w),0))==3);
        SendMessageW(w,WM_NCACTIVATE,TRUE,0);UpdateWindow(w);
        CHECK(swprintf_s(path,MAX_PATH,L"%ls\\demo-%ls.bmp",scratch,styled?L"injected":L"native")>0);screenshots[styled]=capture(w,path);
        LRESULT first=SendDlgItemMessageW(w,101,EM_POSFROMCHAR,0,0),last=SendDlgItemMessageW(w,101,EM_POSFROMCHAR,5,0);
        CHECK(first!=-1 && last!=-1);advances[styled]=(short)LOWORD(last)-(short)LOWORD(first);
        CHECK(SendMessageW(GetDlgItem(w,101),WM_SETTEXT,0,(LPARAM)L"User text"));text(w,101,L"User text");SendDlgItemMessageW(w,100,BM_CLICK,0,0);text(w,108,L"Applied: User text");
        SendDlgItemMessageW(w,102,BM_CLICK,0,0);CHECK(IsDlgButtonChecked(w,102)==BST_UNCHECKED);text(w,108,L"Disabled");
        SendDlgItemMessageW(w,104,BM_CLICK,0,0);CHECK(IsDlgButtonChecked(w,104)==BST_CHECKED && IsDlgButtonChecked(w,103)==BST_UNCHECKED);text(w,108,L"Option B");
        SendDlgItemMessageW(w,105,WM_KEYDOWN,VK_DOWN,0);CHECK(SendDlgItemMessageW(w,105,LB_GETCURSEL,0,0)==2);text(w,108,L"List selection: 3");
        SendDlgItemMessageW(w,106,WM_KEYDOWN,VK_DOWN,0);CHECK(SendDlgItemMessageW(w,106,CB_GETCURSEL,0,0)==2);text(w,108,L"Combo selection: 3");
        SendMessageW(w,WM_HSCROLL,SB_LINERIGHT,(LPARAM)GetDlgItem(w,107));CHECK(GetScrollPos(GetDlgItem(w,107),SB_CTL)==31);text(w,108,L"Scroll position: 31");
        SendMessageW(w,WM_COMMAND,1001,0);text(w,108,L"Ready");text(w,101,L"Native text");CHECK(IsDlgButtonChecked(w,102)==BST_CHECKED && GetScrollPos(GetDlgItem(w,107),SB_CTL)==30);
        SendMessageW(w,WM_SYSCOMMAND,SC_MINIMIZE,0);CHECK(IsIconic(w));SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);CHECK(!IsIconic(w));
        SendMessageW(w,WM_SYSCOMMAND,SC_MAXIMIZE,0);CHECK(IsZoomed(w));SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);CHECK(!IsZoomed(w));
        SendMessageW(w,WM_COMMAND,1002,0);CHECK(pump_wait(process.hProcess,10000)==WAIT_OBJECT_0);DWORD code;CHECK(GetExitCodeProcess(process.hProcess,&code) && !code);
        CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(tree);tree=NULL;
        printf("DEMO OBSERVE PASS styled=%u same image, six classes, data/notifications/menu/window commands, text advance=%d caption pixels=%u\n",styled,advances[styled],screenshots[styled].caption);
    }
    CHECK(screenshots[0].hash!=screenshots[1].hash && screenshots[1].caption>screenshots[0].caption+500 && advances[1]>advances[0]);
    CHECK(UnhookWinEvent(events) && CloseHandle(ready));puts("DEMO PAIR PASS external module/appearance/business observations on private desktop; physical input not claimed");return 0;
}
int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,matrix);}
