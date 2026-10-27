#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#include "../../src/ntw16s-dll/control_glyphs.h"
#define CHECK(x) do { if(!(x)){fprintf(stderr,"CHECKLIST_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;} } while(0)
static WNDPROC list_proc;
static HWND target;
static UINT frame_flags;
static int frame_offset,frame_size=13;
static unsigned measured,mismatched,callbacks;
static LRESULT CALLBACK list_wrapper(HWND w,UINT m,WPARAM a,LPARAM b)
{ return CallWindowProcW(list_proc,w,m,a,b); }
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_DRAWITEM && b){
        DRAWITEMSTRUCT *d=(void*)b;
        if(d->CtlType==ODT_LISTBOX && d->hwndItem==target && d->itemID==0){
            RECT r=d->rcItem;r.left+=1+frame_offset;r.right=r.left+frame_size;r.bottom=r.top+frame_size;
            BOOL ok=DrawFrameControl(d->hDC,&r,DFC_BUTTON,DFCS_BUTTONCHECK|frame_flags);
            if(ok){
                unsigned diff=0,valid=0;int col=(frame_flags&DFCS_CHECKED)?14:0;
                for(int y=0;y<frame_size;y++)for(int x=0;x<frame_size;x++){
                    int sx=0,sy=0;for(int j=0;j<14;j++)if(MulDiv(j,frame_size,14)<=x && MulDiv(j+1,frame_size,14)>x){sx=j;break;}
                    for(int j=0;j<13;j++)if(MulDiv(j,frame_size,13)<=y && MulDiv(j+1,frame_size,13)>y){sy=j;break;}
                    COLORREF c=GetPixel(d->hDC,r.left+x,r.top+y);
                    COLORREF want=control_checks[sy][col+sx]=='0'?RGB(0,0,0):RGB(255,255,255);
                    if(c!=CLR_INVALID){valid++;diff+=c!=want;}
                }
                if(valid){measured=valid;mismatched=diff;callbacks++;}
            }
            return TRUE;
        }
    }
    return DefWindowProcW(w,m,a,b);
}
static int child(BOOL styled)
{
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSEXW c={.cbSize=sizeof(c)};
    CHECK(GetClassInfoExW(NULL,L"LISTBOX",&c));list_proc=c.lpfnWndProc;c.lpfnWndProc=list_wrapper;
    c.hInstance=instance;c.lpszClassName=L"THintCheckListBox";c.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&c));
    WNDCLASSW parent={0};parent.hInstance=instance;parent.lpszClassName=L"ChecklistParent";parent.lpfnWndProc=parent_proc;CHECK(RegisterClassW(&parent));
    HWND root=CreateWindowW(parent.lpszClassName,L"Checklist",WS_OVERLAPPEDWINDOW,0,0,300,200,NULL,NULL,instance,NULL);CHECK(root);
    target=CreateWindowW(c.lpszClassName,L"",WS_CHILD|WS_VISIBLE|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS,10,10,180,70,root,(HMENU)42,instance,NULL);CHECK(target);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=dll?(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
    BOOL (__cdecl *remove_control)(HWND)=dll?(void*)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    if(styled)CHECK(attached && remove_control && attached(target));else CHECK(!dll);
    CHECK(SendMessageW(target,LB_ADDSTRING,0,(LPARAM)L"one")!=LB_ERR);
    ShowWindow(root,SW_SHOWNORMAL);UpdateWindow(root);frame_flags=DFCS_CHECKED;callbacks=0;
    CHECK(RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));GdiFlush();
    CHECK(callbacks && measured>0);unsigned checked_mismatch=mismatched;
    if(styled)CHECK(checked_mismatch==0);else CHECK(checked_mismatch>0);
    frame_flags=0;callbacks=0;CHECK(RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));GdiFlush();CHECK(callbacks && measured>0);
    if(styled)CHECK(mismatched==0);
    if(styled){frame_flags=DFCS_CHECKED;
        CHECK(SendMessageW(target,LB_SETITEMHEIGHT,0,24)!=LB_ERR);frame_size=19;callbacks=0;
        CHECK(RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));GdiFlush();
        if(GetDpiForWindow(target)>=120)CHECK(callbacks && measured>0 && mismatched==0);
        frame_size=13;frame_offset=40;callbacks=0;
        CHECK(RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));GdiFlush();CHECK(callbacks && measured>0 && mismatched>0);frame_offset=0;
        HWND unsupported=CreateWindowW(c.lpszClassName,L"",WS_CHILD|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,0,0,100,50,root,NULL,instance,NULL);
        CHECK(unsupported && !attached(unsupported));CHECK(DestroyWindow(unsupported));
    }
    frame_flags=DFCS_CHECKED|DFCS_INACTIVE;callbacks=0;CHECK(RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));GdiFlush();CHECK(callbacks && measured>0);
    unsigned disabled_before=mismatched;
    if(styled){CHECK(remove_control(target) && !attached(target));callbacks=0;
        CHECK(RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));GdiFlush();CHECK(callbacks && mismatched==disabled_before);
        frame_flags=DFCS_CHECKED;callbacks=0;CHECK(RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));GdiFlush();CHECK(callbacks && mismatched>0);
    }
    CHECK(SendMessageW(target,LB_GETCOUNT,0,0)==1);CHECK(DestroyWindow(root));CHECK(UnregisterClassW(c.lpszClassName,instance));CHECK(UnregisterClassW(parent.lpszClassName,instance));
    printf("CHECKLIST_MODEL_OK bits=%u styled=%u checked_mismatch=%u disabled_native=1 offset_rejected=1 detach=1\n",(unsigned)(sizeof(void*)*8),styled,checked_mismatch);return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned mode=0;mode<2;mode++){
        if(mode)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child styled",argv[1],self);
        else swprintf_s(command,2048,L"\"%ls\" --child bare",self);
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        if(!AssignProcessToJobObject(job,pi.hProcess)){TerminateProcess(pi.hProcess,1);CHECK(FALSE);}ResumeThread(pi.hThread);
        DWORD code=1,wait=WaitForSingleObject(pi.hProcess,20000);if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateJobObject(job,ERROR_TIMEOUT);
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info={0};ULONGLONG deadline=GetTickCount64()+5000;
        do {CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&info,sizeof(info),NULL));if(!info.ActiveProcesses)break;Sleep(10);}while(GetTickCount64()<deadline);
        fprintf(stderr,"CHECKLIST_PROCESS mode=%u wait=%lu exit=%lu active=%lu total=%lu\n",mode,wait,code,info.ActiveProcesses,info.TotalProcesses);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && code==0 && info.ActiveProcesses==0);
    }
    puts("CHECKLIST_CLEANUP_OK jobs=2");return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcscmp(argv[2],L"styled"));return isolated_process(argc,argv,worker);}
