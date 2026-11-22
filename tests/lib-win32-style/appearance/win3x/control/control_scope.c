#include "lib-win32-style/application/control/control_scope.h"
#include <stdio.h>
static style_control_scope *ending;
static style_control_scope_manager manager;
static int installs,edit_installs,reenter;
static style_control_scope_counts ended;
static BOOL control_install(HWND w,const style_window_palette *p)
{(void)w;(void)p;++installs;if(reenter){reenter=0;if(!style_control_scope_end(ending,&ended,&manager))return FALSE;}return TRUE;}
static BOOL edit_install(HWND w,BOOL border,const style_window_palette *p)
{(void)w;(void)p;if(!border)return FALSE;++edit_installs;return TRUE;}
static const style_control_palette_scope_services services={edit_install,control_install};
static LRESULT CALLBACK observe(int code,WPARAM a,LPARAM b)
{LRESULT r=CallNextHookEx(NULL,code,a,b);DWORD error=GetLastError();if(code>=0)style_control_scope_observe(&manager,(const CWPRETSTRUCT *)b,style_control_palette_scope_install,(void *)&services);SetLastError(error);return r;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"scope failure line %d error %lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
int main(void)
{
    HWND root=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,200,100,NULL,NULL,NULL,NULL);CHECK(root);
    style_control_scope *outer=style_control_palette_scope_begin(root,NULL,&manager,observe);CHECK(outer && manager.hook);
    HWND a=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD,0,0,20,20,root,NULL,NULL,NULL);CHECK(a && installs==1);
    style_control_scope *inner=style_control_palette_scope_begin(root,NULL,&manager,observe);CHECK(inner);
    CHECK(!style_control_scope_end((style_control_scope *)(UINT_PTR)1,NULL,&manager) && GetLastError()==ERROR_INVALID_PARAMETER);
    HWND b=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_BORDER,0,0,20,20,root,NULL,NULL,NULL);CHECK(b && edit_installs==1);
    style_control_scope_counts count={0};CHECK(style_control_scope_end(inner,&count,&manager));CHECK(count.installed==1 && manager.hook);
    ending=outer;reenter=1;HWND c=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD,0,0,20,20,root,NULL,NULL,NULL);CHECK(c && installs==2 && !manager.scopes && !manager.hook);
    CHECK(ended.installed==1);CHECK(!style_control_scope_end(NULL,NULL,&manager) && GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(DestroyWindow(root));puts("CONTROL_SCOPE_OK nested=1 counts=1 reentrant=1 cleanup=1");return 0;
}
