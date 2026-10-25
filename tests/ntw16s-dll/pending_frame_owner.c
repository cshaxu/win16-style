#include "ntw16s-dll/attachment.h"
#include <stdio.h>
#include <imm.h>
#include <wchar.h>
#include "isolated_process.h"
static BOOL (__cdecl *attached)(HWND),(__cdecl *remove_window)(HWND),(__cdecl *install_frame)(HWND,const void*),(__cdecl *remove_frame)(HWND);
static BOOL fired,removed,independent;static unsigned mode;
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
 if(m==WM_CREATE && mode<2)ShowWindow(w,SW_SHOWNOACTIVATE);
 if(mode<2 && !fired && m==WM_WINDOWPOSCHANGING && attached(w)){
  fired=TRUE;removed=remove_window(w);if(mode==1)independent=install_frame(w,NULL);
 }
 return DefWindowProcW(w,m,p,l);
}
static int worker(int argc,WCHAR **argv)
{
 if(argc!=2)return 2;
 /* ASCII-only fixture: avoid external TSF monitor teardown on a private desktop. */
 if(!ImmDisableIME(0))return 8;
 HMODULE dll=LoadLibraryW(argv[1]);if(!dll)return 3;
 ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void*)GetProcAddress(dll,"Ntw16sCreationBegin");
 BOOL (__cdecl *end)(ntw16s_creation_scope*,ntw16s_creation_result*)=(void*)GetProcAddress(dll,"Ntw16sCreationEnd");
 attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_window=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");
 install_frame=(void*)GetProcAddress(dll,"win16_style_install");remove_frame=(void*)GetProcAddress(dll,"win16_style_remove");
 if(!begin||!end||!attached||!remove_window||!install_frame||!remove_frame)return 4;
 WNDCLASSW cls={.lpfnWndProc=window_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PendingFrameProbe"};if(!RegisterClassW(&cls))return 5;
 unsigned failures=0;
 for(mode=0;mode<3;++mode){fired=removed=independent=FALSE;ntw16s_creation_scope*scope=begin(TRUE);if(!scope)return 6;
  HWND w=CreateWindowW(cls.lpszClassName,L"owned frame",WS_OVERLAPPEDWINDOW|WS_VISIBLE,20,20,300,200,NULL,NULL,cls.hInstance,NULL);
  ntw16s_creation_result result={0};if(!end(scope,&result)||!w)return 7;
  BOOL coordinator=attached(w);if(mode==2)remove_window(w);SetLastError(0);BOOL fresh=install_frame(w,NULL);DWORD error=GetLastError();
  printf("PENDING_FRAME bits=%u mode=%u fired=%u removed=%u independent=%u attached=%u scope=%lu fresh=%u error=%lu\n",(unsigned)sizeof(void*)*8,mode,fired,removed,independent,coordinator,result.failure,fresh,error);
  if(mode==0)failures+=!(fired&&removed&&!coordinator&&result.failure==ERROR_CANCELLED&&fresh);
  if(mode==1)failures+=!(fired&&removed&&independent&&!coordinator&&result.failure==ERROR_CANCELLED&&!fresh&&error==ERROR_ALREADY_EXISTS);
  if(mode==2)failures+=!(!fired&&coordinator&&!result.failure&&fresh);
  remove_frame(w);DestroyWindow(w);
 }
 FreeLibrary(dll);printf("PENDING_FRAME_OWNER_OK failures=%u\n",failures);return failures?1:0;
}
int wmain(int argc,WCHAR**argv){return isolated_process(argc,argv,worker);}
