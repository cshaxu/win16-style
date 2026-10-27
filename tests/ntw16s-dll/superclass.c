#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SUPER FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static unsigned clicks;
static void settle(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}GdiFlush();}
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_COMMAND && HIWORD(a)==BN_CLICKED)++clicks;return DefWindowProcW(w,m,a,b);}
typedef struct capture_surface {HDC dc;HBITMAP bitmap;HGDIOBJ old;DWORD *bits;} capture_surface;
static capture_surface surface;
static void capture_end(void)
{
    if(!surface.dc)return;
    if(!SelectObject(surface.dc,surface.old) || !DeleteObject(surface.bitmap) || !DeleteDC(surface.dc)){
        fprintf(stderr,"SUPER_CAPTURE_CLEANUP_FAIL error=%lu\n",GetLastError());ExitProcess(1);
    }
    ZeroMemory(&surface,sizeof(surface));GdiFlush();
}
static DWORD image(HWND w,DWORD *pixels,UINT count)
{
    if(!surface.dc){
        BITMAPINFO info={0};HDC dc=GetDC(w);surface.dc=CreateCompatibleDC(dc);ReleaseDC(w,dc);
        info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=200;info.bmiHeader.biHeight=-100;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
        surface.bitmap=CreateDIBSection(surface.dc,&info,DIB_RGB_COLORS,(void **)&surface.bits,NULL,0);
        if(!surface.dc || !surface.bitmap)ExitProcess(1);
        surface.old=SelectObject(surface.dc,surface.bitmap);
    }
    int saved=SaveDC(surface.dc);if(!saved)ExitProcess(1);
    RECT clear={0,0,200,100};FillRect(surface.dc,&clear,GetStockObject(WHITE_BRUSH));
    WCHAR cls[64];GetClassNameW(w,cls,64);
    if(!wcscmp(cls,L"TEdit") || !wcscmp(cls,L"TMemo") || !wcscmp(cls,L"TMaskEdit"))SendMessageW(w,WM_PRINT,(WPARAM)surface.dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);
    else SendMessageW(w,WM_PRINTCLIENT,(WPARAM)surface.dc,PRF_CLIENT);
    GdiFlush();DWORD hash=2166136261u;
    for(UINT i=0;i<200*100;++i)hash=(hash^surface.bits[i])*16777619u;
    if(pixels && count==200*100)CopyMemory(pixels,surface.bits,count*sizeof(DWORD));
    if(!RestoreDC(surface.dc,saved))ExitProcess(1);
    return hash;
}

static WNDPROC native_procs[12];
static const WCHAR *class_names[]={L"RenamedPush",L"RenamedCheck",L"TEdit",L"TComboBox",L"TListBox",L"RenamedStatic",L"RenamedScroll",L"TCheckBox",L"TRadioButton",L"TGroupBox",L"TMemo",L"TMaskEdit"};
/* Like a framework InitWndProc, registration itself is custom. The system
 * instance type is established by calling the saved native procedure. */
