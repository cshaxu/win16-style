#include "lib-win32-style/application/control/tab_snapshot.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TAB SNAPSHOT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_native_state state;
static unsigned action,stage,items,failed;
static int order[3];
static HWND child;
static WCHAR title[651];
static HIMAGELIST images;
static BOOL borrowed;
static LRESULT CALLBACK query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(borrowed)failed=1;
    LRESULT result=DefSubclassProc(w,m,a,b);
    if(m==TCM_GETITEMW){
        if(((TCITEMW *)b)->mask&TCIF_PARAM)failed=1;
        if(action){unsigned now=action;action=0;if(now==1)++state.epoch;else if(now==2)instance.detached=TRUE;else return FALSE;}
    }
    if(m==TCM_GETIMAGELIST)borrowed=TRUE;
    return result;
}
static void surface(HWND w,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    (void)w;(void)context;stage=1;
    if(snapshot->count!=3 || PtVisible(dc,snapshot->page.left+1,snapshot->page.top+1) || PtVisible(dc,3,3))failed=1;
    SetTextColor(dc,RGB(1,2,3));
}
static void prepare(HWND w,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    (void)w;(void)context;(void)dc;
    if(stage!=1 || snapshot->selected!=1)failed=1;
    stage=2;
}
static void item(HWND w,void *context,HDC dc,const style_control_tab_snapshot *snapshot,const style_control_tab_item *value)
{
    (void)w;(void)context;(void)dc;(void)snapshot;
    if(stage!=2 || !borrowed || items>=3)failed=1;
    if(items<3)order[items]=value->index;
    ++items;borrowed=FALSE;
    if(value->index==0 && wcscmp(value->text,title))failed=1;
    if(value->active!=(value->index==1) || value->images!=images || !value->image || value->image_size.cx!=8 || value->image_size.cy!=8)failed=1;
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_TAB_CLASSES};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP|WS_VISIBLE,-10000,-10000,600,150,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND tab=CreateWindowW(WC_TABCONTROLW,L"",WS_CHILD|WS_VISIBLE|TCS_FIXEDWIDTH,0,0,550,140,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(tab);
    child=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,0,0,8,8,tab,NULL,GetModuleHandleW(NULL),NULL);CHECK(child);
    images=ImageList_Create(8,8,ILC_COLOR32,1,1);CHECK(images);
    HBITMAP bitmap=CreateBitmap(8,8,1,32,NULL);CHECK(bitmap && ImageList_Add(images,bitmap,NULL)==0);CHECK(DeleteObject(bitmap));
    SendMessageW(tab,TCM_SETIMAGELIST,0,(LPARAM)images);
    for(unsigned i=0;i<650;++i)title[i]=i%2?L'X':0x4e2d;
    title[650]=0;
    TCITEMW native={.mask=TCIF_TEXT|TCIF_IMAGE|TCIF_PARAM,.pszText=title,.iImage=0,.lParam=0x1234};
    CHECK(SendMessageW(tab,TCM_INSERTITEMW,0,(LPARAM)&native)==0);
    native.pszText=L"Second";CHECK(SendMessageW(tab,TCM_INSERTITEMW,1,(LPARAM)&native)==1);
    native.pszText=L"Third";CHECK(SendMessageW(tab,TCM_INSERTITEMW,2,(LPARAM)&native)==2);
    SendMessageW(tab,TCM_SETITEMSIZE,0,MAKELPARAM(120,25));
    SendMessageW(tab,TCM_SETCURSEL,1,0);CHECK(SetWindowSubclass(tab,query,1,0));
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HBITMAP canvas=CreateBitmap(600,150,1,32,NULL);CHECK(canvas);
    HGDIOBJ old=SelectObject(dc,canvas);CHECK(old);SetTextColor(dc,RGB(4,5,6));
    RECT before,after;GetClipBox(dc,&before);
    style_control_native_init(&state);const style_control_tab_painter painter={NULL,surface,prepare,item};
    CHECK(style_control_tab_paint(tab,dc,&instance,&state,&painter));
    CHECK(!failed && items==3 && order[0]==0 && order[1]==2 && order[2]==1);
    CHECK(GetTextColor(dc)==RGB(4,5,6));GetClipBox(dc,&after);CHECK(EqualRect(&before,&after));
    for(unsigned mode=1;mode<=3;++mode){action=mode;items=stage=0;CHECK(!style_control_tab_paint(tab,dc,&instance,&state,&painter));CHECK(!items && !borrowed);instance.detached=FALSE;CHECK(GetTextColor(dc)==RGB(4,5,6));}
    CHECK(RemoveWindowSubclass(tab,query,1));
    native=(TCITEMW){.mask=TCIF_PARAM};CHECK(SendMessageW(tab,TCM_GETITEMW,0,(LPARAM)&native) && native.lParam==0x1234);
    CHECK(SendMessageW(tab,TCM_GETIMAGELIST,0,0)==(LRESULT)images);borrowed=FALSE;
    CHECK(ImageList_GetImageCount(images)==1);CHECK(DestroyWindow(parent));CHECK(ImageList_Destroy(images));
    SelectObject(dc,old);CHECK(DeleteObject(canvas));CHECK(DeleteDC(dc));
    puts("TAB SNAPSHOT PASS unicode=1 selected_order=1 page_child_clip=1 borrowed_images=1 native_data=1 epoch_retire_failure=1 dc=1");return 0;
}
