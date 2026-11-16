#include "lib-win32-style/application/control/scroll_managed.h"
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MANAGED line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;style_scroll_controller scroll;unsigned draws,disposed;} fixture;
static fixture first,second;
static BOOL replace,retire_on_draw,allowed=TRUE,native_redraw,pinned;
static unsigned removes;
static const style_scroll_managed_ops operations;
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static void dispose(void *context){++((fixture *)context)->disposed;SetLastError(999);}
static style_scroll_binding lookup(HWND w)
{
    style_instance *instance=style_instance_lookup(w,procedure);
    return instance?(style_scroll_binding){instance,&((fixture *)instance->context)->scroll}:(style_scroll_binding){0};
}
static BOOL remove_binding(HWND w)
{
    style_scroll_binding binding=lookup(w);if(!binding.instance)return FALSE;
    ++removes;style_instance_retain(binding.instance);BOOL result=style_instance_detach(binding.instance);style_instance_release(binding.instance);return result;
}
static BOOL get_info(HWND w,int bar,LPSCROLLINFO info)
{(void)w;(void)bar;info->nTrackPos=11;SetLastError(123);return TRUE;}
static int set_info(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{
    (void)bar;(void)info;native_redraw=redraw;
    if(replace){replace=FALSE;(void)remove_binding(w);(void)style_instance_attach(&second.instance,w,procedure,NULL);}
    SetLastError(456);return 87;
}
static void draw(HWND w,void *context,HDC dc,LONG object,BOOL vertical)
{
    (void)dc;(void)object;(void)vertical;fixture *f=context;++f->draws;pinned=f->instance.calls>0;
    if(retire_on_draw)style_scroll_managed_frame_remove(w,&operations);
    SetLastError(888);
}
static BOOL eligible(HWND w,style_scroll_role role){(void)w;(void)role;return allowed;}
static const style_scroll_managed_ops operations={lookup,remove_binding,get_info,set_info,draw,eligible};
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,200,140,NULL,NULL,module,NULL);
    HWND window=CreateWindowW(L"EDIT",L"",WS_CHILD|ES_MULTILINE,0,0,160,90,root,NULL,module,NULL);CHECK(root && window);
    style_instance_init(&first.instance,&first,dispose);style_instance_init(&second.instance,&second,dispose);
    CHECK(style_instance_attach(&first.instance,window,procedure,NULL));first.scroll.resources.frame=TRUE;first.scroll.object=OBJID_VSCROLL;
    CHECK(style_scroll_managed_active(window,&operations) && !style_scroll_managed_active(root,&operations));
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_TRACKPOS};first.scroll.reporting=TRUE;first.scroll.report=88;
    CHECK(style_scroll_managed_get_info(window,SB_VERT,&info,&operations) && info.nTrackPos==88 && GetLastError()==123);
    first.scroll.reporting=FALSE;first.scroll.track.active=TRUE;first.scroll.track.position=55;
    CHECK(style_scroll_managed_get_info(window,SB_VERT,&info,&operations) && info.nTrackPos==55 && GetLastError()==123);
    CHECK(style_scroll_managed_get_info(window,SB_HORZ,&info,&operations) && info.nTrackPos==11 && GetLastError()==123);
    allowed=FALSE;CHECK(!style_scroll_managed_enable(window,STYLE_SCROLL_LIST,&operations) && GetLastError()==ERROR_NOT_SUPPORTED && !first.scroll.resources.list);
    allowed=TRUE;CHECK(style_scroll_managed_enable(window,STYLE_SCROLL_EDIT,&operations) && first.scroll.resources.edit);
    first.scroll.resources.frame=FALSE;style_scroll_managed_frame_remove(window,&operations);CHECK(!removes);first.scroll.resources.frame=TRUE;
    replace=TRUE;
    CHECK(style_scroll_managed_set_info(window,SB_VERT,&info,TRUE,&operations)==87 && GetLastError()==456);
    CHECK(!native_redraw && first.disposed==1 && second.draws==1 && pinned && !first.draws && removes==1);
    second.scroll.resources.frame=TRUE;retire_on_draw=TRUE;
    CHECK(style_scroll_managed_set_info(window,SB_VERT,&info,TRUE,&operations)==87 && GetLastError()==456);
    CHECK(second.disposed==1 && second.draws==2 && pinned && removes==2 && !lookup(window).instance);
    CHECK(style_scroll_managed_set_info(window,SB_VERT,&info,TRUE,&operations)==87 && native_redraw && GetLastError()==456);
    CHECK(DestroyWindow(root));
    puts("MANAGED native errors/redraw, role policy, full track position, replacement lookup and draw retirement pin PASS");return 0;
}
