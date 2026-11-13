#include "lib-style/common/scroll_state.h"
#include <limits.h>
static BOOL read_info(HWND window,int bar,LPSCROLLINFO info)
{return GetScrollInfo(window,bar,info);}
static BOOL fail_info(HWND window,int bar,LPSCROLLINFO info)
{(void)window;(void)bar;(void)info;return FALSE;}
static LONG maximum=65535,minimum=0;static UINT page=1;
static BOOL range_info(HWND window,int bar,LPSCROLLINFO info)
{(void)window;if(bar!=SB_HORZ)return FALSE;info->nMin=minimum;info->nMax=maximum;info->nPage=page;return TRUE;}
typedef struct paint_state {HDC expected;unsigned count;BOOL valid;} paint_state;
static void paint(HWND window,void *context,HDC dc,LONG object,BOOL vertical)
{
    paint_state *state=context;(void)window;
    state->valid=state->valid && dc && (!state->expected || dc==state->expected) &&
        (state->count?object==OBJID_VSCROLL && vertical:object==OBJID_HSCROLL && !vertical);
    ++state->count;
}
int main(void)
{
    static const struct {WPARAM key;UINT command;} keys[]={
        {VK_LEFT,SB_LINEUP},{VK_UP,SB_LINEUP},{VK_RIGHT,SB_LINEDOWN},{VK_DOWN,SB_LINEDOWN},
        {VK_PRIOR,SB_PAGEUP},{VK_NEXT,SB_PAGEDOWN},{VK_HOME,SB_TOP},{VK_END,SB_BOTTOM}};
    for(unsigned i=0;i<ARRAYSIZE(keys);++i){UINT command=99;
        if(!style_scroll_key_command(keys[i].key,&command) || command!=keys[i].command)return 4;}
    UINT unchanged=99;if(style_scroll_key_command(VK_ESCAPE,&unchanged) || unchanged!=99)return 5;
    if(style_scroll_position(10,80,20,220,10)!=20 ||
       style_scroll_position(10,80,20,220,50)!=120 ||
       style_scroll_position(10,80,20,220,90)!=220 ||
       style_scroll_position(10,80,20,220,INT_MIN)!=20 ||
       style_scroll_position(10,80,20,220,INT_MAX)!=220 ||
       style_scroll_position(10,0,20,220,50)!=20 ||
       style_scroll_position(36,151,LONG_MIN,LONG_MAX,187)!=LONG_MAX)return 3;
    HINSTANCE instance=GetModuleHandleW(NULL);
    HWND root=CreateWindowW(L"STATIC",L"",WS_POPUP|WS_VSCROLL,0,0,120,100,NULL,NULL,instance,NULL);
    HWND scroll=root?CreateWindowW(L"SCROLLBAR",L"",WS_CHILD|SBS_HORZ,0,0,100,20,root,NULL,instance,NULL):NULL;
    if(!root || !scroll){if(root)DestroyWindow(root);return 1;}
    HWND list=CreateWindowW(L"LISTBOX",L"",WS_CHILD|LBS_MULTICOLUMN,0,0,80,40,root,NULL,instance,NULL);
    HWND edit=CreateWindowW(L"EDIT",L"one",WS_CHILD|ES_MULTILINE,0,0,80,40,root,NULL,instance,NULL);
    BOOL protocol=list && edit && style_scroll_horizontal_thumb_messages(list,range_info) &&
        !style_scroll_horizontal_thumb_messages(list,fail_info) &&
        style_scroll_list_axis_owned(list,OBJID_HSCROLL,range_info) &&
        !style_scroll_list_axis_owned(list,OBJID_VSCROLL,range_info) &&
        style_scroll_edit_thumb_messages(edit,OBJID_VSCROLL,range_info);
    maximum=65536;protocol=protocol && !style_scroll_horizontal_thumb_messages(list,range_info) &&
        !style_scroll_edit_thumb_messages(edit,OBJID_HSCROLL,range_info);
    page=2;protocol=protocol && style_scroll_horizontal_thumb_messages(list,range_info);
    minimum=-1;protocol=protocol && !style_scroll_horizontal_thumb_messages(list,range_info);
    paint_state absent={NULL,0,TRUE};style_scroll_nonclient_paint(scroll,&absent,NULL,paint);
    SetWindowLongPtrW(root,GWL_STYLE,GetWindowLongPtrW(root,GWL_STYLE)|WS_HSCROLL);
    HDC target=GetWindowDC(root);paint_state painted={target,0,TRUE};
    style_scroll_nonclient_paint(root,&painted,target,paint);
    BOOL axes=absent.count==0 && target && painted.valid && painted.count==2;
    if(target)ReleaseDC(root,target);
    painted=(paint_state){NULL,0,TRUE};style_scroll_nonclient_paint(root,&painted,NULL,paint);
    axes=axes && painted.valid && painted.count==2;
    SCROLLINFO setting={.cbSize=sizeof(setting),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,
        .nMin=4,.nMax=104,.nPage=10,.nPos=25};
    SetScrollInfo(scroll,SB_CTL,&setting,FALSE);
    SCROLLINFO info;RECT bar;POINT origin;UINT dpi;
    BOOL ok=protocol && axes && style_scroll_control_read(scroll,read_info,&bar,&info,&dpi) &&
        bar.right-bar.left==100 && bar.bottom-bar.top==20 && dpi &&
        info.nMin==4 && info.nMax==104 && info.nPage==10 && info.nPos==25 &&
        !style_scroll_control_read(scroll,fail_info,&bar,&info,&dpi);
    SetScrollInfo(root,SB_VERT,&setting,FALSE);ShowWindow(root,SW_SHOWNOACTIVATE);
    ok=ok && style_scroll_embedded_read(root,OBJID_VSCROLL,read_info,&bar,&origin,&info,&dpi) &&
        bar.right>0 && bar.bottom>0 && info.nMin==4 && info.nMax==104 && info.nPos==25 &&
        !style_scroll_embedded_read(root,OBJID_VSCROLL,fail_info,&bar,&origin,&info,&dpi);
    UINT_PTR timer=SetTimer(root,1,10000,NULL);ok=ok && timer!=0;
    style_scroll_stop_timer(root,&timer);ok=ok && !timer;
    HWND previous=root;
    SetCapture(scroll);
    if(GetCapture()==scroll){
        style_scroll_release_capture(scroll,&previous,TRUE);
        ok=ok && !previous && GetCapture()==root;
        ReleaseCapture();
    }else ok=FALSE;
    previous=root;style_scroll_release_capture(scroll,&previous,FALSE);
    ok=ok && !previous && GetCapture()!=scroll;
    DestroyWindow(root);return ok?0:2;
}
