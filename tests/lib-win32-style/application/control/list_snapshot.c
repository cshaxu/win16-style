#include "lib-win32-style/application/control/list_snapshot.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LIST_SNAPSHOT line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;BOOL fail,stop,valid;unsigned rows,selected,columns,disposed;} fixture;
static void dispose(void *context){++((fixture *)context)->disposed;}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;
    if(m==LB_GETTEXTLEN && f->fail)return LB_ERR;
    if(m==LB_GETTEXT && f->stop)(void)style_instance_detach(&f->instance);
    return DefSubclassProc(w,m,a,b);
}
static HFONT font(HWND w,void *context){(void)w;(void)context;return (HFONT)GetStockObject(SYSTEM_FONT);}
static void begin(HWND w,void *context,HDC dc,const style_control_list_snapshot *snapshot)
{(void)w;fixture *f=context;f->valid=snapshot->count==20 && snapshot->font==(HFONT)GetStockObject(SYSTEM_FONT);SetTextColor(dc,RGB(10,20,30));}
static void item(HWND w,void *context,HDC dc,const style_control_list_snapshot *snapshot,const style_control_list_item *row)
{
    (void)w;(void)dc;(void)snapshot;fixture *f=context;++f->rows;
    f->valid=f->valid && row->length==3 && !wcscmp(row->text,L"row") && row->visible.right>row->visible.left;
    if(row->selected>0)++f->selected;
    if(row->row.left>0)++f->columns;
}
static BOOL attach(fixture *f,HWND window)
{f->valid=TRUE;style_instance_init(&f->instance,f,dispose);if(!style_instance_attach(&f->instance,window,procedure,NULL))return FALSE;style_instance_retain(&f->instance);return TRUE;}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,260,150,NULL,NULL,module,NULL);
    HWND single=CreateWindowW(L"LISTBOX",L"",WS_CHILD,0,0,200,80,root,NULL,module,NULL);
    HWND multi=CreateWindowW(L"LISTBOX",L"",WS_CHILD|LBS_MULTICOLUMN,0,0,200,80,root,NULL,module,NULL);CHECK(root && single && multi);
    for(unsigned i=0;i<20;++i){SendMessageW(single,LB_ADDSTRING,0,(LPARAM)L"row");SendMessageW(multi,LB_ADDSTRING,0,(LPARAM)L"row");}
    SendMessageW(single,LB_SETCURSEL,0,0);SendMessageW(multi,LB_SETCOLUMNWIDTH,70,0);
    fixture first={0},second={0};CHECK(attach(&first,single) && attach(&second,multi));
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);SetViewportOrgEx(dc,3,4,NULL);SetTextColor(dc,RGB(1,2,3));
    const style_control_list_painter paint={&first,font,begin,item};style_control_list_paint(single,&first.instance,dc,&paint);
    POINT origin;CHECK(first.rows>0 && first.rows<20 && first.selected==1 && first.valid);
    CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4 && GetTextColor(dc)==RGB(1,2,3));
    const style_control_list_painter columns={&second,font,begin,item};style_control_list_paint(multi,&second.instance,dc,&columns);
    CHECK(second.rows>0 && second.columns>0 && second.valid);
    first.fail=TRUE;first.rows=0;style_control_list_paint(single,&first.instance,dc,&paint);CHECK(!first.rows && !first.instance.detached);
    first.fail=FALSE;first.stop=TRUE;style_control_list_paint(single,&first.instance,dc,&paint);CHECK(!first.rows && first.instance.detached && !first.disposed);
    CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4 && GetTextColor(dc)==RGB(1,2,3));
    style_instance_release(&first.instance);CHECK(first.disposed==1);
    CHECK(style_instance_detach(&second.instance));style_instance_release(&second.instance);CHECK(second.disposed==1);
    CHECK(DeleteDC(dc) && DestroyWindow(root));
    puts("LIST_SNAPSHOT native rows/selection/text, multicolumn geometry, DC restore, failed read and callback retirement PASS");return 0;
}
