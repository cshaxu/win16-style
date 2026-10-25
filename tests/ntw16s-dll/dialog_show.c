#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 180000
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DIALOG_SHOW_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
#define EXPECT(x) do{if(!(x)){fprintf(stderr,"DIALOG_SHOW_ASSERT line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);++failures;}}while(0)
static BOOL (__cdecl *attached)(HWND);
static BOOL (__cdecl *remove_style)(HWND);
static BOOL styled;
static unsigned kind,failures,observed,measured,drawn;
static HMENU menu;
static DWORD frame_image(HWND w)
{
    RECT r;GetWindowRect(w,&r);int width=r.right-r.left,height=r.bottom-r.top;
    HDC source=GetDC(w),dc=CreateCompatibleDC(source);BITMAPINFO info={0};DWORD *bits=NULL;
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=width;
    info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bitmap=dc?CreateDIBSection(dc,&info,DIB_RGB_COLORS,(void **)&bits,NULL,0):NULL;
    DWORD hash=2166136261u;
    if(!bitmap){++failures;if(dc)DeleteDC(dc);if(source)ReleaseDC(w,source);return 0;}
    HGDIOBJ old=SelectObject(dc,bitmap);ZeroMemory(bits,(size_t)width*height*4);
    EXPECT(PrintWindow(w,dc,0));GdiFlush();
    for(int y=0;y<min(height,26);++y)for(int x=0;x<width;++x)hash=(hash^bits[y*width+x])*16777619u;
    EXPECT(SelectObject(dc,old));EXPECT(DeleteObject(bitmap));EXPECT(DeleteDC(dc));EXPECT(ReleaseDC(w,source));return hash;
}
static void verify(HWND w)
{
    WCHAR caption[128];RECT client_before,client_after;HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    if(!IsWindowVisible(w))fprintf(stderr,"VISIBILITY_DIAG kind=%u styled=%u style=%08lx\n",kind,styled,(DWORD)GetWindowLongPtrW(w,GWL_STYLE));
    EXPECT(IsWindowVisible(w));EXPECT((attached && attached(w))==styled);
    GetWindowTextW(w,caption,128);EXPECT(!wcscmp(caption,L"Title formatted during initialization"));
    EXPECT(GetMenu(w)==menu);
    if(kind>=3){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_FTYPE|MIIM_DATA};
        EXPECT(GetMenuItemInfoW(menu,0,TRUE,&item) && (item.fType&MFT_OWNERDRAW) && item.dwItemData==0x7319);
        EXPECT(measured && drawn);}
    DWORD before=frame_image(w);GetClientRect(w,&client_before);
    if(styled){
        EXPECT(remove_style(w));EXPECT(!attached(w));EXPECT(frame_image(w)!=before);
        GetClientRect(w,&client_after);EXPECT(EqualRect(&client_before,&client_after));
        EXPECT((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font);
        GetWindowTextW(w,caption,128);EXPECT(!wcscmp(caption,L"Title formatted during initialization"));
    }
    ++observed;
    EndDialog(w,73);
}
static VOID CALLBACK timer(HWND w,UINT m,UINT_PTR id,DWORD now)
{(void)m;(void)now;KillTimer(w,id);ShowWindow(w,SW_SHOW);UpdateWindow(w);verify(w);}
static INT_PTR CALLBACK dialog(HWND w,UINT m,WPARAM a,LPARAM b)
{
    (void)a;(void)b;
    if(m==WM_MEASUREITEM){MEASUREITEMSTRUCT *item=(MEASUREITEMSTRUCT*)b;
        if(item->CtlType==ODT_MENU && item->itemData==0x7319){item->itemWidth=83;item->itemHeight=21;++measured;return TRUE;}}
    if(m==WM_DRAWITEM){DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT*)b;
        if(item->CtlType==ODT_MENU && item->itemData==0x7319){
            EXPECT(item->hwndItem==(HWND)menu && item->itemID==101 && item->itemAction==ODA_DRAWENTIRE);
            EXPECT(item->rcItem.right>item->rcItem.left && item->rcItem.bottom>item->rcItem.top);
            FillRect(item->hDC,&item->rcItem,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
            DrawTextW(item->hDC,L"Application menu",-1,&item->rcItem,DT_SINGLELINE|DT_VCENTER);
            ++drawn;return TRUE;}}
    if(m!=WM_INITDIALOG)return FALSE;
    EXPECT(!attached || !attached(w));
    SetWindowTextW(w,L"Title formatted during initialization");
    menu=CreateMenu();HMENU popup=CreatePopupMenu();
    EXPECT(menu && popup && AppendMenuW(popup,MF_STRING,101,L"Inspect") && AppendMenuW(menu,MF_POPUP,(UINT_PTR)popup,L"File"));
    if(kind>=3){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_FTYPE|MIIM_DATA|MIIM_ID,
            .fType=MFT_OWNERDRAW,.dwItemData=0x7319,.wID=101};
        EXPECT(SetMenuItemInfoW(menu,0,TRUE,&item));}
    EXPECT(SetMenu(w,menu));
    if(!(kind%3)){EXPECT(SetTimer(w,1,10,timer));return TRUE;}
    /* The private worker starts hidden. Consume that first-show override,
     * then exercise ShowWindow and the SWP_SHOWWINDOW path independently. */
    ShowWindow(w,SW_HIDE);
    if(kind%3==1)ShowWindow(w,SW_SHOW);
    else EXPECT(SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW));
    UpdateWindow(w);
    /* This call runs before WM_INITDIALOG returns, matching applications
     * which enter their own message loop from the initializer. */
    verify(w);return TRUE;
}
static int child(BOOL enable,BOOL aware)
{
    setvbuf(stdout,NULL,_IONBF,0);styled=enable;if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    attached=dll?(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
    remove_style=dll?(void*)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    CHECK(!styled || (attached && remove_style));
    struct {DLGTEMPLATE d;WORD menu,cls,title;} t={.d={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME,.cx=220,.cy=120}};
    HWND startup=CreateWindowW(L"STATIC",L"startup override",WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(startup);ShowWindow(startup,SW_HIDE);CHECK(DestroyWindow(startup));
    for(unsigned repeat=0;repeat<4;++repeat){
        for(unsigned ansi=0;ansi<(repeat?1u:2u);++ansi)for(kind=repeat?1u:0u;kind<(repeat?2u:6u);++kind){
            unsigned before=observed;INT_PTR result=ansi?
                DialogBoxIndirectParamA(GetModuleHandleW(NULL),&t.d,NULL,dialog,0):
                DialogBoxIndirectParamW(GetModuleHandleW(NULL),&t.d,NULL,dialog,0);
            CHECK(result==73 && observed==before+1 && !failures);
            CHECK(!IsMenu(menu));menu=NULL;
        }
        MSG queued;for(unsigned n=0;n<128 && PeekMessageW(&queued,NULL,0,0,PM_REMOVE);++n){TranslateMessage(&queued);DispatchMessageW(&queued);}
        GdiFlush();DWORD resources=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        printf("DIALOG_GDI styled=%u repeat=%u resources=%lu\n",styled,repeat,resources);
    }
    printf("DIALOG_SHOW_OK bits=%u styled=%u aware=%u cases=15 owner_draw_callbacks=1 initialization_active=1 initial_hidden_preserved=1 pixels=1 native_return=73 gdi_observed=1\n",(unsigned)(sizeof(void*)*8),styled,aware);return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned mode=0;mode<2;++mode){
        if(mode)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",argv[1],self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"bare-aware":L"bare");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};
        limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
        si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        if(!AssignProcessToJobObject(job,pi.hProcess)){TerminateProcess(pi.hProcess,1);CHECK(FALSE);}
        ResumeThread(pi.hThread);DWORD code=1,wait=WaitForSingleObject(pi.hProcess,60000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        if(wait!=WAIT_OBJECT_0)TerminateJobObject(job,ERROR_TIMEOUT);
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting={0};BOOL empty=FALSE;ULONGLONG deadline=GetTickCount64()+5000;
        do{CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL));
            if(!accounting.ActiveProcesses){empty=TRUE;break;}WaitForSingleObject(pi.hProcess,10);
        }while(GetTickCount64()<deadline);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code && empty);
    }
    puts("DIALOG_SHOW_CLEANUP_OK owned_jobs_empty=4");return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcsncmp(argv[2],L"styled",6),wcsstr(argv[2],L"aware")!=NULL);
    return isolated_process(argc,argv,worker);
}