static LRESULT CALLBACK framework_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    WCHAR cls[64];GetClassNameW(w,cls,64);
    for(unsigned i=0;i<ARRAYSIZE(class_names);++i)if(!wcscmp(cls,class_names[i]))return CallWindowProcW(native_procs[i],w,m,a,b);
    return DefWindowProcW(w,m,a,b);
}
static LRESULT CALLBACK initializer_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)framework_proc);
    return framework_proc(w,m,a,b);
}
static BOOL superclass(const WCHAR *base,const WCHAR *name)
{
    WNDCLASSEXW c={.cbSize=sizeof(c)};
    if(!GetClassInfoExW(NULL,base,&c))return FALSE;
    for(unsigned i=0;i<ARRAYSIZE(class_names);++i)if(!wcscmp(name,class_names[i]))native_procs[i]=c.lpfnWndProc;
    c.lpfnWndProc=initializer_proc;c.hInstance=GetModuleHandleW(NULL);c.lpszClassName=name;c.style&=~CS_GLOBALCLASS;
    return RegisterClassExW(&c)!=0;
}
static int child(BOOL styled,BOOL aware)
{
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HINSTANCE instance=GetModuleHandleW(NULL);
    HMODULE dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
    BOOL (__cdecl *remove_control)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    CHECK(styled?(dll && attached && remove_control):!dll);
    const WCHAR *bases[]={L"BUTTON",L"BUTTON",L"EDIT",L"COMBOBOX",L"LISTBOX",L"STATIC",L"SCROLLBAR"};
    const WCHAR *names[]={L"RenamedPush",L"RenamedCheck",L"TEdit",L"TComboBox",L"TListBox",L"RenamedStatic",L"RenamedScroll"};
    for(unsigned i=0;i<ARRAYSIZE(bases);++i)CHECK(superclass(bases[i],names[i]));
    for(unsigned i=7;i<10;++i)CHECK(superclass(L"BUTTON",class_names[i]));
    for(unsigned i=10;i<ARRAYSIZE(class_names);++i)CHECK(superclass(L"EDIT",class_names[i]));
    WNDCLASSW cls={0};cls.lpfnWndProc=parent_proc;cls.hInstance=instance;cls.lpszClassName=L"SuperclassParent";
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Superclass protocol",WS_OVERLAPPEDWINDOW,0,0,560,420,NULL,NULL,instance,NULL);CHECK(root);
    const DWORD styles[]={BS_PUSHBUTTON,BS_AUTO3STATE,ES_AUTOHSCROLL,CBS_DROPDOWNLIST|CBS_HASSTRINGS,LBS_NOTIFY,SS_LEFT,SBS_HORZ};
    HWND windows[7];HFONT source=CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,0,0,0,0,L"Arial");CHECK(source);
    for(unsigned i=0;i<ARRAYSIZE(windows);++i){
        printf("SUPER_CREATE index=%u\n",i);fflush(stdout);
        windows[i]=CreateWindowExW(i==2?WS_EX_CLIENTEDGE:0,names[i],L"Protocol",WS_CHILD|WS_VISIBLE|styles[i],10+(i%2)*230,10+(i/2)*70,200,i==3?100:40,root,(HMENU)(UINT_PTR)(i+1),instance,NULL);CHECK(windows[i]);
        WCHAR real[64];CHECK(RealGetWindowClassW(windows[i],real,64) && !_wcsicmp(real,bases[i]));
        if(styled)CHECK(attached(windows[i]));
        SendMessageW(windows[i],WM_SETFONT,(WPARAM)source,TRUE);
    }
    HWND group=CreateWindowW(names[0],L"Group",WS_CHILD|WS_VISIBLE|BS_GROUPBOX,10,300,200,60,root,NULL,instance,NULL);CHECK(group);
    if(styled)CHECK(attached(group));
    SendMessageW(group,WM_SETFONT,(WPARAM)source,TRUE);
    WNDCLASSW custom={0};custom.lpfnWndProc=DefWindowProcW;custom.hInstance=instance;custom.lpszClassName=L"TButton";CHECK(RegisterClassW(&custom));
    HWND impostor=CreateWindowW(L"TButton",L"custom",WS_CHILD,0,0,10,10,root,NULL,instance,NULL);CHECK(impostor);
    HWND ownerdraw=CreateWindowW(names[0],L"owner",WS_CHILD|BS_OWNERDRAW,0,0,10,10,root,NULL,instance,NULL);CHECK(ownerdraw);
    HWND custom_list=CreateWindowW(names[4],L"",WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS,0,0,10,10,root,NULL,instance,NULL);CHECK(custom_list);
    HWND custom_combo=CreateWindowW(names[3],L"",WS_CHILD|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,0,0,100,80,root,NULL,instance,NULL);CHECK(custom_combo);
    if(styled)CHECK(!attached(impostor) && !attached(ownerdraw) && !attached(custom_list) && !attached(custom_combo));
    puts("SUPER_REJECT_OK custom_name ownerdraw_button ownerdraw_list");
    ShowWindow(root,SW_SHOWNORMAL);UpdateWindow(root);settle();
    SendMessageW(windows[0],BM_CLICK,0,0);CHECK(clicks==1);
    SetFocus(windows[0]);SendMessageW(windows[0],WM_KEYDOWN,VK_SPACE,0);SendMessageW(windows[0],WM_KEYUP,VK_SPACE,0);CHECK(clicks==2);
    SendMessageW(windows[1],BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(windows[1],BM_GETCHECK,0,0)==BST_CHECKED);
    SendMessageW(windows[1],BM_SETCHECK,BST_UNCHECKED,0);
    HWND choice[3];const DWORD choice_styles[]={BS_AUTO3STATE,BS_AUTORADIOBUTTON,BS_GROUPBOX};
    for(unsigned i=0;i<ARRAYSIZE(choice);++i){
        choice[i]=CreateWindowW(class_names[7+i],L"Choice",WS_CHILD|WS_VISIBLE|choice_styles[i],10+(int)i*150,340,140,40,root,(HMENU)(UINT_PTR)(30+i),instance,NULL);CHECK(choice[i]);
        WCHAR real[64];CHECK(RealGetWindowClassW(choice[i],real,ARRAYSIZE(real)) && !_wcsicmp(real,L"BUTTON"));
        if(styled)CHECK(attached(choice[i]));
        SendMessageW(choice[i],WM_SETFONT,(WPARAM)source,TRUE);
    }
    unsigned prior_clicks=clicks;
    for(unsigned i=1;i<=3;++i){SendMessageW(choice[0],BM_CLICK,0,0);CHECK(SendMessageW(choice[0],BM_GETCHECK,0,0)==(LRESULT)(i%3));}
    SendMessageW(choice[1],BM_CLICK,0,0);CHECK(SendMessageW(choice[1],BM_GETCHECK,0,0)==BST_CHECKED && clicks==prior_clicks+4);
    if(styled)for(unsigned i=0;i<ARRAYSIZE(choice);++i){DWORD before=image(choice[i],NULL,0);CHECK(remove_control(choice[i]) && !attached(choice[i]));CHECK(image(choice[i],NULL,0)!=before);CHECK((HFONT)SendMessageW(choice[i],WM_GETFONT,0,0)==source);}
    for(unsigned i=0;i<ARRAYSIZE(choice);++i)CHECK(DestroyWindow(choice[i]));
    printf("SUPER_VCL_CHOICE_OK native_superclass=3 state_click=1 styled=%u detach_pixels=%u\n",styled,styled);
    HWND memo=CreateWindowExW(WS_EX_CLIENTEDGE,L"TMemo",L"alpha\r\nbeta\r\ngamma\r\ndelta\r\nepsilon",WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,10,250,180,60,root,(HMENU)41,instance,NULL);CHECK(memo);
    HWND mask=CreateWindowExW(WS_EX_CLIENTEDGE,L"TMaskEdit",L"12345",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,210,250,150,30,root,(HMENU)42,instance,NULL);CHECK(mask);
    WCHAR real_edit[64];CHECK(RealGetWindowClassW(memo,real_edit,ARRAYSIZE(real_edit)) && !_wcsicmp(real_edit,L"EDIT"));
    CHECK(RealGetWindowClassW(mask,real_edit,ARRAYSIZE(real_edit)) && !_wcsicmp(real_edit,L"EDIT"));
    if(styled)CHECK(attached(memo) && attached(mask));
    SendMessageW(memo,WM_SETFONT,(WPARAM)source,TRUE);SendMessageW(mask,WM_SETFONT,(WPARAM)source,TRUE);
    CHECK(SendMessageW(memo,EM_GETLINECOUNT,0,0)>=5);SendMessageW(memo,EM_SETSEL,0,5);SendMessageW(memo,EM_REPLACESEL,TRUE,(LPARAM)L"first");
    CHECK(SendMessageW(memo,EM_UNDO,0,0));CHECK(SendMessageW(memo,EM_SETREADONLY,TRUE,0));
    WCHAR memo_text[128];GetWindowTextW(memo,memo_text,ARRAYSIZE(memo_text));CHECK(!wcscmp(memo_text,L"alpha\r\nbeta\r\ngamma\r\ndelta\r\nepsilon"));
    SendMessageW(memo,EM_LINESCROLL,0,2);CHECK(SendMessageW(memo,EM_GETFIRSTVISIBLELINE,0,0)>0);
    if(styled){DWORD before_memo=image(memo,NULL,0),before_mask=image(mask,NULL,0);CHECK(remove_control(memo) && remove_control(mask));
        CHECK(!attached(memo) && !attached(mask));CHECK(image(memo,NULL,0)!=before_memo && image(mask,NULL,0)!=before_mask);
        CHECK((HFONT)SendMessageW(memo,WM_GETFONT,0,0)==source && (HFONT)SendMessageW(mask,WM_GETFONT,0,0)==source);}
    CHECK(DestroyWindow(memo) && DestroyWindow(mask));
    printf("SUPER_VCL_EDIT_OK native_superclass=2 multiline_undo_readonly_scroll=1 styled=%u detach_pixels=%u\n",styled,styled);
    SendMessageW(windows[2],WM_SETTEXT,0,(LPARAM)L"12345");SendMessageW(windows[2],EM_SETSEL,1,4);
    SendMessageW(windows[2],EM_REPLACESEL,TRUE,(LPARAM)L"8");WCHAR text[80];GetWindowTextW(windows[2],text,80);CHECK(!wcscmp(text,L"185"));
    CHECK(SendMessageW(windows[2],EM_UNDO,0,0));GetWindowTextW(windows[2],text,80);CHECK(!wcscmp(text,L"12345"));
    for(unsigned i=0;i<2;++i){SendMessageW(windows[3],CB_ADDSTRING,0,(LPARAM)(i?L"second":L"first"));SendMessageW(windows[4],LB_ADDSTRING,0,(LPARAM)(i?L"second":L"first"));}
    CHECK(SendMessageW(windows[3],CB_SETCURSEL,1,0)==1 && SendMessageW(windows[3],CB_GETCURSEL,0,0)==1);
    SendMessageW(windows[3],CB_SHOWDROPDOWN,TRUE,0);CHECK(SendMessageW(windows[3],CB_GETDROPPEDSTATE,0,0));
    SendMessageW(windows[3],CB_SHOWDROPDOWN,FALSE,0);CHECK(!SendMessageW(windows[3],CB_GETDROPPEDSTATE,0,0));
    CHECK(SendMessageW(windows[4],LB_SETCURSEL,1,0)!=LB_ERR && SendMessageW(windows[4],LB_GETCURSEL,0,0)==1);
    const LPARAM item_data=(LPARAM)(UINT_PTR)0x1234abcd;
    CHECK(SendMessageW(windows[3],CB_SETITEMDATA,1,item_data)!=CB_ERR && SendMessageW(windows[3],CB_GETITEMDATA,1,0)==item_data);
    CHECK(SendMessageW(windows[4],LB_SETITEMDATA,1,item_data)!=LB_ERR && SendMessageW(windows[4],LB_GETITEMDATA,1,0)==item_data);
    HWND multi=CreateWindowW(names[4],L"",WS_CHILD|WS_VISIBLE|LBS_EXTENDEDSEL|LBS_NOTIFY,250,250,180,80,root,(HMENU)50,instance,NULL);CHECK(multi);
    if(styled)CHECK(attached(multi));
    CHECK(SendMessageW(multi,LB_ADDSTRING,0,(LPARAM)L"one")!=LB_ERR && SendMessageW(multi,LB_ADDSTRING,0,(LPARAM)L"two")!=LB_ERR);
    CHECK(SendMessageW(multi,LB_SETSEL,TRUE,0)!=LB_ERR && SendMessageW(multi,LB_SETSEL,TRUE,1)!=LB_ERR && SendMessageW(multi,LB_GETSELCOUNT,0,0)==2);
    if(styled)CHECK(remove_control(multi) && !attached(multi));
    CHECK(SendMessageW(multi,LB_GETSELCOUNT,0,0)==2 && DestroyWindow(multi));
    RECT geometry_before[7],geometry_after;DWORD hashes[7];
    for(unsigned i=0;i<7;++i){GetClientRect(windows[i],&geometry_before[i]);hashes[i]=image(windows[i],NULL,0);}
    if(styled){
        DWORD actual_pixels[200*100];image(windows[0],actual_pixels,ARRAYSIZE(actual_pixels));
        CHECK((actual_pixels[12*200+12]&0xffffff)==0xc0c0c0);
        puts("SUPER_ACTUAL_FILL_OK classic_face=c0c0c0 initializer_replaced=1");
        CHECK(remove_control(windows[0]));CHECK(image(windows[0],NULL,0)!=hashes[0]);
        CHECK(remove_control(windows[2]));CHECK(image(windows[2],NULL,0)!=hashes[2]);
        for(unsigned i=0;i<7;++i){if(i!=0 && i!=2)CHECK(remove_control(windows[i]));
            CHECK(!attached(windows[i]));GetClientRect(windows[i],&geometry_after);CHECK(EqualRect(&geometry_before[i],&geometry_after));
            CHECK((HFONT)SendMessageW(windows[i],WM_GETFONT,0,0)==source || i==6);
        }
        CHECK(SendMessageW(windows[3],CB_GETITEMDATA,1,0)==item_data && SendMessageW(windows[3],CB_GETCURSEL,0,0)==1);
        CHECK(SendMessageW(windows[4],LB_GETITEMDATA,1,0)==item_data && SendMessageW(windows[4],LB_GETCURSEL,0,0)==1);
        CHECK(image(windows[3],NULL,0)!=hashes[3] && image(windows[4],NULL,0)!=hashes[4]);
        DWORD group_hash=image(group,NULL,0);CHECK(remove_control(group));CHECK(image(group,NULL,0)!=group_hash);
        puts("SUPER_GROUP_OK native_alias=1 detach_pixels=1");
        puts("SUPER_RENDER_OK button edit pixels_changed=1 geometry_restored=1");
    }
    printf("SUPER_VCL_LIST_OK item_data_selection=1 multiselect=1 ownerdraw_rejected=%u detach_pixels=%u styled=%u\n",styled,styled,styled);
    UINT dpi=GetDpiForWindow(root);CHECK(DestroyWindow(root));settle();capture_end();
    LOGFONTW lf;CHECK(GetObjectW(source,sizeof(lf),&lf));CHECK(DeleteObject(source));
    for(unsigned i=0;i<7;++i)CHECK(UnregisterClassW(names[i],instance));
    for(unsigned i=7;i<ARRAYSIZE(class_names);++i)CHECK(UnregisterClassW(class_names[i],instance));
    CHECK(UnregisterClassW(custom.lpszClassName,instance) && UnregisterClassW(cls.lpszClassName,instance));
    printf("SUPER_PROTOCOL_OK bits=%u styled=%u dpi=%u aware=%u types=7 clicks=2 undo=1 combo_cancel=1 source_retained=1\n",(unsigned)(sizeof(void *)*8),styled,dpi,aware);
    return 0;
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
        ResumeThread(pi.hThread);DWORD code=1,wait=WaitForSingleObject(pi.hProcess,30000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        if(wait!=WAIT_OBJECT_0)TerminateJobObject(job,ERROR_TIMEOUT);
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting={0};BOOL empty=FALSE;ULONGLONG deadline=GetTickCount64()+5000;
        do{CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL));
            if(!accounting.ActiveProcesses){empty=TRUE;break;}WaitForSingleObject(pi.hProcess,10);
        }while(GetTickCount64()<deadline);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code && empty);
    }
    puts("SUPER_CLEANUP_OK owned_jobs_empty=4");return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcsncmp(argv[2],L"styled",6),wcsstr(argv[2],L"aware")!=NULL);
    return isolated_process(argc,argv,worker);
}
