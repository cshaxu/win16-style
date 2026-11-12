#include "lib-style/win16-control/control_scope.h"
#include <stdio.h>
static win16_style_control_scope *scopes,*ending;
static HHOOK hook;
static int installs,edit_installs,reenter;
static win16_style_control_counts ended;
static BOOL control_install(HWND w,const win16_style_palette *p)
{(void)w;(void)p;++installs;if(reenter){reenter=0;if(!win16_control_scope_end(ending,&ended,&scopes,&hook))return FALSE;}return TRUE;}
static BOOL edit_install(HWND w,BOOL border,const win16_style_palette *p)
{(void)w;(void)p;if(!border)return FALSE;++edit_installs;return TRUE;}
static const win16_control_scope_services services={edit_install,control_install};
static LRESULT CALLBACK observe(int code,WPARAM a,LPARAM b)
{LRESULT r=CallNextHookEx(NULL,code,a,b);DWORD error=GetLastError();if(code>=0)win16_control_scope_observe(a,b,scopes,&services);SetLastError(error);return r;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"scope failure line %d error %lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
int main(void)
{
    HWND root=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,200,100,NULL,NULL,NULL,NULL);CHECK(root);
    win16_style_control_scope *outer=win16_control_scope_begin(root,NULL,&scopes,&hook,observe);CHECK(outer && hook);
    HWND a=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD,0,0,20,20,root,NULL,NULL,NULL);CHECK(a && installs==1);
    win16_style_control_scope *inner=win16_control_scope_begin(root,NULL,&scopes,&hook,observe);CHECK(inner);
    HWND b=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_BORDER,0,0,20,20,root,NULL,NULL,NULL);CHECK(b && edit_installs==1);
    win16_style_control_counts count={0};CHECK(win16_control_scope_end(inner,&count,&scopes,&hook));CHECK(count.installed==1 && hook);
    ending=outer;reenter=1;HWND c=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD,0,0,20,20,root,NULL,NULL,NULL);CHECK(c && installs==2 && !scopes && !hook);
    CHECK(ended.installed==1);CHECK(!win16_control_scope_end(NULL,NULL,&scopes,&hook) && GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(DestroyWindow(root));puts("CONTROL_SCOPE_OK nested=1 counts=1 reentrant=1 cleanup=1");return 0;
}
