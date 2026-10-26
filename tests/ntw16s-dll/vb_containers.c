#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#include "../integration/vb_queue.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_CONTAINER_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static unsigned notices;
static LRESULT CALLBACK root_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+42){RECT r={8,50,210,70};DrawTextA((HDC)a,"Application drawing",-1,&r,DT_SINGLELINE);return 1;}
    if(m==WM_COMMAND && b)++notices;
    return DefWindowProcW(w,m,a,b);
}
static DWORD pump(HANDLE event)
{
    ULONGLONG end=GetTickCount64()+3000;
    for(;;){ULONGLONG now=GetTickCount64();if(now>=end)return WAIT_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjectsEx(1,&event,(DWORD)(end-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result!=WAIT_OBJECT_0+1)return result;
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
}
static int survey(vb_queue_state *state,HANDLE event,HWND w,LONG action)
{
    state->window=w;state->root=GetParent(w);state->action=action;++state->generation;
    CHECK(ResetEvent(event));UINT marker=RegisterWindowMessageW(VB_QUEUE_MESSAGE);
    CHECK(marker && PostMessageW(w,marker,state->generation,0) && pump(event)==WAIT_OBJECT_0);
    CHECK(state->completed==(LONG)state->generation);return 0;
}
static int pixels(HWND w)
{
    RECT r;CHECK(GetClientRect(w,&r));HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);BITMAPINFO info={0};DWORD *data=NULL;
    info.bmiHeader=(BITMAPINFOHEADER){.biSize=sizeof(BITMAPINFOHEADER),.biWidth=r.right,.biHeight=-r.bottom,.biPlanes=1,.biBitCount=32};
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,(void **)&data,NULL,0);CHECK(screen && dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();
    unsigned image=0,line=0;for(int i=0;i<r.right*r.bottom;++i){image+=(data[i]&0xffffff)==0x0b4dc1;line+=(data[i]&0xffffff)==0xc80915;}
    CHECK(SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc) && ReleaseDC(w,screen));
    CHECK(image==144 && line==16);return 0;
}
static int child(const WCHAR *runtime_path,const WCHAR *witness_path,BOOL styled)
{
    notices=0;
    HMODULE runtime=LoadLibraryW(runtime_path);CHECK(runtime);
    BOOL (WINAPI *registration)(void)=(void *)GetProcAddress(runtime,"VbContainerRegister");
    BOOL (WINAPI *release)(void)=(void *)GetProcAddress(runtime,"VbContainerRelease");
    unsigned (WINAPI *version)(void)=(void *)GetProcAddress(runtime,"VbContainerVersion");
    BOOL (WINAPI *inspect)(HWND,SIZE *,unsigned *,unsigned *)=(void *)GetProcAddress(runtime,"VbContainerInspect");
    void (WINAPI *retire)(HWND)=(void *)GetProcAddress(runtime,"VbContainerRetire");
    CHECK(registration && release && version && inspect && retire && registration());unsigned vb=version();
    HMODULE style=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;
    BOOL (__cdecl *remove)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowRemove"):NULL;CHECK(styled?(attached && remove):!style);
    WCHAR name[128],event_name[128];swprintf_s(name,128,L"Local\\VBContainer-%lu",GetCurrentProcessId());swprintf_s(event_name,128,L"%ls-ready",name);
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(vb_queue_state),name);CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    vb_queue_state *state=MapViewOfFile(mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*state));CHECK(state);ZeroMemory(state,sizeof(*state));
    state->size=sizeof(*state);state->pid=GetCurrentProcessId();state->tid=GetCurrentThreadId();wcscpy_s(state->event_name,128,event_name);
    HANDLE event=CreateEventW(NULL,TRUE,FALSE,event_name);CHECK(event && SetEnvironmentVariableW(VB_QUEUE_ENV,name));
    HMODULE witness=LoadLibraryW(witness_path);CHECK(witness);HOOKPROC get=(void *)GetProcAddress(witness,"VbQueueGet"),sent=(void *)GetProcAddress(witness,"VbQueueSent");CHECK(get && sent);
    HHOOK get_hook=SetWindowsHookExW(WH_GETMESSAGE,get,witness,state->tid),sent_hook=SetWindowsHookExW(WH_CALLWNDPROC,sent,witness,state->tid);CHECK(get_hook && sent_hook);
    GdiFlush();DWORD initial_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=root_proc,.lpszClassName=L"ContainerModelParent"};CHECK(RegisterClassW(&cls));
    /* This fixture isolates child-container ownership; title/menu lifetime is
     * covered by the existing frame tests and the authentic application. */
    HWND root=CreateWindowW(cls.lpszClassName,L"Container model",WS_POPUP,0,0,500,360,NULL,NULL,cls.hInstance,NULL);CHECK(root);ShowWindow(root,SW_SHOWNOACTIVATE);
    const WCHAR *class_name=vb==6?L"ThunderRT6PictureBoxDC":L"ThunderRT5PictureBox";
    HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,class_name,L"",WS_CHILD|WS_VISIBLE,0,0,230,100,root,(HMENU)1,runtime,NULL);CHECK(w);
    HFONT source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);CHECK(source);RECT before,after;CHECK(GetWindowRect(w,&before));
    CHECK(!survey(state,event,w,VB_QUEUE_SURVEY));CHECK(styled?(state->attached && state->system_text>0 && state->native_text>=2):!state->system_text && state->native_text>=3);
    CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==source && GetWindowRect(w,&after) && EqualRect(&before,&after));CHECK(!pixels(w));
    SIZE actual;unsigned clicks,keys;CHECK(inspect(w,&actual,&clicks,&keys));
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HGDIOBJ font=SelectObject(dc,source);RECT measure={0};CHECK(DrawTextA(dc,"Label sample",-1,&measure,DT_CALCRECT|DT_SINGLELINE));
    CHECK(actual.cx==measure.right && actual.cy==measure.bottom && SelectObject(dc,font) && DeleteDC(dc));
    CHECK(!survey(state,event,w,VB_QUEUE_PAINT_SURVEY));CHECK(state->paint_restored && !state->paint_probe_error && state->paint_messages>0);
    CHECK(styled?state->paint_system>0:!state->paint_system);
    HFONT first_private=state->paint_system_font;
    printf("VB_CONTAINER_PAINT_OK vb=%u styled=%u system=%ld native=%ld label_only=1 print_and_app_native=1 image144_line16=1 source_metrics=1\n",vb,styled,state->paint_system,state->paint_native);
    DWORD gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(unsigned i=0;i<6;++i)CHECK(!survey(state,event,w,VB_QUEUE_SURVEY));
    CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=gdi+2);
    SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(80,80));SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(80,80));
    CHECK(PostMessageW(w,WM_KEYDOWN,VK_RIGHT,1) && !survey(state,event,w,0));CHECK(inspect(w,&actual,&clicks,&keys) && clicks==1 && keys==1 && notices==2 && GetFocus()==w && !GetCapture());
    HWND late=CreateWindowW(class_name,L"",WS_CHILD|WS_VISIBLE,0,120,230,100,root,(HMENU)2,runtime,NULL);CHECK(late);
    CHECK(!survey(state,event,late,VB_QUEUE_SURVEY) && (styled?state->system_text>0:!state->system_text));CHECK(!pixels(late));
    WNDCLASSW impostor;CHECK(GetClassInfoW(runtime,class_name,&impostor));impostor.hInstance=cls.hInstance;CHECK(RegisterClassW(&impostor));
    HWND wrong=CreateWindowW(class_name,L"",WS_CHILD|WS_VISIBLE,240,0,230,100,root,(HMENU)3,cls.hInstance,NULL);CHECK(wrong);
    CHECK(!survey(state,event,wrong,VB_QUEUE_SURVEY) && !state->system_text && (!styled || !attached(wrong)));CHECK(DestroyWindow(wrong) && UnregisterClassW(class_name,cls.hInstance));
    HFONT changed=CreateFontW(-17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(changed);
    SendMessageW(late,WM_SETFONT,(WPARAM)changed,FALSE);CHECK(!survey(state,event,late,VB_QUEUE_SURVEY));CHECK((HFONT)SendMessageW(late,WM_GETFONT,0,0)==changed && (styled?state->system_text>0:!state->system_text));
    CHECK(!survey(state,event,late,VB_QUEUE_PAINT_SURVEY));HFONT second_private=state->paint_system_font;
    if(styled){
        DWORD active_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        retire(w);CHECK(!survey(state,event,w,VB_QUEUE_SURVEY) && !attached(w) && !state->system_text && !pixels(w));
        SetWindowLongPtrW(late,GWL_EXSTYLE,WS_EX_LAYOUTRTL);CHECK(!survey(state,event,late,VB_QUEUE_SURVEY) && !attached(late) && !state->system_text);
        DWORD immediate_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);GdiFlush();DWORD removed_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        LOGFONTW first,second;int first_live=GetObjectW(first_private,sizeof(first),&first),second_live=GetObjectW(second_private,sizeof(second),&second);
        printf("VB_CONTAINER_GDI_RETIRE active=%lu immediate=%lu flushed=%lu font1=%p/live%d font2=%p/live%d\n",active_gdi,immediate_gdi,removed_gdi,(void *)first_private,first_live,(void *)second_private,second_live);
        CHECK(first_private && second_private && first_private!=second_private && !first_live && !second_live);
        printf("VB_CONTAINER_LEASES_OK before=%lu after=%lu private_fonts_released=2\n",active_gdi,removed_gdi);
        puts("VB_CONTAINER_RETIRE_OK callback_remove=1 rtl_fallback=1");
    }else{
        DWORD before_rtl=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        SetWindowLongPtrW(late,GWL_EXSTYLE,WS_EX_LAYOUTRTL);CHECK(!survey(state,event,late,VB_QUEUE_SURVEY) && !state->system_text);GdiFlush();
        printf("VB_CONTAINER_NATIVE_RTL before=%lu after=%lu\n",before_rtl,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS));
    }
    LOGFONTW lf;CHECK(GetObjectW(source,sizeof(lf),&lf)==sizeof(lf) && GetObjectW(changed,sizeof(lf),&lf)==sizeof(lf));
    CHECK(DestroyWindow(root) && DeleteObject(changed) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    BOOL (WINAPI *disconnect)(void)=(void *)GetProcAddress(witness,"VbQueueDisconnect");CHECK(disconnect);
    CHECK(UnhookWindowsHookEx(get_hook) && UnhookWindowsHookEx(sent_hook) && disconnect() && FreeLibrary(witness));
    CHECK(UnmapViewOfFile(state) && CloseHandle(mapping) && CloseHandle(event) && SetEnvironmentVariableW(VB_QUEUE_ENV,NULL));CHECK(release() && FreeLibrary(runtime));
    MSG pending;while(PeekMessageW(&pending,NULL,0,0,PM_REMOVE)){TranslateMessage(&pending);DispatchMessageW(&pending);}
    /* First-use GUI/font caches also change this count in the bare model.
     * Keep it diagnostic; ownership assertions above inspect the exact two
     * private fonts, native source fonts and every explicitly owned handle. */
    GdiFlush();DWORD final_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);printf("VB_CONTAINER_FINAL_GDI initial=%lu final=%lu aggregate_diagnostic_only=1\n",initial_gdi,final_gdi);
    printf("VB_CONTAINER_OK bits=%u vb=%u styled=%u fonts_images_native=1 late_create=1 wrong_owner=1 input=1 owned_resources_closed=1\n",(unsigned)(sizeof(void *)*8),vb,styled);return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==4);WCHAR self[MAX_PATH],command[4096];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned mode=0;mode<2;++mode){
        if(mode)swprintf_s(command,4096,L"\"%ls\" --wait \"%ls\" --child \"%ls\" \"%ls\" 1",argv[1],self,argv[2],argv[3]);
        else swprintf_s(command,4096,L"\"%ls\" --child \"%ls\" \"%ls\" 0",self,argv[2],argv[3]);
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION process={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&process));CHECK(AssignProcessToJobObject(job,process.hProcess) && ResumeThread(process.hThread)==1);
        DWORD wait=WaitForSingleObject(process.hProcess,20000),code=1;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&code);
        CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==5 && !wcscmp(argv[1],L"--child"))return child(argv[2],argv[3],!wcscmp(argv[4],L"1"));return isolated_process(argc,argv,worker);}
