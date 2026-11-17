#include "lib-win32-style/application/adapters/frame.h"
#include "lib-win32-style/application/adapters/surface.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"MODERN FRAME FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
typedef struct fixture {
    style_uwp_frame frame;
    int mode;
    style_uwp_frame_action_entry actions[];
} fixture;
static unsigned disposed,painted;
static BOOL geometry(void *context,RECT *outer,RECT *hole,POINT *anchor)
{
    fixture *f=context;BOOL zoomed;
    if(f->mode==1){SetLastError(1234);return FALSE;}
    if(!style_uwp_frame_bounds(f->frame.host.owner,f->frame.host.title,outer,hole,&zoomed))return FALSE;
    *anchor=(POINT){hole->left,hole->top};return TRUE;
}
static void paint(void *context,HDC dc)
{
    fixture *f=context;++painted;
    RECT bounds={0,0,f->frame.bitmap.size.cx,f->frame.bitmap.size.cy};
    FillRect(dc,&bounds,GetStockObject(WHITE_BRUSH));
    if(f->mode==2)style_uwp_frame_remove(f->frame.host.owner);
    if(f->mode==3)DestroyWindow(f->frame.host.title);
}
static LRESULT hit(void *context,POINT point)
{(void)context;return point.x<20?HTMINBUTTON:HTCAPTION;}
static BOOL button(void *context,int part)
{(void)context;return part==HTMINBUTTON;}
static void invoke(void *context,int part,UINT message)
{(void)context;(void)part;(void)message;}
static BOOL menu_message(void *context,HWND w,UINT m,WPARAM a,LPARAM b,BOOL owner,LRESULT *r)
{(void)context;(void)w;(void)m;(void)a;(void)b;(void)owner;(void)r;return FALSE;}
static void dispose(void *context)
{++disposed;free(context);}
static const style_uwp_frame_ops operations={geometry,paint,hit,button,invoke,menu_message,dispose};
static void action(HWND w,void *context)
{(void)w;(void)context;}
static BOOL start(HWND owner,HWND title,int mode,const style_uwp_frame_action *actions,unsigned count,fixture **live)
{
    if(!style_uwp_frame_validate(owner,title,actions,count))return FALSE;
    fixture *f=calloc(1,sizeof(*f)+count*sizeof(*f->actions));
    if(!f){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    f->mode=mode;
    BOOL ok=style_uwp_frame_start(&f->frame,owner,title,&operations,f,actions,count,f->actions);
    if(ok && live)*live=f;
    return ok;
}
static DWORD WINAPI wrong_thread(void *context)
{
    HWND w=context;
    if(style_uwp_frame_validate(w,NULL,NULL,0) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 1;
    if(style_uwp_frame_remove(w) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 2;
    if(style_uwp_frame_surface(w) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 3;
    return 0;
}
int main(void)
{
    HWND w=CreateWindowW(L"STATIC",L"Independent modern provider",WS_OVERLAPPEDWINDOW,
        30,30,360,240,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    RECT client;CHECK(GetClientRect(w,&client));
    HWND title=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,0,0,client.right,32,w,NULL,NULL,NULL);CHECK(title);
    style_uwp_frame_action actions[256];
    for(unsigned i=0;i<256;++i)actions[i]=(style_uwp_frame_action){L"Owned action",action,w};
    CHECK(!style_uwp_frame_validate(w,title,NULL,1) && GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(!style_uwp_frame_validate(w,title,actions,257) && GetLastError()==ERROR_INVALID_PARAMETER);
    style_uwp_frame_action invalid={L"",action,NULL};
    CHECK(!style_uwp_frame_validate(w,title,&invalid,1) && GetLastError()==ERROR_INVALID_PARAMETER);
    HWND other=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,100,100,NULL,NULL,NULL,NULL);CHECK(other);
    CHECK(!style_uwp_frame_validate(other,title,NULL,0) && GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(DestroyWindow(other));
    HANDLE thread=CreateThread(NULL,0,wrong_thread,w,0,NULL);CHECK(thread);
    CHECK(WaitForSingleObject(thread,5000)==WAIT_OBJECT_0);
    DWORD code;CHECK(GetExitCodeThread(thread,&code) && !code && CloseHandle(thread));
    fixture *f=NULL;
    CHECK(start(w,title,0,actions,256,&f));
    CHECK(!painted && f->frame.action_count==256 && f->frame.actions[255].value.context==w);
    actions[255].context=NULL;CHECK(f->frame.actions[255].value.context==w);
    CHECK(!style_uwp_frame_validate(w,title,NULL,0) && GetLastError()==ERROR_ALREADY_EXISTS);
    HWND surface=style_uwp_frame_surface(w);CHECK(surface && !IsWindowVisible(surface));
    CHECK(style_uwp_frame_remove(w) && disposed==1 && !IsWindow(surface));
    CHECK(!style_uwp_frame_remove(w) && GetLastError()==ERROR_NOT_FOUND);
    ShowWindow(w,SW_SHOWNOACTIVATE);
    CHECK(!start(w,title,1,NULL,0,NULL) && GetLastError()==1234 && disposed==2);
    CHECK(!style_uwp_frame_surface(w));
    CHECK(!start(w,title,2,NULL,0,NULL) && disposed==3 && !style_uwp_frame_surface(w));
    CHECK(!start(w,title,3,NULL,0,NULL) && disposed==4 && !style_uwp_frame_surface(w) && !IsWindow(title));
    title=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,0,0,client.right,32,w,NULL,NULL,NULL);CHECK(title);
    for(unsigned i=0;i<3;++i){
        CHECK(start(w,title,0,NULL,0,&f));surface=style_uwp_frame_surface(w);
        CHECK(surface && IsWindowVisible(surface) && f->frame.bitmap.pixels);
        CHECK(f->frame.bitmap.pixels[3]==255);
        SendMessageW(w,WM_NCACTIVATE,FALSE,0);CHECK(!f->frame.active);
        SendMessageW(w,WM_NCACTIVATE,TRUE,0);CHECK(f->frame.active);
        ShowWindow(w,SW_HIDE);CHECK(!IsWindowVisible(surface));
        ShowWindow(w,SW_SHOWNOACTIVATE);CHECK(IsWindowVisible(surface));
        CHECK(style_uwp_frame_remove(w) && !IsWindow(surface) && disposed==5+i);
    }
    CHECK(start(w,title,0,NULL,0,NULL));surface=style_uwp_frame_surface(w);
    CHECK(DestroyWindow(w) && !IsWindow(surface) && disposed==8);
    puts("MODERN FRAME PASS independent_provider=1 actions_256=1 copy=1 validation=1 thread=1 failure=1 reentrant_remove=1 reentrant_destroy=1 activation=1 repeat=3 destroy=1");
    return 0;
}
