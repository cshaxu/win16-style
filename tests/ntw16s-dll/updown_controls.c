#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include <uxtheme.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "ntw16s-dll/attachment.h"
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UPDOWN_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static BOOL (__cdecl *attached)(HWND),(__cdecl *remove_style)(HWND);
static WNDPROC original;static HWND active,buddy;static unsigned mode,notices,action;static int last_delta;static BOOL inside,fired;
static DWORD native_bits[256*128],classic_bits[256*128],spare_bits[256*128];static HDC dc;static HBITMAP bitmap;static HGDIOBJ old;static DWORD *bits;
static void capture(HWND w,DWORD *out,BOOL frame)
{
    int save=SaveDC(dc);RECT r={0,0,256,128};FillRect(dc,&r,GetStockObject(WHITE_BRUSH));SendMessageW(w,frame?WM_PRINT:WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();CopyMemory(out,bits,sizeof(native_bits));RestoreDC(dc,save);
}
static LRESULT CALLBACK alias_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==UDM_GETBASE && action && attached && attached(w) && !fired){fired=TRUE;if(action==1)remove_style(w);else{DestroyWindow(w);return 0;}}
    return CallWindowProcW(original,w,m,p,l);
}
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_NOTIFY&&l){NMHDR *h=(NMHDR*)l;if(h->hwndFrom==active&&h->code==UDN_DELTAPOS){NMUPDOWN *d=(NMUPDOWN*)l;++notices;last_delta=d->iDelta;
        if(!inside){inside=TRUE;capture(active,spare_bits,FALSE);inside=FALSE;}
        if(mode==2){d->iDelta=-d->iDelta;}
        if(mode==3&&!fired){fired=TRUE;remove_style(active);}return mode==1;
    }}
    return DefWindowProcW(w,m,p,l);
}
static void save_image(HWND w,unsigned aware,unsigned horizontal,unsigned small)
{
    WCHAR dir[32768],file[32768];RECT r;GetClientRect(w,&r);if(!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",dir,32768))return;
    if(swprintf_s(file,32768,L"%ls/updown-%u-aware%u-h%u-small%u.bmp",dir,(unsigned)sizeof(void*)*8,aware,horizontal,small)<0)return;
    FILE *f=_wfopen(file,L"wb");if(!f)return;BITMAPFILEHEADER h={.bfType=0x4d42,.bfOffBits=54,.bfSize=54+(DWORD)(r.right*r.bottom*4)};BITMAPINFOHEADER i={.biSize=40,.biWidth=r.right,.biHeight=-r.bottom,.biPlanes=1,.biBitCount=32};fwrite(&h,sizeof(h),1,f);fwrite(&i,sizeof(i),1,f);
    for(int y=0;y<r.bottom;++y)fwrite(classic_bits+y*256,4,(size_t)r.right,f);
    fclose(f);
}
static int child(BOOL styled,BOOL aware)
{
    CHECK(ImmDisableIME(0));if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));INITCOMMONCONTROLSEX cc={sizeof(cc),ICC_UPDOWN_CLASS};CHECK(InitCommonControlsEx(&cc));
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(styled?dll!=NULL:dll==NULL);attached=dll?(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;remove_style=dll?(void*)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"UpDownProtocolParent"};CHECK(RegisterClassW(&cls));HWND root=CreateWindowW(cls.lpszClassName,L"UpDown protocol",WS_OVERLAPPEDWINDOW,0,0,640,300,NULL,NULL,cls.hInstance,NULL);CHECK(root);ShowWindow(root,SW_SHOW);ShowWindow(root,SW_SHOW);
    WNDCLASSEXW alias={.cbSize=sizeof(alias)};CHECK(GetClassInfoExW(NULL,UPDOWN_CLASSW,&alias));original=alias.lpfnWndProc;alias.hInstance=cls.hInstance;alias.lpszClassName=L"TUpDown";alias.lpfnWndProc=alias_proc;alias.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&alias));
    if(styled){WCHAR path[32768];CHECK(attached&&remove_style&&attached(root)&&GetModuleFileNameW(dll,path,32768));printf("UPDOWN_MODULE bits=%u pid=%lu path=%ls bootstrap_frame_attached=1\n",(unsigned)sizeof(void*)*8,GetCurrentProcessId(),path);}
    dc=CreateCompatibleDC(NULL);BITMAPINFO bi={0};bi.bmiHeader.biSize=40;bi.bmiHeader.biWidth=256;bi.bmiHeader.biHeight=-128;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bitmap=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,(void**)&bits,NULL,0);CHECK(dc&&bitmap);old=SelectObject(dc,bitmap);
    BYTE saved[256],keys[256]={0},restored[256];CHECK(GetKeyboardState(saved));const WCHAR *names[]={UPDOWN_CLASSW,L"TUpDown"};unsigned variants=0;
    buddy=CreateWindowW(L"EDIT",L"0",WS_CHILD|WS_VISIBLE|WS_BORDER,100,80,100,24,root,(HMENU)71,cls.hInstance,NULL);CHECK(buddy);
    for(unsigned kind=0;kind<2;++kind)for(unsigned horizontal=0;horizontal<2;++horizontal)for(unsigned small=0;small<2;++small){
        HWND w=CreateWindowW(names[kind],L"",WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ARROWKEYS|(horizontal?UDS_HORZ:0),0,0,horizontal?(small?34:64):17,horizontal?(small?17:24):(small?20:40),root,(HMENU)72,cls.hInstance,NULL);CHECK(w);active=w;if(styled)CHECK(attached(w));
        SendMessageW(w,UDM_SETBUDDY,(WPARAM)buddy,0);SendMessageW(w,UDM_SETRANGE32,(WPARAM)(INT_PTR)-5,5);SendMessageW(w,UDM_SETPOS32,0,0);RECT before,after;GetClientRect(w,&before);HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);mode=0;
        capture(w,classic_bits,FALSE);capture(w,native_bits,TRUE);CHECK(!memcmp(classic_bits,native_bits,sizeof(native_bits)));
        int unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));if(styled){CHECK((classic_bits[0]&0xffffff)==0 && (classic_bits[unit*256+unit]&0xffffff)==0xffffff);if(!kind)save_image(w,aware,horizontal,small);}
        RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);HDC visible=GetDC(w);CHECK(visible);if(styled)CHECK(GetPixel(visible,0,0)==0);ReleaseDC(w,visible);
        for(mode=0;mode<3;++mode){
            SendMessageW(w,UDM_SETPOS32,0,0);unsigned prev=notices;keys[VK_LBUTTON]=0x80;CHECK(SetKeyboardState(keys));LPARAM point=MAKELPARAM(before.right/4,before.bottom/4);PostMessageW(w,WM_LBUTTONUP,0,point);SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,point);CHECK(notices==prev+1&&GetCapture()==w);CHECK(SendMessageW(w,UDM_GETPOS32,0,0)==(mode==1?0:mode==2?-last_delta:last_delta));capture(w,classic_bits,FALSE);
            if(styled)CHECK((classic_bits[unit*256+unit]&0xffffff)==0x808080);
            SendMessageW(w,WM_LBUTTONUP,0,point);CHECK(GetCapture()!=w);MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE))DispatchMessageW(&msg);keys[VK_LBUTTON]=0;CHECK(SetKeyboardState(keys));
            SendMessageW(w,UDM_SETPOS32,0,0);SetFocus(buddy);keys[VK_UP]=0x80;CHECK(SetKeyboardState(keys));prev=notices;SendMessageW(buddy,WM_KEYDOWN,VK_UP,1);CHECK(notices==prev+1&&GetCapture()!=w);CHECK(SendMessageW(w,UDM_GETPOS32,0,0)==(mode==1?0:mode==2?-last_delta:last_delta));capture(w,classic_bits,FALSE);
            if(styled){int x=horizontal?before.right/2+unit:unit;CHECK((classic_bits[unit*256+x]&0xffffff)==0x808080);}
            SendMessageW(buddy,WM_KEYUP,VK_UP,1);keys[VK_UP]=0;CHECK(SetKeyboardState(keys));capture(w,classic_bits,FALSE);if(styled)CHECK((classic_bits[unit*256+unit]&0xffffff)==0xffffff);
        }mode=0;
        /* Native cancellation can release capture while keeping its pressed
         * visual. The renderer must follow the API's real state, not capture. */
        SendMessageW(w,UDM_SETPOS32,0,0);keys[VK_LBUTTON]=0x80;CHECK(SetKeyboardState(keys));
        SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(before.right/4,before.bottom/4));CHECK(GetCapture()==w);
        SendMessageW(w,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(-10,-10));SendMessageW(w,WM_CANCELMODE,0,0);CHECK(GetCapture()!=w);capture(w,classic_bits,FALSE);
        if(styled)CHECK((classic_bits[unit*256+unit]&0xffffff)==0x808080);
        SendMessageW(w,WM_LBUTTONUP,0,0);keys[VK_LBUTTON]=0;CHECK(SetKeyboardState(keys));
        SendMessageW(buddy,WM_KEYDOWN,VK_UP,1);SendMessageW(buddy,WM_KEYUP,VK_UP,1);capture(w,classic_bits,FALSE);
        if(styled)CHECK((classic_bits[unit*256+unit]&0xffffff)==0xffffff);
        SendMessageW(w,UDM_SETRANGE32,5,-5);SendMessageW(w,UDM_SETPOS32,0,0);SendMessageW(buddy,WM_KEYDOWN,VK_UP,1);CHECK(SendMessageW(w,UDM_GETPOS32,0,0)==-1);SendMessageW(buddy,WM_KEYUP,VK_UP,1);
        SendMessageW(w,UDM_SETRANGE32,(WPARAM)(INT_PTR)-5,5);SendMessageW(w,UDM_SETPOS32,0,0);EnableWindow(w,FALSE);capture(w,classic_bits,FALSE);CHECK(!IsWindowEnabled(w));EnableWindow(w,TRUE);
        SendMessageW(w,UDM_SETBASE,16,0);CHECK(SendMessageW(w,UDM_GETBASE,0,0)==16);SendMessageW(w,UDM_SETBASE,10,0);capture(w,classic_bits,FALSE);
        HWND unowned=NULL;
        if(styled && !kind && !horizontal && !small){
            unowned=CreateWindowW(UPDOWN_CLASSW,L"",WS_CHILD|WS_VISIBLE,100,0,17,40,root,(HMENU)79,cls.hInstance,NULL);CHECK(unowned&&attached(unowned)&&remove_style(unowned));
            CHECK(GetWindowTheme(w)==GetWindowTheme(unowned));capture(unowned,native_bits,FALSE);CopyMemory(spare_bits,native_bits,sizeof(native_bits));
        }
        if(styled){CHECK(remove_style(w)&&!attached(w));}
        if(unowned){capture(unowned,native_bits,FALSE);CHECK(!memcmp(native_bits,spare_bits,sizeof(native_bits))&&DestroyWindow(unowned));puts("UPDOWN_SHARED_THEME_OK unowned_native_with_owned_present=1");}
        capture(w,native_bits,FALSE);unsigned diff=0;for(int y=0;y<before.bottom;++y)for(int x=0;x<before.right;++x)diff+=(classic_bits[y*256+x]&0xffffff)!=(native_bits[y*256+x]&0xffffff);CHECK(styled?diff>10:!diff);
        GetClientRect(w,&after);int lo=0,hi=0;SendMessageW(w,UDM_GETRANGE32,(WPARAM)&lo,(LPARAM)&hi);CHECK(EqualRect(&before,&after)&&lo==-5&&hi==5&&SendMessageW(w,UDM_GETPOS32,0,0)==0&&(HWND)SendMessageW(w,UDM_GETBUDDY,0,0)==buddy&&(HFONT)SendMessageW(w,WM_GETFONT,0,0)==font);
        printf("UPDOWN_VARIANT kind=%u horizontal=%u small=%u styled=%u pixels=%u native_input_udn_cancel_delta_geometry_font=1\n",kind,horizontal,small,styled,diff);CHECK(DestroyWindow(w));active=NULL;++variants;
    }
    if(styled){
        mode=3;fired=FALSE;active=CreateWindowW(UPDOWN_CLASSW,L"",WS_CHILD|UDS_SETBUDDYINT,0,0,17,40,root,(HMENU)73,cls.hInstance,NULL);CHECK(active&&attached(active));SendMessageW(active,UDM_SETBUDDY,(WPARAM)buddy,0);SendMessageW(active,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(8,8));CHECK(fired&&!attached(active));SendMessageW(active,WM_LBUTTONUP,0,0);CHECK(DestroyWindow(active));active=NULL;mode=0;
        for(action=1;action<=2;++action){fired=FALSE;HWND w=CreateWindowW(L"TUpDown",L"",WS_CHILD,0,0,17,40,root,(HMENU)74,cls.hInstance,NULL);CHECK(w&&attached(w));SendMessageW(w,UDM_GETBASE,0,0);CHECK(fired);CHECK(action==1?!attached(w):!IsWindow(w));if(IsWindow(w))CHECK(DestroyWindow(w));}action=0;
        CHECK(UnregisterClassW(L"TUpDown",cls.hInstance));WNDCLASSW fake={.lpfnWndProc=DefWindowProcW,.hInstance=cls.hInstance,.lpszClassName=L"TUpDown",.cbWndExtra=sizeof(void*)};CHECK(RegisterClassW(&fake));HWND w=CreateWindowW(fake.lpszClassName,L"",WS_CHILD,0,0,17,40,root,(HMENU)75,cls.hInstance,NULL);CHECK(w&&!attached(w));CHECK(DestroyWindow(w));puts("UPDOWN_RETIRE_OK notify_remove=1 query_remove_destroy=1 fake_rejected=1");
    }
    CHECK(SetKeyboardState(saved)&&GetKeyboardState(restored)&&!memcmp(saved,restored,sizeof(saved)));CHECK(SetKeyboardState(keys)&&GdiFlush());DWORD gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    for(unsigned i=0;i<64;++i){HWND w=CreateWindowW(UPDOWN_CLASSW,L"",WS_CHILD|WS_VISIBLE|UDS_ARROWKEYS|UDS_SETBUDDYINT,0,0,17,40,root,(HMENU)76,cls.hInstance,NULL);CHECK(w);SendMessageW(w,UDM_SETBUDDY,(WPARAM)buddy,0);SendMessageW(w,UDM_SETPOS32,0,0);SendMessageW(buddy,WM_KEYDOWN,VK_UP,1);SendMessageW(buddy,WM_KEYUP,VK_UP,1);RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);if(styled){HDC visible=GetDC(w);CHECK(visible&&GetPixel(visible,0,0)==0);ReleaseDC(w,visible);}CHECK(DestroyWindow(w)&&!IsWindow(w));}
    CHECK(SetKeyboardState(saved)&&GetKeyboardState(restored)&&!memcmp(saved,restored,sizeof(saved)));DWORD raw_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(GdiFlush());DWORD ga=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),ua=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);printf("UPDOWN_RESOURCE_DIAG styled=%u gdi_before=%lu raw_after=%lu flushed_after=%lu user=%lu,%lu\n",styled,gdi,raw_gdi,ga,user,ua);printf("UPDOWN_RESOURCE_OBSERVED cycles=64 gdi=%lu,%lu user=%lu,%lu\n",gdi,ga,user,ua);
    if(!styled){WCHAR path[MAX_PATH];CHECK(GetEnvironmentVariableW(L"WIN16STYLE_UP_PASSIVE_DLL",path,MAX_PATH));HMODULE passive=LoadLibraryW(path);CHECK(passive);ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void*)GetProcAddress(passive,"Ntw16sCreationBegin");BOOL (__cdecl *end)(ntw16s_creation_scope*,ntw16s_creation_result*)=(void*)GetProcAddress(passive,"Ntw16sCreationEnd");BOOL (__cdecl *is_attached)(HWND)=(void*)GetProcAddress(passive,"Ntw16sWindowAttached");CHECK(begin&&end&&is_attached);ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);HWND w=CreateWindowW(UPDOWN_CLASSW,L"",WS_CHILD,0,0,17,40,root,(HMENU)77,cls.hInstance,NULL);ntw16s_creation_result result={0};CHECK(w&&end(scope,&result)&&!result.failure&&result.unsupported&&!is_attached(w));capture(w,classic_bits,FALSE);CHECK(DestroyWindow(w));w=CreateWindowW(UPDOWN_CLASSW,L"",WS_CHILD,0,0,17,40,root,(HMENU)78,cls.hInstance,NULL);CHECK(w);capture(w,native_bits,FALSE);CHECK(!memcmp(classic_bits,native_bits,sizeof(native_bits))&&DestroyWindow(w)&&FreeLibrary(passive));puts("UPDOWN_PASSIVE_OK loaded=1 api_init=0 unattached=1 native_pixels=1");}
    printf("UPDOWN_CONTROL_OK bits=%u styled=%u aware=%u dpi=%u variants=%u\n",(unsigned)sizeof(void*)*8,styled,aware,GetDpiForWindow(root),variants);CHECK(DestroyWindow(buddy)&&DestroyWindow(root)&&SelectObject(dc,old)&&DeleteObject(bitmap)&&DeleteDC(dc));return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048],dll[MAX_PATH];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    CHECK(GetFullPathNameW(argv[1],MAX_PATH,dll,NULL));WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)slash=wcsrchr(dll,L'/');CHECK(slash);slash[1]=0;
    CHECK(!wcscat_s(dll,MAX_PATH,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll"));CHECK(SetEnvironmentVariableW(L"WIN16STYLE_UP_PASSIVE_DLL",dll));
    for(unsigned aware=0;aware<2;++aware)for(unsigned styled=0;styled<2;++styled){
        if(styled)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",argv[1],self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"native-aware":L"native");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        CHECK(AssignProcessToJobObject(job,pi.hProcess));ResumeThread(pi.hThread);DWORD started=GetTickCount(),code=1,wait=WaitForSingleObject(pi.hProcess,10000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        printf("UPDOWN_WORKER aware=%u styled=%u child_pid=%lu creator_pid=%lu elapsed_ms=%lu wait=%lu exit=%lu\n",aware,styled,pi.dwProcessId,GetCurrentProcessId(),GetTickCount()-started,wait,code);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    puts("UPDOWN_CLEANUP_OK");return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcsncmp(argv[2],L"styled",6),wcsstr(argv[2],L"aware")!=NULL);return isolated_process(argc,argv,worker);}
