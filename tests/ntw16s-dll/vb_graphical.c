#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include "isolated_process.h"
#include "../integration/vb_queue.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_GRAPHICAL_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT (WINAPI *draw)(const DRAWITEMSTRUCT *);
static unsigned notices;
static BOOL application_draw;
static LRESULT CALLBACK parent(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_DRAWITEM && b){const DRAWITEMSTRUCT *item=(const void *)b;
        if(!application_draw)return draw(item);
        RECT rect=item->rcItem;DrawFrameControl(item->hDC,&rect,DFC_BUTTON,DFCS_BUTTONPUSH);
        SelectObject(item->hDC,(HFONT)SendMessageW(item->hwndItem,WM_GETFONT,0,0));
        DrawTextA(item->hDC,"Application",-1,&rect,DT_SINGLELINE);return TRUE;}
    if(m==WM_COMMAND && b && HIWORD(a)==BN_CLICKED)++notices;
    return DefWindowProcW(w,m,a,b);
}
static int survey(vb_queue_state *state,HANDLE event,HWND w,LONG action)
{
    state->window=w;state->root=GetParent(w);state->action=action;++state->generation;
    CHECK(ResetEvent(event));UINT message=RegisterWindowMessageW(VB_QUEUE_MESSAGE);
    CHECK(message && PostMessageW(w,message,state->generation,0));ULONGLONG end=GetTickCount64()+3000;
    for(;;){ULONGLONG now=GetTickCount64();CHECK(now<end);
        DWORD result=MsgWaitForMultipleObjectsEx(1,&event,(DWORD)(end-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result==WAIT_OBJECT_0)break;
        CHECK(result==WAIT_OBJECT_0+1);
        MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
    }
    CHECK(state->completed==(LONG)state->generation);return 0;
}
static int image(HWND w,BOOL classic,BOOL bound,uint64_t *border_hash)
{
    RECT r;CHECK(GetClientRect(w,&r));HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);DWORD *bits=NULL;BITMAPINFO info={0};
    info.bmiHeader=(BITMAPINFOHEADER){.biSize=sizeof(BITMAPINFOHEADER),.biWidth=r.right,.biHeight=-r.bottom,.biPlanes=1,.biBitCount=32};
    HBITMAP image=CreateDIBSection(screen,&info,DIB_RGB_COLORS,(void **)&bits,NULL,0);CHECK(screen && dc && image);
    HGDIOBJ old=SelectObject(dc,image);FillRect(dc,&r,GetSysColorBrush(COLOR_BTNFACE));
    if(bound)SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    else {DRAWITEMSTRUCT item={.CtlType=ODT_BUTTON,.hwndItem=w,.hDC=dc,.rcItem=r};draw(&item);}
    GdiFlush();uint64_t hash=1469598103934665603ull;
    for(int y=0;y<r.bottom;++y)for(int x=0;x<r.right;++x)if(x<3 || y<3 || x>=r.right-3 || y>=r.bottom-3){hash^=bits[y*r.right+x]&0xffffff;hash*=1099511628211ull;}
    if(border_hash)*border_hash=hash;
    unsigned colors=0;for(int i=0;i<r.right*r.bottom;++i)colors+=(bits[i]&0xffffff)==0x0b4dc1;
    COLORREF corner=GetPixel(dc,0,0),edge=GetPixel(dc,1,0);CHECK(colors==81);
    if(classic)CHECK(corner==RGB(255,255,255) && edge==RGB(0,0,0));
    if(classic && (SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED))CHECK(GetPixel(dc,1,1)==RGB(128,128,128));
    CHECK(SelectObject(dc,old) && DeleteObject(image) && DeleteDC(dc) && ReleaseDC(w,screen));
    printf("VB_GRAPHICAL_PIXELS image81=1 classic=%u corner=%06lx edge=%06lx\n",classic,corner,edge);return 0;
}
static int pixels(HWND w,BOOL classic){return image(w,classic,TRUE,NULL);}
static int child(const WCHAR *runtime_path,const WCHAR *witness_path,BOOL styled)
{
    HMODULE runtime=LoadLibraryW(runtime_path);CHECK(runtime);
    BOOL (WINAPI *registration)(void)=(void *)GetProcAddress(runtime,"VbGraphicalRegister");
    BOOL (WINAPI *release)(void)=(void *)GetProcAddress(runtime,"VbGraphicalRelease");
    HFONT (WINAPI *font)(void)=(void *)GetProcAddress(runtime,"VbGraphicalFont");
    void (WINAPI *mode)(unsigned)=(void *)GetProcAddress(runtime,"VbGraphicalMode");
    SIZE (WINAPI *measure)(void)=(void *)GetProcAddress(runtime,"VbGraphicalMeasured");
    void (WINAPI *api_state)(RECT *,DWORD *,DWORD *)=(void *)GetProcAddress(runtime,"VbGraphicalApiState");
    draw=(void *)GetProcAddress(runtime,"VbGraphicalDraw");CHECK(registration && release && font && mode && measure && api_state && draw && registration());
    HMODULE style=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;CHECK(styled?(attached!=NULL):!style);
    WNDCLASSW registered;
    const WCHAR *name=GetClassInfoW(runtime,L"ThunderRT6CommandButton",&registered)?L"ThunderRT6CommandButton":L"ThunderRT5CommandButton";
    WCHAR mapping_name[128],event_name[128];swprintf_s(mapping_name,128,L"Local\\VBGraphical-%lu",GetCurrentProcessId());swprintf_s(event_name,128,L"%ls-ready",mapping_name);
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(vb_queue_state),mapping_name);CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    vb_queue_state *state=MapViewOfFile(mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*state));CHECK(state);ZeroMemory(state,sizeof(*state));
    state->size=sizeof(*state);state->pid=GetCurrentProcessId();state->tid=GetCurrentThreadId();wcscpy_s(state->event_name,128,event_name);
    HANDLE event=CreateEventW(NULL,TRUE,FALSE,event_name);CHECK(event && SetEnvironmentVariableW(VB_QUEUE_ENV,mapping_name));
    HMODULE witness=LoadLibraryW(witness_path);CHECK(witness);HOOKPROC get=(void *)GetProcAddress(witness,"VbQueueGet"),sent=(void *)GetProcAddress(witness,"VbQueueSent");
    BOOL (WINAPI *disconnect)(void)=(void *)GetProcAddress(witness,"VbQueueDisconnect");CHECK(get && sent && disconnect);
    HHOOK get_hook=SetWindowsHookExW(WH_GETMESSAGE,get,NULL,state->tid),sent_hook=SetWindowsHookExW(WH_CALLWNDPROC,sent,NULL,state->tid);CHECK(get_hook && sent_hook);
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent,.lpszClassName=L"GraphicalParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Graphical model",WS_POPUP,0,0,400,250,NULL,NULL,cls.hInstance,NULL);CHECK(root);ShowWindow(root,SW_SHOWNOACTIVATE);
    HWND w=CreateWindowW(name,L"Graphical",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,0,0,150,35,root,(HMENU)1,runtime,NULL);CHECK(w);
    HFONT source=font();SendMessageW(w,WM_SETFONT,(WPARAM)source,FALSE);RECT before,after;CHECK(GetWindowRect(w,&before));
    CHECK(!survey(state,event,w,VB_QUEUE_SURVEY) && (styled?state->attached && state->system_text>0:!state->system_text));CHECK(!pixels(w,styled));
    if(!wcscmp(name,L"ThunderRT6CommandButton")){
        HWND option=CreateWindowW(L"ThunderRT6OptionButton",L"Option",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,170,50,150,35,root,(HMENU)5,runtime,NULL);
        CHECK(option);SendMessageW(option,WM_SETFONT,(WPARAM)source,FALSE);
        CHECK(!survey(state,event,option,VB_QUEUE_SURVEY) && (styled?state->attached && state->system_text>0:!state->system_text));
        uint64_t normal,checked;CHECK(!image(option,styled,TRUE,&normal));mode(5);
        CHECK(!image(option,FALSE,TRUE,&checked));RECT rr;DWORD ee,flags;api_state(&rr,&ee,&flags);CHECK(flags&DFCS_CHECKED);
        if(styled)CHECK(normal!=checked);
        mode(0);CHECK(DestroyWindow(option));
        puts("VB6_GRAPHICAL_OPTION_OK ownerdraw=1 checked_frame=1 native_image=1");
    }
    uint64_t outside,inside;CHECK(!image(w,FALSE,FALSE,&outside) && !image(w,styled,TRUE,&inside));
    CHECK(styled?outside!=inside:outside==inside);
    for(unsigned variant=2;variant<=3;++variant){mode(variant);CHECK(!image(w,FALSE,FALSE,&outside) && !image(w,FALSE,TRUE,&inside) && outside==inside);}mode(0);
    RECT api_before,api_after;DWORD error_before,error_after,flags;mode(4);
    CHECK(!image(w,FALSE,FALSE,NULL));api_state(&api_before,&error_before,&flags);CHECK(flags&DFCS_ADJUSTRECT);
    CHECK(!pixels(w,styled));api_state(&api_after,&error_after,&flags);CHECK(EqualRect(&api_before,&api_after) && error_before==error_after);mode(0);
    application_draw=TRUE;CHECK(!survey(state,event,w,VB_QUEUE_SURVEY) && !state->system_text);application_draw=FALSE;
    puts("VB_GRAPHICAL_SCOPE_OK outside_native=1 unsupported_frame_native=1 application_drawing_native=1");
    CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==source && GetWindowRect(w,&after) && EqualRect(&before,&after));
    SIZE actual=measure();HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HGDIOBJ previous=SelectObject(dc,source);RECT expected={0};
    CHECK(DrawTextA(dc,"Graphical",-1,&expected,DT_CALCRECT|DT_SINGLELINE));CHECK(actual.cx==expected.right && actual.cy==expected.bottom && SelectObject(dc,previous) && DeleteDC(dc));
    CHECK(!survey(state,event,w,VB_QUEUE_PAINT_SURVEY) && state->paint_restored && !state->paint_probe_error && (styled?state->paint_system>0:!state->paint_system));
    HFONT private_font=state->paint_system_font;DWORD gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(unsigned i=0;i<6;++i)CHECK(!survey(state,event,w,VB_QUEUE_SURVEY));
    CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=gdi+2);
    SendMessageW(w,BM_CLICK,0,0);CHECK(notices==1 && !GetCapture());
    SetFocus(w);CHECK(GetFocus()==w && PostMessageW(w,WM_KEYDOWN,VK_SPACE,1) && PostMessageW(w,WM_KEYUP,VK_SPACE,0xc0000001));
    CHECK(!survey(state,event,w,0) && notices==2 && GetFocus()==w && !GetCapture());
    SendMessageW(w,BM_SETSTATE,TRUE,0);CHECK(!pixels(w,styled));api_state(&api_after,&error_after,&flags);CHECK(flags&DFCS_PUSHED);SendMessageW(w,BM_SETSTATE,FALSE,0);
    EnableWindow(w,FALSE);CHECK(!survey(state,event,w,VB_QUEUE_SURVEY) && !pixels(w,styled));api_state(&api_after,&error_after,&flags);CHECK(flags&DFCS_INACTIVE);EnableWindow(w,TRUE);
    puts("VB_GRAPHICAL_API_OK native_rect_and_error=1 pressed_disabled_flags=1");
    HWND late=CreateWindowW(name,L"Late",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,0,50,150,35,root,(HMENU)2,runtime,NULL);CHECK(late);SendMessageW(late,WM_SETFONT,(WPARAM)source,FALSE);
    CHECK(!survey(state,event,late,VB_QUEUE_SURVEY) && (styled?state->system_text>0:!state->system_text));
    HFONT changed=CreateFontW(-17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(changed);
    SendMessageW(late,WM_SETFONT,(WPARAM)changed,FALSE);CHECK(!survey(state,event,late,VB_QUEUE_SURVEY) && (styled?state->system_text>0:!state->system_text) && (HFONT)SendMessageW(late,WM_GETFONT,0,0)==changed);
    HWND other=CreateWindowW(cls.lpszClassName,L"Other",WS_POPUP,0,0,400,250,NULL,NULL,cls.hInstance,NULL);CHECK(other && SetParent(late,other)==root);
    CHECK(!survey(state,event,late,VB_QUEUE_SURVEY) && !state->system_text && (!styled || !attached(late)));CHECK(DestroyWindow(other));
    WNDCLASSW impostor;CHECK(GetClassInfoW(runtime,name,&impostor));impostor.hInstance=cls.hInstance;CHECK(RegisterClassW(&impostor));
    HWND wrong=CreateWindowW(name,L"Wrong",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,170,0,150,35,root,(HMENU)3,cls.hInstance,NULL);CHECK(wrong);SendMessageW(wrong,WM_SETFONT,(WPARAM)source,FALSE);
    CHECK(!survey(state,event,wrong,VB_QUEUE_SURVEY) && !state->system_text && (!styled || !attached(wrong)));CHECK(DestroyWindow(wrong) && UnregisterClassW(name,cls.hInstance));
    HWND live=CreateWindowW(name,L"Live",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,0,100,150,35,root,(HMENU)4,runtime,NULL);CHECK(live);SendMessageW(live,WM_SETFONT,(WPARAM)source,FALSE);
    CHECK(!survey(state,event,live,VB_QUEUE_SURVEY) && (styled?state->system_text>0:!state->system_text));
    SendMessageW(live,BM_SETSTYLE,BS_PUSHBUTTON,FALSE);CHECK(!survey(state,event,live,VB_QUEUE_SURVEY) && !state->system_text && (!styled || !attached(live)));CHECK(DestroyWindow(live));
    if(styled){mode(1);CHECK(!survey(state,event,w,VB_QUEUE_SURVEY) && !attached(w) && !state->system_text);mode(0);
        LOGFONTW lf;CHECK(private_font && !GetObjectW(private_font,sizeof(lf),&lf));puts("VB_GRAPHICAL_RETIRE_OK callback_remove=1 private_font_released=1");}
    CHECK(DestroyWindow(root));LOGFONTW lf;CHECK(GetObjectW(source,sizeof(lf),&lf)==sizeof(lf) && GetObjectW(changed,sizeof(lf),&lf)==sizeof(lf) && DeleteObject(changed));
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance) && UnhookWindowsHookEx(get_hook) && UnhookWindowsHookEx(sent_hook) && disconnect() && FreeLibrary(witness));
    CHECK(UnmapViewOfFile(state) && CloseHandle(mapping) && CloseHandle(event) && SetEnvironmentVariableW(VB_QUEUE_ENV,NULL) && release() && FreeLibrary(runtime));
    printf("VB_GRAPHICAL_CORE_OK bits=%u styled=%u native_data_metrics_image=1 input=1 late_create=1 owned_resources_closed=1\n",(unsigned)(sizeof(void *)*8),styled);return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==4);WCHAR self[MAX_PATH],command[4096];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned styled=0;styled<2;++styled){
        if(styled)swprintf_s(command,4096,L"\"%ls\" --wait \"%ls\" --child \"%ls\" \"%ls\" 1",argv[1],self,argv[2],argv[3]);
        else swprintf_s(command,4096,L"\"%ls\" --child \"%ls\" \"%ls\" 0",self,argv[2],argv[3]);
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION p={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&p));CHECK(AssignProcessToJobObject(job,p.hProcess) && ResumeThread(p.hThread)==1);
        DWORD wait=WaitForSingleObject(p.hProcess,20000),code=1;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(p.hProcess,&code);
        CloseHandle(p.hThread);CloseHandle(p.hProcess);CloseHandle(job);
        if(wait!=WAIT_OBJECT_0 || code)fprintf(stderr,"VB_GRAPHICAL_CHILD styled=%u wait=%lu exit=%lu\n",styled,wait,code);
        CHECK(wait==WAIT_OBJECT_0 && !code);
    }return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==5 && !wcscmp(argv[1],L"--child"))return child(argv[2],argv[3],!wcscmp(argv[4],L"1"));return isolated_process(argc,argv,worker);}
