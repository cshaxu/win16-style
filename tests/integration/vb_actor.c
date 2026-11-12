/* Optional real VB6 application observer; paths are explicit arguments.
 * Own private desktop/Job, native API observations, no existing-process access. */
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include "../ntw16s-dll/isolated_process.h"
#include "../../src/lib-style/win16-control/control_glyphs.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ACTOR_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);ExitProcess(1);}}while(0)
static HANDLE job,ready,port;
#include "vb_process.h"
static HWND form;
static DWORD pid;
static HWND checks[4];
static unsigned count;
static void CALLBACK shown(HWINEVENTHOOK h,DWORD e,HWND w,LONG object,LONG child,DWORD thread,DWORD time){
    (void)h;
    (void)e;
    (void)thread;
    (void)time;
    WCHAR cls[80];
    if(object!=OBJID_WINDOW || child || !GetClassNameW(w,cls,80) || wcscmp(cls,L"ThunderRT6FormDC"))return;
    DWORD candidate=vb_process_member(w);
    if(candidate){
        form=w;
        pid=candidate;
        SetEvent(ready);
    }
}
static BOOL CALLBACK collect(HWND w,LPARAM ignored){
    (void)ignored;
    WCHAR cls[80];
    GetClassNameW(w,cls,80);
    if(!wcscmp(cls,L"ThunderRT6CheckBox") && count<4)checks[count++]=w;
    return TRUE;
}
static void capture_verify(HWND w,RECT r,int cx,int cy,void *data,BOOL verify){
    (void)r;
    if(verify){RECT client;CHECK(GetClientRect(w,&client));int dpi=(int)GetDpiForWindow(w);int top=(client.bottom-MulDiv(13,dpi,96))/2;CHECK(top>=0 && cx>=MulDiv(14,dpi,96) && cy>=top+MulDiv(13,dpi,96));
        LRESULT check=SendMessageW(w,BM_GETCHECK,0,0);CHECK(check==0 || check==1);DWORD *pixels=data;
        for(int y=0;y<13;++y)for(int x=0;x<14;++x){DWORD expected=control_checks[y][x+(check?14:0)]=='0'?0:0xffffff;CHECK((pixels[(top+MulDiv(y,dpi,96))*cx+MulDiv(x,dpi,96)]&0xffffff)==expected);}
        printf("ACTOR_GLYPH_OK source_pixels=182 check=%lld\n",(long long)check);
    }
}
#include "vb_capture.h"

static int worker(int argc,WCHAR **argv){
    CHECK(argc==4);
    BOOL styled=wcscmp(argv[1],L"bare")!=0;
    WCHAR path[MAX_PATH],scratch[MAX_PATH];
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    PROCESS_INFORMATION p={0};
    HWINEVENTHOOK hook=vb_process_start(argv,styled,shown,&p);
    HANDLE target=OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE,FALSE,pid);
    CHECK(target && WaitForInputIdle(target,5000)==0 && IsWindowVisible(form) && !IsIconic(form));
    vb_process_identity(target,pid,styled?p.dwProcessId:GetCurrentProcessId(),p.dwProcessId,form,"ACTOR",path);
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
    CHECK(snap!=INVALID_HANDLE_VALUE);
    MODULEENTRY32W mod= {.dwSize=sizeof(mod)};
    HMODULE runtime=NULL;
    BOOL dll=FALSE;
    CHECK(Module32FirstW(snap,&mod));
    do{
        if(!_wcsicmp(mod.szModule,L"MSVBVM60.DLL")){
            runtime=mod.hModule;
            printf("ACTOR_RUNTIME base=%p path=%ls\n",(void *)runtime,mod.szExePath);
            vb_process_file_path(mod.szExePath,"ACTOR");
        }
        if(!_wcsicmp(mod.szModule,L"ntw16s32.dll")){
            dll=TRUE;
            printf("ACTOR_STYLE_MODULE base=%p path=%ls\n",(void *)mod.hModule,mod.szExePath);
            vb_process_file_path(mod.szExePath,"ACTOR");
        }
    }
    while(Module32NextW(snap,&mod));
    CloseHandle(snap);
    CHECK(runtime && dll==styled && EnumChildWindows(form,collect,0) && count==2);
    CHECK(swprintf_s(path,MAX_PATH,L"%ls/actor-%ls.bmp",scratch,styled?L"styled":L"bare")>0);
    printf("ACTOR_IMAGE hash=%llu\n",(unsigned long long)capture(form,path,FALSE));
    for(unsigned i=0; i<count; ++i){
        HWND w=checks[i];
        CHECK((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==runtime);
        LRESULT original=SendMessageW(w,BM_GETCHECK,0,0);HFONT source_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);LONG_PTR source_style=GetWindowLongPtrW(w,GWL_STYLE);
        printf("ACTOR_CHECK index=%u hwnd=%p owner=%p style=%08lx dlgcode=%llx check=%lld state=%lld font=%p\n",i,(void *)w,(void *)GetClassLongPtrW(w,GCLP_HMODULE),(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(unsigned long long)SendMessageW(w,WM_GETDLGCODE,0,0),(long long)original,(long long)SendMessageW(w,BM_GETSTATE,0,0),(void *)SendMessageW(w,WM_GETFONT,0,0));
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/check-%ls-%u.bmp",scratch,styled?L"styled":L"bare",i)>0);
        printf("ACTOR_CHECK_IMAGE index=%u hash=%llu\n",i,(unsigned long long)capture(w,path,!wcscmp(argv[3],L"verify")));
        if(!wcscmp(argv[3],L"interact") || !wcscmp(argv[3],L"verify")){
            for(int j=0; j<4; ++j){
                SendMessageW(w,BM_CLICK,0,0);
                printf("ACTOR_CLICK index=%u round=%d check=%lld state=%lld\n",i,j,(long long)SendMessageW(w,BM_GETCHECK,0,0),(long long)SendMessageW(w,BM_GETSTATE,0,0));
            }
            CHECK(SendMessageW(w,BM_GETCHECK,0,0)==original && (HFONT)SendMessageW(w,WM_GETFONT,0,0)==source_font && GetWindowLongPtrW(w,GWL_STYLE)==source_style);
        }
    }
    SendMessageW(form,WM_CLOSE,0,0);
    vb_process_finish(target,&p,hook);
    printf("ACTOR_PASS styled=%d count=%u active=0\n",styled,count);
    return 0;
}
int wmain(int argc,WCHAR **argv){
    return isolated_process(argc,argv,worker);
}
