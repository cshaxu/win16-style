#define COBJMACROS
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"COLOR_TRACE_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static HWND target;static WNDPROC original;static BOOL in_create;static unsigned early,late,creation_done;static LRESULT early_bar,early_bg;
static LRESULT CALLBACK watch_cbt(int code,WPARAM a,LPARAM b){
 if(code==HCBT_CREATEWND){WCHAR cls[64];GetClassNameW((HWND)a,cls,64);if(!wcscmp(cls,L"S24ProgressWitness"))target=(HWND)a;}
 return CallNextHookEx(NULL,code,a,b);
}
static LRESULT CALLBACK watch_before(int code,WPARAM a,LPARAM b){
 if(code>=0){CWPSTRUCT *m=(void*)b;if(m->hwnd==target){if(m->message==WM_CREATE)in_create=TRUE;
  if(m->message==PBM_SETBARCOLOR || m->message==PBM_SETBKCOLOR){if(in_create)early++;else late++;}}}
 return CallNextHookEx(NULL,code,a,b);
}
static LRESULT CALLBACK watch_after(int code,WPARAM a,LPARAM b){
 if(code>=0){CWPRETSTRUCT *m=(void*)b;if(m->hwnd==target && m->message==WM_CREATE){in_create=FALSE;creation_done++;}}
 return CallNextHookEx(NULL,code,a,b);
}
static LRESULT CALLBACK wrapper(HWND w,UINT m,WPARAM a,LPARAM b){
 if(m==WM_CREATE){LRESULT r=CallWindowProcW(original,w,m,a,b);early_bar=SendMessageW(w,PBM_SETBARCOLOR,0,RGB(11,22,33));early_bg=SendMessageW(w,PBM_SETBKCOLOR,0,RGB(44,55,66));return r;}
 return CallWindowProcW(original,w,m,a,b);
}
static int worker(int argc,WCHAR **argv){
 (void)argv;CHECK(argc==1);INITCOMMONCONTROLSEX init={sizeof(init),ICC_PROGRESS_CLASS};CHECK(InitCommonControlsEx(&init));
 HMODULE common=GetModuleHandleW(L"comctl32.dll");DLLVERSIONINFO version={.cbSize=sizeof(version)};HRESULT (CALLBACK *dv)(DLLVERSIONINFO*)=(void*)GetProcAddress(common,"DllGetVersion");CHECK(dv && SUCCEEDED(dv(&version)) && version.dwMajorVersion==5);
 WNDCLASSEXW cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,PROGRESS_CLASSW,&cls));original=cls.lpfnWndProc;cls.lpfnWndProc=wrapper;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"S24ProgressWitness";cls.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&cls));
 DWORD tid=GetCurrentThreadId();HHOOK cbt=SetWindowsHookExW(WH_CBT,watch_cbt,NULL,tid),before=SetWindowsHookExW(WH_CALLWNDPROC,watch_before,NULL,tid),after=SetWindowsHookExW(WH_CALLWNDPROCRET,watch_after,NULL,tid);CHECK(cbt&&before&&after);
 HWND w=CreateWindowExW(WS_EX_STATICEDGE,cls.lpszClassName,L"",WS_POPUP,0,0,180,24,NULL,NULL,cls.hInstance,NULL);CHECK(w&&w==target&&early==2&&creation_done==1);
 LRESULT getbar=SendMessageW(w,PBM_GETBARCOLOR,0,0),getbg=SendMessageW(w,PBM_GETBKCOLOR,0,0);
 LRESULT previous=SendMessageW(w,PBM_SETBARCOLOR,0,RGB(70,80,90));CHECK(late==1);
 unsigned before_direct=late;LRESULT bypass=CallWindowProcW(original,w,PBM_SETBARCOLOR,0,RGB(100,110,120));CHECK(late==before_direct);
 LRESULT next=SendMessageW(w,PBM_SETBARCOLOR,0,RGB(70,80,90));CHECK(late==before_direct+1);
 printf("COLOR_WITNESS bits=%u version=%lu.%lu create_setters=%u previous_bar=%08lx previous_bg=%08lx getters=%08lx,%08lx late_previous=%08lx direct_previous=%08lx next_previous=%08lx direct_hook_observed=%u\n",(unsigned)sizeof(void*)*8,version.dwMajorVersion,version.dwMinorVersion,early,(DWORD)early_bar,(DWORD)early_bg,(DWORD)getbar,(DWORD)getbg,(DWORD)previous,(DWORD)bypass,(DWORD)next,late-before_direct-1);
 CHECK((DWORD)getbar==0 && (DWORD)getbg==0 && (DWORD)previous==RGB(11,22,33) && (DWORD)bypass==RGB(70,80,90) && (DWORD)next==RGB(100,110,120));
 CHECK(DestroyWindow(w));CHECK(UnhookWindowsHookEx(after)&&UnhookWindowsHookEx(before)&&UnhookWindowsHookEx(cbt));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));puts("COLOR_WITNESS_CLEANUP_OK");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
