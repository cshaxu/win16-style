/* Self-contained native witness test; no installed VB or external sample. */
#include <stdio.h>
#include "vb_queue.h"
#include "../ntw16s-dll/isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_QUEUE_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK parent(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+1)return SendMessageW(w,WM_HSCROLL,MAKEWPARAM(SB_LINERIGHT,0),b);
    if(m==WM_HSCROLL && b){HWND bar=(HWND)b;
        if(LOWORD(a)==SB_LINERIGHT)SetScrollPos(bar,SB_CTL,GetScrollPos(bar,SB_CTL)+1,TRUE);
    }
    return DefWindowProcW(w,m,a,b);
}
static DWORD pump(HANDLE event,DWORD timeout)
{
    ULONGLONG deadline=GetTickCount64()+timeout;
    for(;;){
        ULONGLONG now=GetTickCount64();if(now>=deadline)return WAIT_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjectsEx(1,&event,(DWORD)(deadline-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result!=WAIT_OBJECT_0+1)return result;
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR map_name[128],event_name[128];
    CHECK(swprintf_s(map_name,128,L"Local\\VBQueueUnit-%lu",GetCurrentProcessId())>0);
    CHECK(swprintf_s(event_name,128,L"%ls-ready",map_name)>0);
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(vb_queue_state),map_name);
    CHECK(map && GetLastError()!=ERROR_ALREADY_EXISTS);
    vb_queue_state *state=MapViewOfFile(map,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*state));CHECK(state);
    ZeroMemory(state,sizeof(*state));state->size=sizeof(*state);state->pid=GetCurrentProcessId();state->tid=GetCurrentThreadId();state->generation=7;
    CHECK(!wcscpy_s(state->event_name,128,event_name));HANDLE event=CreateEventW(NULL,TRUE,FALSE,event_name);CHECK(event);
    CHECK(SetEnvironmentVariableW(VB_QUEUE_ENV,map_name));
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW cls={.lpfnWndProc=parent,.hInstance=instance,.lpszClassName=L"VBQueueUnit"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"queue",WS_OVERLAPPEDWINDOW,0,0,300,150,NULL,NULL,instance,NULL);CHECK(root);
    HWND bar=CreateWindowW(L"SCROLLBAR",L"",WS_CHILD|WS_VISIBLE|SBS_HORZ,0,0,180,17,root,(HMENU)1,instance,NULL);CHECK(bar);
    SetScrollRange(bar,SB_CTL,0,10,FALSE);state->root=root;state->window=bar;
    HMODULE dll=LoadLibraryW(argv[1]);CHECK(dll);
    HOOKPROC get=(HOOKPROC)(UINT_PTR)GetProcAddress(dll,"VbQueueGet"),sent=(HOOKPROC)(UINT_PTR)GetProcAddress(dll,"VbQueueSent");CHECK(get && sent);
    HHOOK get_hook=SetWindowsHookExW(WH_GETMESSAGE,get,dll,state->tid),sent_hook=SetWindowsHookExW(WH_CALLWNDPROC,sent,dll,state->tid);CHECK(get_hook && sent_hook);
    UINT marker=RegisterWindowMessageW(VB_QUEUE_MESSAGE);CHECK(marker);
    CHECK(PostMessageW(bar,marker,8,0) && PostMessageW(root,WM_APP+1,0,(LPARAM)bar) && PostMessageW(bar,marker,7,0));
    CHECK(pump(event,3000)==WAIT_OBJECT_0 && state->completed==7 && state->acknowledgements==1 && state->rejected==1);
    CHECK(GetScrollPos(bar,SB_CTL)==1 && state->hscroll==1 && state->vscroll==0 && state->scroll_code==SB_LINERIGHT);
    CHECK(UnhookWindowsHookEx(get_hook) && ResetEvent(event));
    CHECK(PostMessageW(bar,marker,7,0) && pump(event,40)==WAIT_TIMEOUT && state->acknowledgements==1);
    CHECK(UnhookWindowsHookEx(sent_hook) && DestroyWindow(root) && UnregisterClassW(cls.lpszClassName,instance));
    CHECK(FreeLibrary(dll) && UnmapViewOfFile(state) && CloseHandle(map) && CloseHandle(event));
    CHECK(SetEnvironmentVariableW(VB_QUEUE_ENV,NULL));
    printf("VB_QUEUE_UNIT_OK bits=%u wrong_tag_rejected=1 notification=1 no_hook_negative=1 resources_closed=1\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
