#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#include "../integration/vb_queue.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_FILES_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static DWORD pump(HANDLE event,DWORD timeout)
{
    ULONGLONG end=GetTickCount64()+timeout;
    for(;;){ULONGLONG now=GetTickCount64();if(now>=end)return WAIT_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjectsEx(1,&event,(DWORD)(end-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result!=WAIT_OBJECT_0+1)return result;
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
}
static int survey(vb_queue_state *state,HANDLE event,HWND w,LONG action)
{
    state->window=w;state->root=GetParent(w);state->action=action;++state->generation;
    CHECK(ResetEvent(event));UINT marker=RegisterWindowMessageW(VB_QUEUE_MESSAGE);CHECK(marker);
    CHECK(PostMessageW(w,marker,state->generation,0) && pump(event,3000)==WAIT_OBJECT_0);
    CHECK(state->completed==(LONG)state->generation);return 0;
}
static unsigned image_pixels(HWND w,BOOL combo)
{
    RECT client,item;GetClientRect(w,&client);
    if(combo){COMBOBOXINFO info={.cbSize=sizeof(info)};if(!GetComboBoxInfo(w,&info))return CLR_INVALID;item=info.rcItem;}
    else if(SendMessageW(w,LB_GETITEMRECT,0,(LPARAM)&item)==LB_ERR)return CLR_INVALID;
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);BITMAPINFO info={0};DWORD *pixels=NULL;
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=client.right;info.bmiHeader.biHeight=-client.bottom;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP image=CreateDIBSection(screen,&info,DIB_RGB_COLORS,(void **)&pixels,NULL,0);
    if(!screen || !dc || !image)ExitProcess(82);
    HGDIOBJ old=SelectObject(dc,image);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    GdiFlush();unsigned count=0;for(int i=0;i<client.right*client.bottom;++i)count+=(pixels[i]&0xffffff)==0x0b4dc1;
    printf("VB_FILES_IMAGE count=%u combo=%u item=%ld,%ld,%ld,%ld client=%ld,%ld\n",count,combo,item.left,item.top,item.right,item.bottom,client.right,client.bottom);
    SelectObject(dc,old);DeleteObject(image);DeleteDC(dc);ReleaseDC(w,screen);return count;
}
static int child(const WCHAR *runtime_path,const WCHAR *witness_path,BOOL styled)
{
    HMODULE runtime=LoadLibraryW(runtime_path);CHECK(runtime);
    BOOL (WINAPI *registration)(void)=(void *)GetProcAddress(runtime,"VbFileRegister");
    BOOL (WINAPI *release)(void)=(void *)GetProcAddress(runtime,"VbFileRelease");
    void (WINAPI *retire)(HWND)=(void *)GetProcAddress(runtime,"VbFileRemoveOnDraw");
    unsigned (WINAPI *version)(void)=(void *)GetProcAddress(runtime,"VbFileVersion");
    CHECK(registration && release && retire && version && registration());
    BOOL vb6=version()==6;unsigned total=vb6?2:3;
    HMODULE dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
    BOOL (__cdecl *remove)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    CHECK(styled?(attached && remove):!dll);
    WCHAR name[128],event_name[128];swprintf_s(name,128,L"Local\\VBFileProbe-%lu",GetCurrentProcessId());swprintf_s(event_name,128,L"%ls-ready",name);
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(vb_queue_state),name);CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    vb_queue_state *state=MapViewOfFile(mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*state));CHECK(state);ZeroMemory(state,sizeof(*state));
    state->size=sizeof(*state);state->pid=GetCurrentProcessId();state->tid=GetCurrentThreadId();wcscpy_s(state->event_name,128,event_name);
    HANDLE event=CreateEventW(NULL,TRUE,FALSE,event_name);CHECK(event && SetEnvironmentVariableW(VB_QUEUE_ENV,name));
    HMODULE witness=LoadLibraryW(witness_path);CHECK(witness);
    HOOKPROC get=(HOOKPROC)(UINT_PTR)GetProcAddress(witness,"VbQueueGet"),sent=(HOOKPROC)(UINT_PTR)GetProcAddress(witness,"VbQueueSent");CHECK(get && sent);
    HHOOK get_hook=SetWindowsHookExW(WH_GETMESSAGE,get,witness,state->tid),sent_hook=SetWindowsHookExW(WH_CALLWNDPROC,sent,witness,state->tid);CHECK(get_hook && sent_hook);
    HWND root=CreateWindowW(L"VBFileFixtureParent",L"File controls",WS_OVERLAPPEDWINDOW,0,0,450,300,NULL,NULL,runtime,NULL);CHECK(root);
    const WCHAR *names[]={vb6?L"ThunderRT6FileListBox":L"ThunderRT5FileListBox",vb6?L"ThunderRT6DirListBox":L"ThunderRT5DirListBox",L"ThunderRT5DriveListBox"};
    HWND controls[3];HFONT sources[3];
    for(unsigned i=0;i<total;++i){DWORD style=WS_CHILD|WS_VISIBLE|(i==2?(CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS):(LBS_NOTIFY|LBS_HASSTRINGS|(i==1?LBS_OWNERDRAWVARIABLE:0)));
        controls[i]=CreateWindowExW(i==2?0:WS_EX_CLIENTEDGE,names[i],L"",style,(int)i*140,0,130,i==2?150:100,root,(HMENU)(UINT_PTR)(i+1),runtime,NULL);CHECK(controls[i]);
        sources[i]=(HFONT)SendMessageW(controls[i],WM_GETFONT,0,0);CHECK(sources[i]);
        for(unsigned item=0;item<3;++item){WCHAR text[40];swprintf_s(text,40,L"Native item %u",item);CHECK(SendMessageW(controls[i],i==2?CB_ADDSTRING:LB_ADDSTRING,0,(LPARAM)text)>=0);}
        SendMessageW(controls[i],i==2?CB_SETCURSEL:LB_SETCURSEL,1,0);
        CHECK(!survey(state,event,controls[i],i==2?VB_QUEUE_POPUP_SURVEY:VB_QUEUE_SURVEY));
        CHECK(styled?(state->attached && state->system_text>0):state->system_text==0);
        CHECK((HFONT)SendMessageW(controls[i],WM_GETFONT,0,0)==sources[i]);
        CHECK(SendMessageW(controls[i],i==2?CB_GETCOUNT:LB_GETCOUNT,0,0)==3 && SendMessageW(controls[i],i==2?CB_GETCURSEL:LB_GETCURSEL,0,0)==1);
        if(i)CHECK(image_pixels(controls[i],i==2)==(i==2?144u:432u));
        printf("VB_FILES_RENDER_OK index=%u styled=%u system_text=%ld native_text=%ld\n",i,styled,state->system_text,state->native_text);
    }
    WNDCLASSW impostor;CHECK(GetClassInfoW(NULL,L"LISTBOX",&impostor));impostor.hInstance=GetModuleHandleW(NULL);impostor.lpszClassName=names[1];CHECK(RegisterClassW(&impostor));
    HWND wrong=CreateWindowW(impostor.lpszClassName,L"",WS_CHILD|WS_VISIBLE|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,0,120,130,80,root,(HMENU)9,impostor.hInstance,NULL);CHECK(wrong);
    CHECK(SendMessageW(wrong,LB_ADDSTRING,0,(LPARAM)L"Untrusted owner")>=0);
    CHECK(!survey(state,event,wrong,VB_QUEUE_SURVEY));CHECK(state->system_text==0 && (!styled || !attached(wrong)));
    CHECK(DestroyWindow(wrong) && UnregisterClassW(impostor.lpszClassName,impostor.hInstance));
    DWORD gdi_before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(unsigned repeat=0;repeat<8;++repeat)for(unsigned i=0;i<total;++i){
        CHECK(!survey(state,event,controls[i],VB_QUEUE_SURVEY));
    }
    DWORD gdi_after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(gdi_after<=gdi_before+2);
    printf("VB_FILES_OWNERSHIP_OK wrong_runtime_owner_rejected=1 native_images_preserved=%u repeat_surveys=%u gdi_delta=%ld\n",total-1,total*8,(LONG)(gdi_after-gdi_before));
    ShowWindow(root,SW_SHOWNOACTIVATE);
    for(unsigned i=0;i<total;++i){
        CHECK(SetFocus(controls[i]) || GetFocus()==controls[i]);
        state->commands=0;
        CHECK(PostMessageW(controls[i],WM_KEYDOWN,i==2?VK_DOWN:VK_END,1));
        CHECK(PostMessageW(controls[i],WM_KEYUP,i==2?VK_DOWN:VK_END,(LPARAM)0xc0000001u));
        CHECK(!survey(state,event,controls[i],0));
        CHECK(SendMessageW(controls[i],i==2?CB_GETCURSEL:LB_GETCURSEL,0,0)==2 && state->commands>0);
    }
    if(!vb6){SendMessageW(controls[2],CB_SHOWDROPDOWN,TRUE,0);CHECK(SendMessageW(controls[2],CB_GETDROPPEDSTATE,0,0));
    CHECK(PostMessageW(controls[2],WM_KEYDOWN,VK_ESCAPE,1) && !survey(state,event,controls[2],0));
    CHECK(!SendMessageW(controls[2],CB_GETDROPPEDSTATE,0,0) && SendMessageW(controls[2],CB_GETCURSEL,0,0)==2);
    }
    printf("VB_FILES_INPUT_OK queued_keys=%u parent_notifications=%u popup_escape=%u selection_preserved=1\n",total,total,!vb6);
    if(styled){
        retire(controls[1]);CHECK(!survey(state,event,controls[1],VB_QUEUE_SURVEY));CHECK(!attached(controls[1]) && state->system_text==0);
        CHECK(image_pixels(controls[1],FALSE)==432);
        HWND second=CreateWindowW(L"VBFileFixtureParent",L"directory parent",WS_OVERLAPPEDWINDOW,0,0,300,200,NULL,NULL,runtime,NULL);CHECK(second);
        HWND directory=CreateWindowW(names[1],L"",WS_CHILD|WS_VISIBLE|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS,0,0,130,80,root,(HMENU)8,runtime,NULL);CHECK(directory);
        CHECK(SendMessageW(directory,LB_ADDSTRING,0,(LPARAM)L"Directory")>=0);
        CHECK(!survey(state,event,directory,VB_QUEUE_SURVEY) && attached(directory) && state->system_text>0);
        CHECK(SetParent(directory,second)==root);SendMessageW(directory,WM_NULL,0,0);
        CHECK(!survey(state,event,directory,VB_QUEUE_SURVEY) && !attached(directory) && !state->system_text);
        CHECK(image_pixels(directory,FALSE)==144 && DestroyWindow(second));
        if(!vb6){CHECK(!survey(state,event,controls[2],VB_QUEUE_SURVEY));CHECK(attached(controls[2]));
        HWND other=CreateWindowW(L"VBFileFixtureParent",L"other",WS_OVERLAPPEDWINDOW,0,0,300,200,NULL,NULL,runtime,NULL);CHECK(other);
        CHECK(SetParent(controls[2],other)==root);SendMessageW(controls[2],WM_NULL,0,0);CHECK(!attached(controls[2]));
        CHECK(!survey(state,event,controls[2],VB_QUEUE_SURVEY) && state->system_text==0);
        CHECK(image_pixels(controls[2],TRUE)==144);
        CHECK(SetParent(controls[2],root)==other && DestroyWindow(other));}
        CHECK(remove(controls[0]));CHECK(!survey(state,event,controls[0],VB_QUEUE_SURVEY) && !state->system_text);
        printf("VB_FILES_RETIRE_OK draw_callback=1 reparent=1 drive_reparent=%u file_remove=1\n",!vb6);
    }
    for(unsigned i=0;i<total;++i){LOGFONTW font;CHECK(GetObjectW(sources[i],sizeof(font),&font)==sizeof(font));}
    CHECK(DestroyWindow(root) && UnhookWindowsHookEx(get_hook) && UnhookWindowsHookEx(sent_hook));
    CHECK(FreeLibrary(witness) && UnmapViewOfFile(state) && CloseHandle(mapping) && CloseHandle(event));
    CHECK(SetEnvironmentVariableW(VB_QUEUE_ENV,NULL) && release() && FreeLibrary(runtime));
    printf("VB_FILES_OK bits=%u styled=%u vb=%u source_fonts_preserved=1 resources_closed=1\n",(unsigned)(sizeof(void *)*8),styled,vb6?6:5);return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==4);WCHAR self[MAX_PATH],command[4096];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned mode=0;mode<2;++mode){
        if(mode)swprintf_s(command,4096,L"\"%ls\" --wait \"%ls\" --child \"%ls\" \"%ls\" 1",argv[1],self,argv[2],argv[3]);
        else swprintf_s(command,4096,L"\"%ls\" --child \"%ls\" \"%ls\" 0",self,argv[2],argv[3]);
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION process={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&process));
        CHECK(AssignProcessToJobObject(job,process.hProcess) && ResumeThread(process.hThread)==1);
        DWORD wait=WaitForSingleObject(process.hProcess,20000),code=1;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&code);
        CloseHandle(process.hThread);CloseHandle(process.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==5 && !wcscmp(argv[1],L"--child"))return child(argv[2],argv[3],!wcscmp(argv[4],L"1"));return isolated_process(argc,argv,worker);}
