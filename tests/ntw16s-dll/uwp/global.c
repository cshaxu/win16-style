#define COBJMACROS
#include "ntw16s-dll/global/manager.h"
#include "ntw16s-dll/uwp/target.h"
#include "ntw16s-dll/global/session.h"
#include <shobjidl.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include <dwmapi.h>
static BOOL capture(HWND window)
{
    WCHAR path[32768];
    if(!GetEnvironmentVariableW(L"UWP_GLOBAL_CAPTURE",path,ARRAYSIZE(path)))return TRUE;
    /* Let the application finish its visible activation animation; this delay
       is screenshot-only and is not part of the attachment success check. */
    ULONGLONG settle=GetTickCount64()+1000;
    while(GetTickCount64()<settle){
        MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
    RECT r;DWORD cloaked=1;
    if(IsIconic(window)||!IsWindowVisible(window)||!GetWindowRect(window,&r)||
       FAILED(DwmGetWindowAttribute(window,DWMWA_CLOAKED,&cloaked,sizeof(cloaked)))||cloaked)return FALSE;
    int width=r.right-r.left,height=r.bottom-r.top;
    if(width<=0||height<=0||width>8192||height>8192)return FALSE;
    HDC screen=GetDC(NULL),dc=screen?CreateCompatibleDC(screen):NULL;
    BITMAPINFO info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=width,
        .biHeight=-height,.biPlanes=1,.biBitCount=32}};
    void *bits=NULL;HBITMAP bitmap=dc?CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0):NULL;
    BOOL ok=FALSE,topmost=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOPMOST)!=0;
    SetWindowPos(window,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    if(bitmap){
        HGDIOBJ old=SelectObject(dc,bitmap);DwmFlush();
        if(BitBlt(dc,0,0,width,height,screen,r.left,r.top,SRCCOPY|CAPTUREBLT)){
            GdiFlush();FILE *file=_wfopen(path,L"wb");
            if(file){
                DWORD size=(DWORD)width*(DWORD)height*4;
                BITMAPFILEHEADER header={.bfType=0x4d42,.bfSize=sizeof(header)+sizeof(info.bmiHeader)+size,
                    .bfOffBits=sizeof(header)+sizeof(info.bmiHeader)};
                ok=fwrite(&header,sizeof(header),1,file)==1&&fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,file)==1&&fwrite(bits,size,1,file)==1;
                fclose(file);
            }
        }
        SelectObject(dc,old);DeleteObject(bitmap);
    }
    if(dc)DeleteDC(dc);
    if(screen)ReleaseDC(NULL,screen);
    if(!topmost)SetWindowPos(window,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    printf("CAPTURE ok=%d dpi=%u rect=%ld,%ld,%ld,%ld\n",ok,GetDpiForWindow(window),r.left,r.top,r.right,r.bottom);
    return ok;
}
static HWND frame,surface;
static unsigned surfaces;
static DWORD observed_pid;
static void CALLBACK events(HWINEVENTHOOK hook,DWORD event,HWND window,LONG object,LONG child,DWORD thread,DWORD time)
{
    (void)hook;(void)thread;
    if(object!=OBJID_WINDOW||child!=CHILDID_SELF||!observed_pid)return;
    if(event!=EVENT_OBJECT_SHOW&&event!=EVENT_OBJECT_PARENTCHANGE&&event!=EVENT_OBJECT_UNCLOAKED)return;
    DWORD pid=0;GetWindowThreadProcessId(window,&pid);ntw16s_uwp_target target;
    if(pid!=observed_pid&&(!ntw16s_uwp_target_window(window,&target)||target.pid!=observed_pid))return;
    WCHAR cls[80]={0};GetClassNameW(window,cls,ARRAYSIZE(cls));
    printf("WINDOW_EVENT event=%lx time=%lu pid=%lu hwnd=%p root=%p visible=%d class=%ls\n",
        (unsigned long)event,time,pid,(void*)window,(void*)GetAncestor(window,GA_ROOT),IsWindowVisible(window),cls);
}
static BOOL CALLBACK owned(HWND w,LPARAM unused)
{
    (void)unused;
    if(frame&&GetWindow(w,GW_OWNER)==frame&&(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_LAYERED)){
        surface=w;++surfaces;
    }
    return TRUE;
}
static BOOL existing(PCWSTR image)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snapshot==INVALID_HANDLE_VALUE)return TRUE;
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};BOOL found=FALSE;
    for(BOOL n=Process32FirstW(snapshot,&entry);n;n=Process32NextW(snapshot,&entry))
        if(!_wcsicmp(entry.szExeFile,image))found=TRUE;
    CloseHandle(snapshot);return found;
}
int wmain(int argc,WCHAR **argv)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    BOOL new_target=argc==4&&!wcscmp(argv[3],L"new");
    BOOL prestyled=argc==6&&!wcscmp(argv[3],L"pair-styled");
    BOOL pair=prestyled||(argc==6&&!wcscmp(argv[3],L"pair"));
    if((argc!=3&&!new_target&&!pair)||sizeof(void*)!=8){puts("SKIP explicit AUMID/image and x64 host required");return 77;}
    if(existing(argv[2])||(pair&&existing(argv[5]))){puts("SKIP existing application");return 77;}
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],dll[32768];DWORD n=GetModuleFileNameW(NULL,dll,ARRAYSIZE(dll));
    WCHAR *tail=wcsrchr(dll,L'\\');if(!n||n>=ARRAYSIZE(dll)||!tail||(size_t)(tail-dll)+20>=ARRAYSIZE(dll))return 1;
    wcscpy(tail+1,L"ntw16s64.dll");
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return 1;
    if(swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0)return 1;
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!mapping)return 1;
    if(GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(mapping);puts("SKIP existing global session");return 77;}
    ntw16s_global_session *s=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*s));
    HANDLE stop=CreateEventW(NULL,TRUE,FALSE,stop_name),application=NULL,second_application=NULL;
    ntw16s_uwp_target second_target={0};HWND second_surface=NULL;
    ntw16s_global_manager *global=NULL;ntw16s_uwp_target target={0};BOOL ok=FALSE;DWORD error=0;
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);BOOL com=SUCCEEDED(hr);
    HWINEVENTHOOK event_hook=SetWinEventHook(EVENT_OBJECT_CREATE,EVENT_OBJECT_UNCLOAKED,NULL,events,0,0,WINEVENT_OUTOFCONTEXT);
    if(!s||!stop||!com)goto done;
    *s=(ntw16s_global_session){.magic=NTW16S_GLOBAL_MAGIC,.version=NTW16S_GLOBAL_VERSION,.owner_pid=GetCurrentProcessId(),.cookie=GetCurrentProcessId(),.phase=NTW16S_GLOBAL_ACTIVE};
    if(!win32_process_global_birth(GetCurrentProcess(),&s->owner_birth_low,&s->owner_birth_high))goto done;
    if(new_target){
        ntw16s_global_manager_test_filter(MAXDWORD);
        error=ntw16s_global_manager_start(dll,IMAGE_FILE_MACHINE_AMD64,&global);
        if(error)goto done;
    }
    FILETIME start;GetSystemTimeAsFileTime(&start);DWORD pid=0;IApplicationActivationManager *manager=NULL;
    hr=CoCreateInstance(&CLSID_ApplicationActivationManager,NULL,CLSCTX_LOCAL_SERVER,&IID_IApplicationActivationManager,(void**)&manager);
    if(SUCCEEDED(hr)){CoAllowSetForegroundWindow((IUnknown*)manager,NULL);hr=IApplicationActivationManager_ActivateApplication(manager,argv[1],NULL,AO_NOERRORUI,&pid);IApplicationActivationManager_Release(manager);}
    printf("ACTIVATE hr=%lx pid=%lu\n",(unsigned long)hr,pid);if(FAILED(hr))goto done;
    observed_pid=pid;
    ntw16s_global_manager_test_filter(pid);
    application=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE|PROCESS_TERMINATE,FALSE,pid);
    FILETIME birth,a,b,c;
    if(!application||!GetProcessTimes(application,&birth,&a,&b,&c)||CompareFileTime(&birth,&start)<0){if(application)CloseHandle(application);application=NULL;goto done;}
    ULONGLONG deadline=GetTickCount64()+10000;
    while(!ntw16s_uwp_target_find(pid,&target)&&GetTickCount64()<deadline){
        MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
    frame=target.frame;EnumWindows(owned,0);
    printf("BEFORE frame=%p core=%p surfaces=%u\n",(void*)frame,(void*)target.core,surfaces);
    if(!frame||(!new_target&&surfaces))goto done;
    if(pair){
        GetSystemTimeAsFileTime(&start);DWORD second_pid=0;
        manager=NULL;hr=CoCreateInstance(&CLSID_ApplicationActivationManager,NULL,CLSCTX_LOCAL_SERVER,&IID_IApplicationActivationManager,(void**)&manager);
        if(SUCCEEDED(hr)){hr=IApplicationActivationManager_ActivateApplication(manager,argv[4],NULL,AO_NOERRORUI,&second_pid);IApplicationActivationManager_Release(manager);}
        if(FAILED(hr))goto done;
        second_application=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE|PROCESS_TERMINATE,FALSE,second_pid);
        if(!second_application||!GetProcessTimes(second_application,&birth,&a,&b,&c)||CompareFileTime(&birth,&start)<0){if(second_application)CloseHandle(second_application);second_application=NULL;goto done;}
        ntw16s_global_manager_test_second(second_pid);
        deadline=GetTickCount64()+10000;
        while(!ntw16s_uwp_target_find(second_pid,&second_target)&&GetTickCount64()<deadline){
            MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
            MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
        }
        DWORD host1=0,host2=0;GetWindowThreadProcessId(frame,&host1);GetWindowThreadProcessId(second_target.frame,&host2);
        printf("PAIR_TARGET first=%lu second=%lu host1=%lu host2=%lu\n",pid,second_pid,host1,host2);
        if(!second_target.frame||!host1||host1!=host2||frame==second_target.frame)goto done;
        if(prestyled){error=ntw16s_uwp_target_change(&target,dll,TRUE);printf("PAIR_PRESTYLE error=%lu\n",error);if(error)goto done;}
    }
    if(!new_target){error=ntw16s_global_manager_start(dll,IMAGE_FILE_MACHINE_AMD64,&global);if(error)goto done;}
    BOOL admitted=new_target||!ntw16s_global_manager_refresh(global);
    if(new_target){
        deadline=GetTickCount64()+7000;
        do{
            MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
            MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
            surfaces=0;surface=NULL;EnumWindows(owned,0);
            if(surface&&IsWindowVisible(surface))break;
        }while(GetTickCount64()<deadline);
    }
    error=ntw16s_global_manager_wait_idle(global,7000);
    surfaces=0;surface=NULL;EnumWindows(owned,0);
    ok=!error&&admitted&&surfaces==1&&surface&&IsWindowVisible(surface);
    printf("GLOBAL_APPLY admitted=%d idle_error=%lu surfaces=%u visible=%d\n",admitted,error,surfaces,surface?IsWindowVisible(surface):0);
    ntw16s_global_manager_stats stats;ntw16s_global_manager_stats_read(global,&stats);
    printf("GLOBAL_STATS queued=%ld active=%ld failed=%ld\n",stats.queued,stats.active,stats.failed);
    if(ok)ok=capture(frame);
    if(ok&&pair){
        HWND first_frame=frame,first_surface=surface;frame=second_target.frame;surface=NULL;surfaces=0;EnumWindows(owned,0);
        second_surface=surface;ok=surfaces==1&&second_surface&&IsWindowVisible(second_surface)&&IsWindowVisible(first_surface);
        printf("PAIR_APPLY both_visible=%d distinct_surfaces=%d\n",ok,second_surface!=first_surface);
        ok=ok&&second_surface!=first_surface;frame=first_frame;surface=first_surface;
    }
