#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PROGRESS_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static BOOL (__cdecl *attached)(HWND),(__cdecl *remove_style)(HWND);
static WNDPROC native_proc;static unsigned action;static BOOL fired,emulate_state;
static LRESULT CALLBACK wrapper(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==PBM_GETSTATE && emulate_state)return PBST_NORMAL;
    if(m==PBM_GETRANGE && action && !fired && attached && attached(w)){
        unsigned selected=action;fired=TRUE;
        if(selected==1)remove_style(w);
        if(selected==2){DestroyWindow(w);return 0;}
        if(selected==3)SendMessageW(w,PBM_SETPOS,75,0);
    }
    return CallWindowProcW(native_proc,w,m,p,l);
}
static DWORD native_pixels[320*96],classic_pixels[320*96],frame_pixels[320*96];
static HDC dc;static HBITMAP bitmap;static HGDIOBJ previous;static DWORD *pixels;
static void capture(HWND w,DWORD *output,BOOL frame)
{
    RECT clear={0,0,320,96};int saved=SaveDC(dc);FillRect(dc,&clear,GetStockObject(WHITE_BRUSH));
    SendMessageW(w,frame?WM_PRINT:WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND|(frame?PRF_NONCLIENT:0));GdiFlush();CopyMemory(output,pixels,sizeof(native_pixels));RestoreDC(dc,saved);
}
static void save_progress(HWND w,unsigned aware,unsigned state)
{
    WCHAR directory[32768],path[32768];RECT r;GetClientRect(w,&r);
    if(!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,32768))return;
    if(swprintf_s(path,32768,L"%ls/progress-%u-aware%u-state%u.bmp",directory,(unsigned)sizeof(void*)*8,aware,state)<0)return;
    FILE *file=_wfopen(path,L"wb");if(!file)return;
    BITMAPFILEHEADER header={.bfType=0x4d42,.bfOffBits=54,.bfSize=54+(DWORD)r.right*(DWORD)r.bottom*4};BITMAPINFOHEADER info={.biSize=40,.biWidth=r.right,.biHeight=-r.bottom,.biPlanes=1,.biBitCount=32};
    fwrite(&header,sizeof(header),1,file);fwrite(&info,sizeof(info),1,file);
    for(int y=0;y<r.bottom;++y)fwrite(classic_pixels+y*320,sizeof(DWORD),(size_t)r.right,file);
    fclose(file);
}
static unsigned count_color(const DWORD *bits,RECT r,COLORREF color)
{unsigned found=0;DWORD rgb=((color&255)<<16)|(color&0xff00)|((color>>16)&255);for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x)found+=(bits[y*320+x]&0xffffff)==rgb;return found;}
static int child(BOOL styled,BOOL aware)
{
    CHECK(ImmDisableIME(0));if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_PROGRESS_CLASS};CHECK(InitCommonControlsEx(&init));
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(styled?dll!=NULL:dll==NULL);
    attached=dll?(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;remove_style=dll?(void*)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;if(styled)CHECK(attached && remove_style);
    WNDCLASSW parent={.lpfnWndProc=DefWindowProcW,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"ProgressProtocolParent"};CHECK(RegisterClassW(&parent));
    HWND root=CreateWindowW(parent.lpszClassName,L"progress protocol",WS_OVERLAPPEDWINDOW,0,0,640,280,NULL,NULL,parent.hInstance,NULL);CHECK(root);ShowWindow(root,SW_SHOWNORMAL);
    WNDCLASSEXW alias={.cbSize=sizeof(alias)};CHECK(GetClassInfoExW(NULL,PROGRESS_CLASSW,&alias));native_proc=alias.lpfnWndProc;alias.hInstance=parent.hInstance;alias.lpszClassName=L"TProgressBar";alias.lpfnWndProc=wrapper;alias.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&alias));
    dc=CreateCompatibleDC(NULL);BITMAPINFO info={0};info.bmiHeader.biSize=40;info.bmiHeader.biWidth=320;info.bmiHeader.biHeight=-96;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,(void**)&pixels,NULL,0);CHECK(dc&&bitmap);previous=SelectObject(dc,bitmap);
    if(styled){WCHAR module_path[32768];CHECK(attached(root) && GetModuleFileNameW(dll,module_path,32768));
        printf("PROGRESS_MODULE bits=%u pid=%lu path=%ls bootstrap_frame_attached=1\n",(unsigned)sizeof(void*)*8,GetCurrentProcessId(),module_path);}
    unsigned supported=0,fallback=0;const WCHAR *classes[]={PROGRESS_CLASSW,L"TProgressBar"};
    for(unsigned kind=0;kind<2;++kind)for(unsigned vertical=0;vertical<2;++vertical)for(unsigned smooth=0;smooth<2;++smooth){
        DWORD style=WS_CHILD|WS_VISIBLE|(vertical?PBS_VERTICAL:0)|(smooth?PBS_SMOOTH:0)|((kind&&smooth)?PBS_SMOOTHREVERSE:0);int width=vertical?36:300,height=vertical?80:30;
        HWND w=CreateWindowExW(WS_EX_NOPARENTNOTIFY,classes[kind],L"",style,0,0,width,height,root,(HMENU)41,parent.hInstance,NULL);CHECK(w);
        LRESULT initial_state=SendMessageW(w,PBM_GETSTATE,0,0);BOOL capability=initial_state>=PBST_NORMAL && initial_state<=PBST_PAUSED,classic=styled&&capability;
        if(styled){CHECK(attached(w)==classic);}
        if(classic)++supported;else ++fallback;
        RECT client,before,after;GetClientRect(w,&client);before=client;COLORREF bar=RGB(31,127,219),background=RGB(241,239,233);
        SendMessageW(w,PBM_SETRANGE32,0,100);LRESULT oldstep=SendMessageW(w,PBM_SETSTEP,7,0);(void)oldstep;SendMessageW(w,PBM_SETPOS,50,0);SendMessageW(w,PBM_SETBARCOLOR,0,bar);SendMessageW(w,PBM_SETBKCOLOR,0,background);
        CHECK((int)SendMessageW(w,PBM_GETPOS,0,0)==50);if(capability)CHECK(SendMessageW(w,PBM_GETSTEP,0,0)==7 && (COLORREF)SendMessageW(w,PBM_GETBARCOLOR,0,0)==bar && (COLORREF)SendMessageW(w,PBM_GETBKCOLOR,0,0)==background);
        capture(w,classic_pixels,FALSE);capture(w,frame_pixels,TRUE);
        if(classic){CHECK(!memcmp(classic_pixels,frame_pixels,sizeof(classic_pixels)));CHECK(count_color(classic_pixels,client,bar)>10 && count_color(classic_pixels,client,background)>10 && count_color(classic_pixels,client,0)>10);
            int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));RECT inner=client;InflateRect(&inner,-u,-u);
            if(smooth){unsigned expected=(unsigned)((vertical?(inner.bottom-inner.top)/2:(inner.right-inner.left)/2)*(vertical?inner.right-inner.left:inner.bottom-inner.top));CHECK(count_color(classic_pixels,client,bar)==expected);}
            HDC screen=GetDC(w);CHECK(screen);RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);BOOL visible_edge=GetPixel(screen,0,0)==0;ReleaseDC(w,screen);CHECK(visible_edge);
            for(unsigned state=PBST_NORMAL;state<=PBST_PAUSED;++state){SendMessageW(w,PBM_SETSTATE,state,0);capture(w,classic_pixels,FALSE);COLORREF expected=state==PBST_ERROR?RGB(192,0,0):state==PBST_PAUSED?RGB(192,192,0):bar;CHECK(count_color(classic_pixels,client,expected)>10 && SendMessageW(w,PBM_GETSTATE,0,0)==(LRESULT)state);if(!kind&&!vertical&&smooth)save_progress(w,aware,state);}
            SendMessageW(w,PBM_SETSTATE,PBST_NORMAL,0);SendMessageW(w,PBM_SETRANGE32,(WPARAM)(INT_PTR)INT32_MIN,(LPARAM)INT32_MAX);SendMessageW(w,PBM_SETPOS,0,0);capture(w,classic_pixels,FALSE);PBRANGE large={0};SendMessageW(w,PBM_GETRANGE,0,(LPARAM)&large);CHECK(large.iLow==INT32_MIN && large.iHigh==INT32_MAX && count_color(classic_pixels,client,bar)>10);
            SendMessageW(w,PBM_SETRANGE32,0,100);SendMessageW(w,PBM_SETPOS,10,0);CHECK(SendMessageW(w,PBM_STEPIT,0,0)==10 && SendMessageW(w,PBM_GETPOS,0,0)==17);CHECK(SendMessageW(w,PBM_DELTAPOS,3,0)==17 && SendMessageW(w,PBM_GETPOS,0,0)==20);
            SendMessageW(w,PBM_SETSTATE,PBST_PAUSED,0);EnableWindow(w,FALSE);capture(w,classic_pixels,FALSE);CHECK(count_color(classic_pixels,client,RGB(128,128,128))>10);EnableWindow(w,TRUE);SendMessageW(w,PBM_SETSTATE,PBST_NORMAL,0);SendMessageW(w,PBM_SETPOS,50,0);
        }
        HFONT font=CreateFontW(-13,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,0,0,L"Arial");CHECK(font);SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);HFONT stored=(HFONT)SendMessageW(w,WM_GETFONT,0,0);capture(w,classic_pixels,FALSE);
        if(classic){CHECK(remove_style(w)&&!attached(w));}
        capture(w,native_pixels,FALSE);unsigned difference=0;for(int y=0;y<client.bottom;++y)for(int x=0;x<client.right;++x)difference+=(native_pixels[y*320+x]&0xffffff)!=(classic_pixels[y*320+x]&0xffffff);CHECK(classic?difference>10:difference==0);
        PBRANGE range={0};SendMessageW(w,PBM_GETRANGE,0,(LPARAM)&range);GetClientRect(w,&after);CHECK(range.iLow==0 && range.iHigh==100 && SendMessageW(w,PBM_GETPOS,0,0)==50 && (HFONT)SendMessageW(w,WM_GETFONT,0,0)==stored && EqualRect(&before,&after));
        printf("PROGRESS_VARIANT kind=%u vertical=%u smooth=%u styled=%u capable=%u pixels=%u native_data_geometry_font=1\n",kind,vertical,smooth,styled,capability,difference);
        CHECK(DestroyWindow(w)&&DeleteObject(font));
    }
    if(styled){
        HWND w=CreateWindowW(PROGRESS_CLASSW,L"",WS_CHILD|PBS_MARQUEE,0,0,300,30,root,(HMENU)42,parent.hInstance,NULL);CHECK(w&&!attached(w));CHECK(DestroyWindow(w));
        w=CreateWindowExW(WS_EX_LAYOUTRTL,PROGRESS_CLASSW,L"",WS_CHILD|PBS_SMOOTH,0,0,300,30,root,(HMENU)43,parent.hInstance,NULL);CHECK(w&&!attached(w));CHECK(DestroyWindow(w));
        if(supported){
            w=CreateWindowW(PROGRESS_CLASSW,L"",WS_CHILD|PBS_SMOOTH,0,0,1,1,root,(HMENU)46,parent.hInstance,NULL);CHECK(w&&attached(w));
            SendMessageW(w,PBM_SETRANGE32,50,50);SendMessageW(w,PBM_SETPOS,50,0);capture(w,classic_pixels,FALSE);CHECK((classic_pixels[0]&0xffffff)==0 && SendMessageW(w,PBM_GETPOS,0,0)==50);CHECK(DestroyWindow(w));
            w=CreateWindowW(PROGRESS_CLASSW,L"",WS_CHILD|PBS_SMOOTH,0,0,300,30,root,(HMENU)47,parent.hInstance,NULL);CHECK(w&&attached(w));
            SendMessageW(w,PBM_SETMARQUEE,FALSE,0);SetWindowLongPtrW(w,GWL_STYLE,GetWindowLongPtrW(w,GWL_STYLE)|PBS_MARQUEE);capture(w,classic_pixels,FALSE);
            CHECK(remove_style(w)&&!attached(w));capture(w,native_pixels,FALSE);CHECK(!memcmp(classic_pixels,native_pixels,sizeof(classic_pixels)));CHECK(DestroyWindow(w));
            puts("PROGRESS_BOUNDARY_OK tiny_zero_span=1 runtime_marquee_native=1");
            for(action=1;action<=3;++action){fired=FALSE;w=CreateWindowW(L"TProgressBar",L"",WS_CHILD|PBS_SMOOTH,0,0,300,30,root,(HMENU)44,parent.hInstance,NULL);CHECK(w&&attached(w));SendMessageW(w,PBM_SETBARCOLOR,0,RGB(31,127,219));SendMessageW(w,PBM_SETPOS,25,0);capture(w,classic_pixels,FALSE);CHECK(fired);
            if(action==1){CHECK(!attached(w));}
            if(action==2){CHECK(!IsWindow(w));}
            if(action==3){CHECK(SendMessageW(w,PBM_GETPOS,0,0)==75);capture(w,classic_pixels,FALSE);RECT bounds;GetClientRect(w,&bounds);int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));CHECK(count_color(classic_pixels,bounds,RGB(31,127,219))==(unsigned)((bounds.right-2*u)*75/100*(bounds.bottom-2*u)));}
            if(IsWindow(w))CHECK(DestroyWindow(w));}action=0;puts("PROGRESS_QUERY_RETIRE_OK remove=1 destroy=1 mutate=1");}
        if(!supported){emulate_state=TRUE;w=CreateWindowW(L"TProgressBar",L"",WS_CHILD|PBS_SMOOTH,0,0,300,30,root,(HMENU)48,parent.hInstance,NULL);CHECK(w&&!attached(w));CHECK(DestroyWindow(w));emulate_state=FALSE;puts("PROGRESS_V5_CAPABILITY_GUARD_OK emulated_state_not_enough=1");}
        CHECK(UnregisterClassW(L"TProgressBar",parent.hInstance));WNDCLASSW fake={.lpfnWndProc=DefWindowProcW,.hInstance=parent.hInstance,.lpszClassName=L"TProgressBar",.cbWndExtra=sizeof(void*)};CHECK(RegisterClassW(&fake));w=CreateWindowW(fake.lpszClassName,L"",WS_CHILD,0,0,300,30,root,(HMENU)45,parent.hInstance,NULL);CHECK(w&&!attached(w));CHECK(DestroyWindow(w));
    }
    printf("PROGRESS_CONTROL_OK bits=%u styled=%u aware=%u dpi=%u supported=%u native_fallback=%u\n",(unsigned)sizeof(void*)*8,styled,aware,GetDpiForWindow(root),supported,fallback);
    CHECK(DestroyWindow(root)&&SelectObject(dc,previous)&&DeleteObject(bitmap)&&DeleteDC(dc));return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned styled=0;styled<2;++styled){
        if(styled)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",argv[1],self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"native-aware":L"native");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        CHECK(AssignProcessToJobObject(job,pi.hProcess));ResumeThread(pi.hThread);DWORD started=GetTickCount(),code=1,wait=WaitForSingleObject(pi.hProcess,10000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        printf("PROGRESS_WORKER aware=%u styled=%u child_pid=%lu creator_pid=%lu elapsed_ms=%lu wait=%lu exit=%lu\n",aware,styled,pi.dwProcessId,GetCurrentProcessId(),GetTickCount()-started,wait,code);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    puts("PROGRESS_CLEANUP_OK");return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcsncmp(argv[2],L"styled",6),wcsstr(argv[2],L"aware")!=NULL);return isolated_process(argc,argv,worker);}
