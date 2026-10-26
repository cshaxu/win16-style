#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#include "../../src/ntw16s-dll/control_glyphs.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
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
        fprintf(stderr,"VB_CAPTURE_CLEANUP_FAIL error=%lu\n",GetLastError());ExitProcess(1);
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
    if(!wcscmp(cls,L"ThunderRT6TextBox"))SendMessageW(w,WM_PRINT,(WPARAM)surface.dc,PRF_CLIENT|PRF_ERASEBKGND);
    else SendMessageW(w,WM_PRINTCLIENT,(WPARAM)surface.dc,PRF_CLIENT);
    GdiFlush();DWORD hash=2166136261u;
    for(UINT i=0;i<200*100;++i)hash=(hash^surface.bits[i])*16777619u;
    if(pixels && count==200*100)CopyMemory(pixels,surface.bits,count*sizeof(DWORD));
    if(!RestoreDC(surface.dc,saved))ExitProcess(1);
    return hash;
}
static int child(const WCHAR *runtime_path,BOOL styled,BOOL aware)
{
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HMODULE runtime=LoadLibraryW(runtime_path);CHECK(runtime);
    BOOL (WINAPI *registration)(void)=(void *)GetProcAddress(runtime,"VbRegister");
    void (WINAPI *release)(void)=(void *)GetProcAddress(runtime,"VbRelease");CHECK(registration && release && registration());
    HMODULE dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
    BOOL (__cdecl *remove_control)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    CHECK(styled?(dll && attached && remove_control):!dll);
    WNDCLASSW cls={0};cls.lpfnWndProc=parent_proc;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"VBProtocolParent";
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"VB protocol",WS_OVERLAPPEDWINDOW,0,0,360,240,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    HWND group=CreateWindowExW(0,L"ThunderRT6Frame",L"Frame",WS_CHILD|WS_VISIBLE|BS_GROUPBOX,0,0,200,100,root,(HMENU)1,runtime,NULL);CHECK(group);
    HWND button=CreateWindowW(L"ThunderRT6CommandButton",L"&Press",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,210,0,100,30,root,(HMENU)2,runtime,NULL);CHECK(button);
    HWND edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"ThunderRT6TextBox",L"native",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,210,40,100,24,root,(HMENU)3,runtime,NULL);CHECK(edit);
    HWND checkbox=CreateWindowW(L"ThunderRT6CheckBox",L"&Check",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTO3STATE,210,90,120,30,root,(HMENU)4,runtime,NULL);CHECK(checkbox);
    WCHAR real_check_class[80];CHECK(RealGetWindowClassW(checkbox,real_check_class,80) && wcscmp(real_check_class,L"Button") && wcscmp(real_check_class,L"BUTTON"));
    if(styled)CHECK(attached(group) && attached(button) && attached(edit) && attached(checkbox));
    /* Same spelling under another registration owner must remain custom. */
    WNDCLASSW impostor={0};impostor.lpfnWndProc=DefWindowProcW;
    impostor.hInstance=cls.hInstance;impostor.lpszClassName=L"ThunderRT6CommandButton";
    CHECK(RegisterClassW(&impostor));
    HWND custom=CreateWindowW(impostor.lpszClassName,L"custom",WS_CHILD,0,0,10,10,root,NULL,cls.hInstance,NULL);
    CHECK(custom);if(styled)CHECK(!attached(custom));
    impostor.lpszClassName=L"ThunderRT6CheckBox";CHECK(RegisterClassW(&impostor));
    HWND fake_check=CreateWindowW(impostor.lpszClassName,L"custom check",WS_CHILD,0,0,10,10,root,NULL,cls.hInstance,NULL);CHECK(fake_check);if(styled)CHECK(!attached(fake_check));
    HWND graphical=CreateWindowW(L"ThunderRT6CheckBox",L"native graphical",WS_CHILD|WS_VISIBLE|BS_AUTO3STATE|BS_PUSHLIKE,0,150,140,28,root,(HMENU)5,runtime,NULL);CHECK(graphical);if(styled)CHECK(!attached(graphical));
    HWND transition=CreateWindowW(L"ThunderRT6CheckBox",L"native transition",WS_CHILD|WS_VISIBLE|BS_AUTO3STATE,150,150,140,28,root,(HMENU)6,runtime,NULL);CHECK(transition);if(styled)CHECK(attached(transition));
    HWND option_a=CreateWindowW(L"ThunderRT6OptionButton",L"Option A",WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON|WS_GROUP,0,190,120,28,root,(HMENU)8,runtime,NULL);
    HWND option_b=CreateWindowW(L"ThunderRT6OptionButton",L"Option B",WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,140,190,120,28,root,(HMENU)9,runtime,NULL);
    HWND wrong_runtime=CreateWindowW(L"ThunderRT5CheckBox",L"wrong runtime",WS_CHILD|BS_AUTO3STATE,0,0,100,28,root,NULL,runtime,NULL);CHECK(option_a && option_b && wrong_runtime);
    if(styled)CHECK(attached(option_a) && attached(option_b) && !attached(wrong_runtime));
    RECT geometry_before,geometry_after;GetClientRect(edit,&geometry_before);

    ShowWindow(root,SW_SHOWNORMAL);UpdateWindow(root);settle();
    WCHAR text[80];SendMessageW(edit,EM_SETSEL,1,4);SendMessageW(edit,EM_REPLACESEL,TRUE,(LPARAM)L"8");
    GetWindowTextW(edit,text,80);CHECK(!wcscmp(text,L"n8ve"));CHECK(SendMessageW(edit,EM_UNDO,0,0));
    GetWindowTextW(edit,text,80);CHECK(!wcscmp(text,L"native"));
    SendMessageW(button,BM_CLICK,0,0);CHECK(clicks==1);
    SetFocus(button);SendMessageW(button,WM_KEYDOWN,VK_SPACE,0);SendMessageW(button,WM_KEYUP,VK_SPACE,0);CHECK(clicks==2);
    SendMessageW(button,WM_ENABLE,FALSE,0);SendMessageW(button,WM_ENABLE,TRUE,0);
    GetClientRect(edit,&geometry_after);CHECK(EqualRect(&geometry_before,&geometry_after));
    if(styled){DWORD styled_edit=image(edit,NULL,0);CHECK(remove_control(edit));
        CHECK(image(edit,NULL,0)!=styled_edit);puts("VB_EDIT_FONT_PIXELS changed=1 native_font_property=preserved");}
    SendMessageW(option_a,BM_CLICK,0,0);CHECK(SendMessageW(option_a,BM_GETCHECK,0,0)==1 && SendMessageW(option_b,BM_GETCHECK,0,0)==0);
    SendMessageW(option_b,BM_CLICK,0,0);CHECK(SendMessageW(option_b,BM_GETCHECK,0,0)==1 && SendMessageW(option_a,BM_GETCHECK,0,0)==0);
    if(styled){DWORD pixels[200*100];(void)image(option_b,pixels,ARRAYSIZE(pixels));RECT bounds;CHECK(GetClientRect(option_b,&bounds));int dpi=(int)GetDpiForWindow(option_b),top=(bounds.bottom-MulDiv(13,dpi,96))/2;
        for(int y=0;y<13;++y)for(int x=0;x<14;++x){unsigned expected=control_checks[13+y][14+x]=='0'?0:0xffffff;CHECK((pixels[(top+MulDiv(y,dpi,96))*200+MulDiv(x,dpi,96)]&0xffffff)==expected);}}
    CHECK(DestroyWindow(option_a) && DestroyWindow(option_b) && DestroyWindow(wrong_runtime));puts("VB_OPTION_OK group_native=1 source_pixels=182 wrong_runtime_rejected=1");
    HFONT check_font=(HFONT)SendMessageW(checkbox,WM_GETFONT,0,0);unsigned original_clicks=clicks;
    for(unsigned value=1;value<=3;++value){SendMessageW(checkbox,BM_CLICK,0,0);CHECK(SendMessageW(checkbox,BM_GETCHECK,0,0)==(LRESULT)(value%3));}
    CHECK(clicks==original_clicks+3 && (HFONT)SendMessageW(checkbox,WM_GETFONT,0,0)==check_font);
    SetFocus(checkbox);SendMessageW(checkbox,WM_KEYDOWN,VK_SPACE,0);SendMessageW(checkbox,WM_KEYUP,VK_SPACE,0);CHECK(SendMessageW(checkbox,BM_GETCHECK,0,0)==BST_CHECKED && clicks==original_clicks+4);
    EnableWindow(checkbox,FALSE);CHECK(!IsWindowEnabled(checkbox));EnableWindow(checkbox,TRUE);
    if(styled){DWORD pixels[200*100];DWORD painted=image(checkbox,pixels,ARRAYSIZE(pixels));RECT bounds;CHECK(GetClientRect(checkbox,&bounds));UINT dpi=GetDpiForWindow(checkbox);int top=(bounds.bottom-MulDiv(13,(int)dpi,96))/2;
        for(int y=0;y<13;++y)for(int x=0;x<14;++x){unsigned expected=control_checks[y][14+x]=='0'?0:0xffffff;CHECK((pixels[(top+MulDiv(y,(int)dpi,96))*200+MulDiv(x,(int)dpi,96)]&0xffffff)==expected);}
        puts("VB_CHECKBOX_GLYPH_OK source_pixels=182 checked=1");CHECK(remove_control(checkbox) && !attached(checkbox));CHECK(image(checkbox,NULL,0)!=painted && (HFONT)SendMessageW(checkbox,WM_GETFONT,0,0)==check_font);}
    LONG_PTR transition_style=GetWindowLongPtrW(transition,GWL_STYLE);SetWindowLongPtrW(transition,GWL_STYLE,transition_style|BS_PUSHLIKE);
    if(styled){DWORD fallback=image(transition,NULL,0);CHECK(remove_control(transition));CHECK(image(transition,NULL,0)==fallback);}
    HWND dying=CreateWindowW(L"ThunderRT6CheckBox",L"retirement callback",WS_CHILD|BS_AUTO3STATE,0,190,140,28,root,(HMENU)7,runtime,NULL);CHECK(dying);
    if(styled){void (WINAPI *arm)(HWND)=(void *)GetProcAddress(runtime,"VbCheckboxRetirementProbe");unsigned (WINAPI *late)(void)=(void *)GetProcAddress(runtime,"VbCheckboxLateCalls");CHECK(arm && late && attached(dying));arm(dying);SetWindowLongPtrW(dying,GWL_STYLE,GetWindowLongPtrW(dying,GWL_STYLE)|BS_PUSHLIKE);CHECK(!IsWindow(dying) && !late());puts("VB_CHECKBOX_RETIRE_DESTROY_OK no_forward_after_destroy=1");}
    else CHECK(DestroyWindow(dying));
    CHECK(DestroyWindow(graphical) && DestroyWindow(transition));puts("VB_CHECKBOX_NATIVE_FALLBACK_OK initial_graphical=1 live_transition=1");
    CHECK(DestroyWindow(checkbox) && DestroyWindow(fake_check));CHECK(UnregisterClassW(L"ThunderRT6CheckBox",cls.hInstance));
    puts("VB_CHECKBOX_OK three_states=1 keyboard=1 notifications=4 owner_negative=1 font_preserved=1 detach=1");
    CHECK(DestroyWindow(edit) && DestroyWindow(button) && DestroyWindow(custom));settle();
    DWORD pixels[200*100];DWORD styled_hash=image(group,pixels,ARRAYSIZE(pixels));
    CHECK((pixels[72*200+24]&0xffffff)==RGB(67,31,219));
    /* Host USER/font caches are observed separately. Private leases and
     * mapping allocations have deterministic assertions in core/vb-font. */
    for(unsigned i=0;i<8;++i){(void)image(group,NULL,0);settle();}
    DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(unsigned i=0;i<32;++i)CHECK(image(group,NULL,0)==styled_hash);
    settle();DWORD after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);printf("VB_HOST_GDI_OBSERVE %lu,%lu\n",before,after);
    for(unsigned batch=0;batch<2;++batch){before=after;
        for(unsigned i=0;i<32;++i)CHECK(image(group,NULL,0)==styled_hash);
        settle();after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);printf("VB_HOST_GDI_BATCH %u %lu,%lu\n",batch,before,after);
    }
    if(styled){
        DWORD native_pixels[200*100];unsigned changed=0;
        CHECK(remove_control(group));CHECK(image(group,native_pixels,ARRAYSIZE(native_pixels))!=styled_hash);
        for(unsigned y=32;y<70;++y)for(unsigned x=16;x<180;++x)
            changed+=pixels[y*200+x]!=native_pixels[y*200+x];
        for(unsigned y=80;y<94;++y)for(unsigned x=140;x<160;++x)
            CHECK(pixels[y*200+x]==native_pixels[y*200+x]);
        CHECK(changed>20);printf("VB_LABEL_PIXELS changed=%u restored_after_detach=1\n",changed);
    }
    UINT dpi=GetDpiForWindow(root);
    CHECK(DestroyWindow(root));CHECK(UnregisterClassW(L"ThunderRT6CommandButton",cls.hInstance));
    capture_end();release();CHECK(FreeLibrary(runtime));
    printf("VB_WINDOWED_OK bits=%u styled=%u clicks=%u dpi=%u aware=%u\nVB_EDIT_OK undo=1\nVB_CONTAINER_PAINT_OK label_changed=%u sentinel=preserved handles=%lu,%lu\n",(unsigned)(sizeof(void *)*8),styled,clicks,dpi,aware,styled,before,after);
    return 0;
}
static int worker(int argc,WCHAR **argv)
{
    if(argc==4 && !wcscmp(argv[1],L"--child"))return child(argv[2],!wcsncmp(argv[3],L"styled",6),wcsstr(argv[3],L"aware")!=NULL);
    CHECK(argc==3);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned mode=0;mode<2;++mode){
        if(mode)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child \"%ls\" %ls",argv[1],self,argv[2],aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child \"%ls\" %ls",self,argv[2],aware?L"bare-aware":L"bare");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};
        limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
        si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        CHECK(AssignProcessToJobObject(job,pi.hProcess));ResumeThread(pi.hThread);
        DWORD code=1,wait=WaitForSingleObject(pi.hProcess,30000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    puts("VB_CLEANUP_OK bare/styled owned jobs closed");return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==4 && !wcscmp(argv[1],L"--child"))return child(argv[2],!wcsncmp(argv[3],L"styled",6),wcsstr(argv[3],L"aware")!=NULL);
    return isolated_process(argc,argv,worker);
}
