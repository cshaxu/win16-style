/* Native UI workload: no style import, LoadLibrary, install, or policy setup.
 * Optional read-only diagnostic query proves actual attached state. */
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <commctrl.h>
static BOOL expected;
static BOOL dialog_suite;
static BOOL message_suite;
static WCHAR witness[MAX_PATH];
static BOOL (__cdecl *attached)(HWND);
static void failure(int line,const char *expression)
{
    DWORD error=GetLastError();FILE *file=NULL;
    fprintf(stderr,"AUTOMATIC FAIL line=%d error=%lu: %s\n",line,error,expression);
    if(witness[0] && !_wfopen_s(&file,witness,L"at")){
        fprintf(file,"FAIL line=%d error=%lu: %s\n",line,error,expression);fclose(file);}
    ExitProcess(1);
}
#define CHECK(x) do{if(!(x))failure(__LINE__,#x);}while(0)
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_APP+1)return 12345;return DefWindowProcW(w,m,a,b);}
static void observe(HWND w)
{CHECK(w);if(expected)CHECK(attached(w));}
static void record(const char *phase)
{
    FILE *file=NULL;CHECK(!_wfopen_s(&file,witness,L"at"));
    fprintf(file,"pid=%lu tid=%lu %s\n",GetCurrentProcessId(),GetCurrentThreadId(),phase);CHECK(!fclose(file));
}
#include "automatic_messages.h"
static INT_PTR CALLBACK dialog(HWND w,UINT message,WPARAM a,LPARAM modal)
{
    (void)a;
    if(message==WM_INITDIALOG){SetWindowLongPtrW(w,DWLP_USER,modal);CHECK(PostMessageW(w,WM_APP+7,0,0));return TRUE;}
    if(message==WM_APP+7){observe(w);record("dialog-observed");
        if(GetWindowLongPtrW(w,DWLP_USER))EndDialog(w,47);
        return TRUE;
        }
    return FALSE;
}
static void dialogs(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);HWND w;
    HRSRC resource=FindResourceW(module,MAKEINTRESOURCEW(101),RT_DIALOG);
    HGLOBAL loaded;const DLGTEMPLATE *data;
    CHECK(resource);loaded=LoadResource(module,resource);CHECK(loaded);data=LockResource(loaded);CHECK(data);
    w=CreateDialogParamW(module,MAKEINTRESOURCEW(101),NULL,dialog,0);observe(w);CHECK(DestroyWindow(w));
    w=CreateDialogParamA(module,MAKEINTRESOURCEA(101),NULL,dialog,0);observe(w);CHECK(DestroyWindow(w));
    w=CreateDialogIndirectParamW(module,data,NULL,dialog,0);observe(w);CHECK(DestroyWindow(w));
    w=CreateDialogIndirectParamA(module,data,NULL,dialog,0);observe(w);CHECK(DestroyWindow(w));
    CHECK(DialogBoxParamW(module,MAKEINTRESOURCEW(101),NULL,dialog,1)==47);
    CHECK(DialogBoxParamA(module,MAKEINTRESOURCEA(101),NULL,dialog,1)==47);
    CHECK(DialogBoxIndirectParamW(module,data,NULL,dialog,1)==47);
    CHECK(DialogBoxIndirectParamA(module,data,NULL,dialog,1)==47);
    record("eight-dialog-APIs-complete");
}
static DWORD WINAPI windows(void *unused)
{
    HWND root,children[7];WNDCLASSW cls={0};(void)unused;
    cls.lpfnWndProc=owner;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"StandaloneAutomaticTarget";
    CHECK(RegisterClassW(&cls) || GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
    for(unsigned variant=0;variant<2;++variant){
        root=variant?CreateWindowExA(0,"StandaloneAutomaticTarget","Original A",WS_OVERLAPPEDWINDOW,0,0,500,400,NULL,NULL,cls.hInstance,NULL):
            CreateWindowExW(0,cls.lpszClassName,L"Original W",WS_OVERLAPPEDWINDOW,0,0,500,400,NULL,NULL,cls.hInstance,NULL);
        observe(root);CHECK(SendMessageW(root,WM_APP+1,0,0)==12345);
        children[0]=CreateWindowW(L"BUTTON",L"Check",WS_CHILD|BS_AUTOCHECKBOX,10,10,120,30,root,(HMENU)101,cls.hInstance,NULL);
        children[1]=CreateWindowA("STATIC","Label",WS_CHILD,10,50,120,30,root,(HMENU)102,cls.hInstance,NULL);
        children[2]=CreateWindowW(L"EDIT",L"Native text",WS_CHILD|WS_BORDER,10,90,180,30,root,(HMENU)103,cls.hInstance,NULL);
        children[3]=CreateWindowW(L"LISTBOX",NULL,WS_CHILD|WS_BORDER,10,130,180,50,root,(HMENU)104,cls.hInstance,NULL);
        children[4]=CreateWindowW(L"COMBOBOX",NULL,WS_CHILD|CBS_DROPDOWNLIST,10,190,180,90,root,(HMENU)105,cls.hInstance,NULL);
        children[5]=CreateWindowW(L"SCROLLBAR",NULL,WS_CHILD|SBS_HORZ,10,230,180,20,root,(HMENU)106,cls.hInstance,NULL);
        children[6]=CreateWindowA("BUTTON","Dynamic",WS_CHILD|BS_PUSHBUTTON,220,30,150,30,root,(HMENU)107,cls.hInstance,NULL);
        for(unsigned i=0;i<7;++i)observe(children[i]);
        SendMessageW(children[0],BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(children[0],BM_GETCHECK,0,0)==BST_CHECKED);
        SendMessageW(children[3],LB_ADDSTRING,0,(LPARAM)L"Stored item");CHECK(SendMessageW(children[3],LB_GETCOUNT,0,0)==1);
        SendMessageW(children[4],CB_ADDSTRING,0,(LPARAM)L"Selection");CHECK(SendMessageW(children[4],CB_SETCURSEL,0,0)==0);
        CHECK(SetWindowTextW(root,L"Dynamic title retained"));
        {WCHAR text[80];CHECK(GetWindowTextW(root,text,80) && !wcscmp(text,L"Dynamic title retained"));}
        /* Attachment keeps native default-font semantics; styling is witnessed
         * by Ntw16sWindowAttached above, not by a destructive font override. */
        CHECK(!SendMessageW(children[0],WM_GETFONT,0,0));
        CHECK(DestroyWindow(root));
    }
    /* Keep an owned window alive while successive modal windows close. This
     * does not change their owner, close messages, results, or assertions. */
    HWND anchor=NULL;
    if(dialog_suite || message_suite){
        anchor=CreateWindowW(L"STATIC",L"Integration test anchor",WS_OVERLAPPEDWINDOW,0,0,120,80,NULL,NULL,cls.hInstance,NULL);
        CHECK(anchor);ShowWindow(anchor,SW_SHOW);
    }
    if(dialog_suite)dialogs();
    if(message_suite)messages();
    if(anchor)CHECK(DestroyWindow(anchor));
    record("GUI-thread-complete");
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    HANDLE thread;DWORD code;HMODULE dll;
    CHECK(argc==7 || argc==8);CHECK(!wcscmp(argv[1],L"styled") || !wcscmp(argv[1],L"native"));
    expected=!wcscmp(argv[1],L"styled");
    dialog_suite=argc==8 && !wcscmp(argv[7],L"dialogs");message_suite=argc==8 && !wcscmp(argv[7],L"messages");
    CHECK(argc==7 || dialog_suite || message_suite);
    {WCHAR directory[MAX_PATH];CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,MAX_PATH));
        CHECK(swprintf_s(witness,MAX_PATH,L"%ls\\target-%u-%d-%d.txt",directory,(unsigned)(sizeof(void *)*8),expected,dialog_suite?1:message_suite?2:0)>0);}
    record("target-entry");
    CHECK(!wcscmp(argv[2],L"two words") && !wcscmp(argv[3],L"quote\"inside") &&
        !wcscmp(argv[4],L"trailing\\") && !wcscmp(argv[5],L"") && !wcscmp(argv[6],L"中文 αβ"));
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    CHECK(!!dll==!!expected);
    if(expected){attached=(BOOL (__cdecl *)(HWND))(void *)GetProcAddress(dll,"Ntw16sWindowAttached");CHECK(attached);}
    windows(NULL);thread=CreateThread(NULL,0,windows,NULL,0,NULL);CHECK(thread);
    {ULONGLONG deadline=GetTickCount64()+120000;DWORD result=WAIT_TIMEOUT;
        while(GetTickCount64()<deadline){MSG message;DWORD remaining=(DWORD)(deadline-GetTickCount64());
            result=MsgWaitForMultipleObjectsEx(1,&thread,remaining,QS_ALLINPUT,MWMO_INPUTAVAILABLE);
            if(result!=WAIT_OBJECT_0+1)break;
            while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){CHECK(message.message!=WM_QUIT);TranslateMessage(&message);DispatchMessageW(&message);}}
        CHECK(result==WAIT_OBJECT_0 && GetExitCodeThread(thread,&code) && !code);}
    CloseHandle(thread);record("target-complete");
    printf("AUTOMATIC TARGET PASS width=%u styled=%d initial/new GUI thread A/W root+7 controls, native state, exact Unicode/quote/empty arguments\n",(unsigned)(sizeof(void *)*8),expected);
    return 0;
}