done:
    if(s)s->phase=NTW16S_GLOBAL_STOPPING;
    if(global){DWORD removed=ntw16s_global_manager_stop(global,5000);printf("GLOBAL_REMOVE error=%lu surface_exists=%d\n",removed,IsWindow(surface));ok=ok&&!removed&&!IsWindow(surface);if(removed)error=removed;else global=NULL;}
    if(pair){printf("PAIR_REMOVE second_exists=%d\n",IsWindow(second_surface));ok=ok&&!IsWindow(second_surface);}
    if(ok){
        s->phase=NTW16S_GLOBAL_ACTIVE;
        error=ntw16s_global_manager_start(dll,IMAGE_FILE_MACHINE_AMD64,&global);
        if(!error)ok=!ntw16s_global_manager_refresh(global);
        s->phase=NTW16S_GLOBAL_STOPPING;
        if(global){DWORD removed=ntw16s_global_manager_stop(global,5000);if(removed)error=removed;else global=NULL;}
        surfaces=0;surface=NULL;EnumWindows(owned,0);ok=ok&&!error&&!surfaces;
        printf("GLOBAL_STOP_RACE error=%lu surfaces=%u\n",error,surfaces);
        if(pair){HWND first_frame=frame;frame=second_target.frame;surfaces=0;surface=NULL;EnumWindows(owned,0);
            printf("PAIR_STOP_RACE second_surfaces=%u\n",surfaces);ok=ok&&!surfaces;frame=first_frame;}
    }
    if(application){if(frame)PostMessageW(frame,WM_CLOSE,0,0);if(WaitForSingleObject(application,3000)!=WAIT_OBJECT_0){TerminateProcess(application,0);WaitForSingleObject(application,1000);}CloseHandle(application);}
    if(second_application){if(second_target.frame)PostMessageW(second_target.frame,WM_CLOSE,0,0);if(WaitForSingleObject(second_application,3000)!=WAIT_OBJECT_0){TerminateProcess(second_application,0);WaitForSingleObject(second_application,1000);}CloseHandle(second_application);}
    if(global){DWORD removed=ntw16s_global_manager_stop(global,5000);if(removed){printf("FAIL retained worker error=%lu\n",removed);return 1;}}
    if(com)CoUninitialize();
    if(event_hook)UnhookWinEvent(event_hook);
    if(s)UnmapViewOfFile(s);
    CloseHandle(mapping);
    if(stop)CloseHandle(stop);
    printf("UWP_GLOBAL_%s error=%lu\n",ok?"PASS":"FAIL",error);return ok?0:1;
}
