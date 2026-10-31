#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include "ntw16s-dll/injection/startup_hooks.h"
static BOOL (WINAPI *real_edge)(HDC,LPRECT,UINT,UINT);static LONG count,matched;static HWND pick;
static BOOL WINAPI edge(HDC dc,LPRECT rect,UINT kind,UINT flags){
    HWND w=WindowFromDC(dc);HMODULE mod=NULL;WCHAR path[MAX_PATH]=L"";
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)__builtin_return_address(0),&mod);
    if(mod)GetModuleFileNameW(mod,path,MAX_PATH);
    if(w==pick&&mod==GetModuleHandleW(L"comdlg32.dll")&&kind==EDGE_SUNKEN&&flags==(BF_RECT|BF_ADJUST))InterlockedIncrement(&matched);
    LONG n=InterlockedIncrement(&count);if(n<=4&&rect)printf("EDGE n=%ld hwnd=%p dialog=%p edge=%x flags=%x rect=%ld,%ld,%ld,%ld module=%ls\n",n,(void *)w,(void *)pick,kind,flags,rect->left,rect->top,rect->right,rect->bottom,path);
    return real_edge(dc,rect,kind,flags);
}
static UINT_PTR CALLBACK hook(HWND w,UINT m,WPARAM wp,LPARAM lp){(void)lp;if(m==WM_INITDIALOG){pick=w;for(int id=720;id<=721;++id){HWND child=GetDlgItem(w,id);RECT r={0};if(child&&GetWindowRect(child,&r)){MapWindowPoints(NULL,w,(POINT *)&r,2);printf("GRID id=%d rect=%ld,%ld,%ld,%ld\n",id,r.left,r.top,r.right,r.bottom);}}SetTimer(w,427,500,NULL);}if(m==WM_TIMER&&wp==427){KillTimer(w,427);PostMessageW(w,WM_COMMAND,IDCANCEL,0);}return 0;}
int wmain(void){setvbuf(stdout,NULL,_IONBF,0);HMODULE u=GetModuleHandleW(L"user32.dll");real_edge=(void *)GetProcAddress(u,"DrawEdge");
    if(!real_edge||ntw16s_hooks_begin()||ntw16s_hooks_attach(&real_edge,(void *)edge)||ntw16s_hooks_commit())return 2;
    COLORREF colors[16]={0};CHOOSECOLORW o={0};o.lStructSize=sizeof(o);o.lpCustColors=colors;o.Flags=CC_ENABLEHOOK;o.lpfnHook=hook;BOOL result=ChooseColorW(&o);DWORD error=CommDlgExtendedError();printf("EDGE_RESULT bits=%u count=%ld matched=%ld result=%d error=%lu\n",(unsigned)(sizeof(void*)*8),count,matched,result,error);return result||error||matched<64?1:0;}
