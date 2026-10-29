#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
static BOOL (__cdecl *attached)(HWND);static int initialized;static BOOL frame;
typedef struct page_template {DLGTEMPLATE dialog;WORD menu,cls,title;} page_template;
static INT_PTR CALLBACK page_proc(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{(void)window;(void)message;(void)wparam;(void)lparam;return FALSE;}
static void CALLBACK close_sheet(HWND window,UINT message,UINT_PTR timer,DWORD time)
{(void)message;(void)time;KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);}
static int CALLBACK sheet_callback(HWND window,UINT message,LPARAM lparam)
{(void)lparam;if(message==PSCB_INITIALIZED){++initialized;SetTimer(window,916,100,close_sheet);}return 0;}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    HMODULE m=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    attached=m?(BOOL (__cdecl *)(HWND))(void *)GetProcAddress(m,"Ntw16sWindowAttached"):NULL;
    page_template page={0};page.dialog.style=WS_CHILD|WS_VISIBLE|DS_CONTROL;page.dialog.cx=150;page.dialog.cy=80;
    INT_PTR result;
    if(argv[1][0]==L'a'){
        PROPSHEETPAGEA p={0};p.dwSize=sizeof(p);p.dwFlags=PSP_DLGINDIRECT;p.pResource=&page.dialog;p.pfnDlgProc=page_proc;
        PROPSHEETHEADERA h={0};h.dwSize=sizeof(h);h.dwFlags=PSH_PROPSHEETPAGE|PSH_USECALLBACK;h.pszCaption="Property probe";h.nPages=1;h.ppsp=&p;h.pfnCallback=sheet_callback;
        result=PropertySheetA(&h);
    }else{
        PROPSHEETPAGEW p={0};p.dwSize=sizeof(p);p.dwFlags=PSP_DLGINDIRECT;p.pResource=&page.dialog;p.pfnDlgProc=page_proc;
        PROPSHEETHEADERW h={0};h.dwSize=sizeof(h);h.dwFlags=PSH_PROPSHEETPAGE|PSH_USECALLBACK;h.pszCaption=L"Property probe";h.nPages=1;h.ppsp=&p;h.pfnCallback=sheet_callback;
        result=PropertySheetW(&h);
    }
    printf("SHEET_DIALOG bits=%u ansi=%d initialized=%d frame=%d result=%lld\n",(unsigned)(sizeof(void*)*8),argv[1][0]==L'a',initialized,frame,(long long)result);
    return initialized==1 && frame && result!=-1 ? 0:1;
}
