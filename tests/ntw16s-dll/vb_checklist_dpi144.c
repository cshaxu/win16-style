/* The 144-DPI geometries are taken from real VB5 and VB6 lists. */
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"VB_CHECKLIST_144_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x); return 1; } } while (0)
static int (WINAPI *draw)(HDC,RECT,int,BOOL,BOOL);
static HWND list;
static BOOL expected,selected;
static int shift,observations,mismatch;
static LRESULT CALLBACK parent(HWND w,UINT msg,WPARAM a,LPARAM b)
{
    if(msg==WM_DRAWITEM && b){
        DRAWITEMSTRUCT *item=(void *)b;
        if(item->CtlType==ODT_LISTBOX && item->hwndItem==list && item->itemID==0){
            mismatch=draw(item->hDC,item->rcItem,shift,selected,expected);
            ++observations;return TRUE;
        }
    }
    return DefWindowProcW(w,msg,a,b);
}
static BOOL repaint(void)
{
    observations=0;mismatch=-1;
    if(!RedrawWindow(list,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW))return FALSE;
    GdiFlush();return observations>0 && mismatch==0;
}
static int child(const WCHAR *runtime,BOOL styled)
{
    HMODULE dll=LoadLibraryW(runtime);CHECK(dll);
    BOOL (WINAPI *reg)(void)=(void *)GetProcAddress(dll,"VbListRegister");
    BOOL (WINAPI *release)(void)=(void *)GetProcAddress(dll,"VbListRelease");
    draw=(void *)GetProcAddress(dll,"VbListDraw");CHECK(reg && release && draw && reg());
    BOOL vb5=GetModuleHandleW(L"MSVBVM50.DLL")==dll;
    HINSTANCE self=GetModuleHandleW(NULL);
    WNDCLASSW c={.hInstance=self,.lpfnWndProc=parent,.lpszClassName=L"VBList144Parent"};
    CHECK(RegisterClassW(&c));
    HWND root=CreateWindowW(c.lpszClassName,L"test",WS_OVERLAPPEDWINDOW,0,0,300,180,NULL,NULL,self,NULL);CHECK(root);
    list=CreateWindowW(vb5?L"ThunderRT5ListBox":L"ThunderRT6ListBox",L"",
        WS_CHILD|WS_VISIBLE|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_MULTIPLESEL|LBS_NOTIFY|LBS_USETABSTOPS,
        10,10,150,70,root,NULL,dll,NULL);CHECK(list && GetDpiForWindow(list)==144);
    CHECK(SendMessageW(list,LB_SETITEMHEIGHT,0,vb5?23:22)!=LB_ERR);
    CHECK(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"first")!=LB_ERR);
    ShowWindow(root,SW_SHOWNOACTIVATE);UpdateWindow(root);
    expected=styled;selected=FALSE;CHECK(repaint());
    CHECK(SendMessageW(list,LB_SETSEL,TRUE,0)!=LB_ERR);selected=TRUE;CHECK(repaint());
    shift=20;expected=FALSE;CHECK(repaint());shift=-1;CHECK(repaint());shift=0;
    EnableWindow(list,FALSE);expected=styled;CHECK(repaint());
    CHECK(SendMessageW(list,LB_SETSEL,FALSE,0)!=LB_ERR);selected=FALSE;CHECK(repaint());
    EnableWindow(list,TRUE);expected=styled;CHECK(repaint());
    CHECK(SendMessageW(list,LB_SETITEMHEIGHT,0,24)!=LB_ERR);expected=FALSE;CHECK(repaint());
    CHECK(DestroyWindow(root) && UnregisterClassW(c.lpszClassName,self) && release() && FreeLibrary(dll));
    printf("VB_CHECKLIST_144_OK bits=%u vb5=%u styled=%u states=4 invalid_geometry=1\n",
        (unsigned)(8*sizeof(void *)),vb5,styled);
    return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==3);WCHAR self[MAX_PATH],cmd[4096];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(int styled=0;styled<2;++styled){
        if(styled)swprintf_s(cmd,4096,L"\"%ls\" --wait \"%ls\" --child \"%ls\" 1",argv[1],self,argv[2]);
        else swprintf_s(cmd,4096,L"\"%ls\" --child \"%ls\" 0",self,argv[2]);
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};
        limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
        STARTUPINFOW si={.cb=sizeof(si),.dwFlags=STARTF_USESTDHANDLES,
            .hStdInput=GetStdHandle(STD_INPUT_HANDLE),.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE),.hStdError=GetStdHandle(STD_ERROR_HANDLE)};
        PROCESS_INFORMATION p={0};CHECK(CreateProcessW(NULL,cmd,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&p));
        CHECK(AssignProcessToJobObject(job,p.hProcess) && ResumeThread(p.hThread)==1);
        DWORD wait=WaitForSingleObject(p.hProcess,10000),code=1;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(p.hProcess,&code);
        CloseHandle(p.hThread);CloseHandle(p.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))return 1;
    if(GetDpiForSystem()!=144){printf("VB_CHECKLIST_144_SKIP dpi=%u\n",GetDpiForSystem());return 77;}
    if(argc==4 && !wcscmp(argv[1],L"--child"))return child(argv[2],!wcscmp(argv[3],L"1"));
    return isolated_process(argc,argv,worker);
}
