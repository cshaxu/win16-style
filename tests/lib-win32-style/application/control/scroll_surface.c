#include "lib-win32-style/application/control/scroll_surface.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SCROLL_SURFACE line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,0,0,280,240,NULL,NULL,module,NULL);
    HWND child=CreateWindowW(L"SCROLLBAR",L"",WS_CHILD|WS_VISIBLE|SBS_VERT,10,10,20,120,root,NULL,module,NULL);CHECK(root && child);
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=30};
    SetScrollInfo(root,SB_HORZ,&info,FALSE);SetScrollInfo(root,SB_VERT,&info,FALSE);SetScrollInfo(child,SB_CTL,&info,FALSE);
    ShowWindow(root,SW_SHOWNOACTIVATE);UpdateWindow(root);
    style_scroll_surface surface;RECT bounds;
    CHECK(style_scroll_surface_read(child,OBJID_CLIENT,&surface) && GetClientRect(child,&bounds));
    CHECK(EqualRect(&surface.bar,&bounds) && EqualRect(&surface.target,&bounds) && !surface.origin.x && !surface.origin.y);
    for(unsigned i=0;i<2;++i){
        LONG object=i?OBJID_VSCROLL:OBJID_HSCROLL;CHECK(style_scroll_surface_read(root,object,&surface) && GetWindowRect(root,&bounds));
        CHECK(surface.origin.x==surface.info.rcScrollBar.left-bounds.left && surface.origin.y==surface.info.rcScrollBar.top-bounds.top);
        RECT expected=surface.info.rcScrollBar;OffsetRect(&expected,-bounds.left,-bounds.top);
        CHECK(EqualRect(&expected,&surface.target) && surface.bar.left==0 && surface.bar.top==0 && surface.bar.right==expected.right-expected.left && surface.bar.bottom==expected.bottom-expected.top);
    }
    CHECK(ShowScrollBar(root,SB_VERT,FALSE) && !style_scroll_surface_read(root,OBJID_VSCROLL,&surface));
    CHECK(!style_scroll_surface_read(NULL,OBJID_CLIENT,&surface) && DestroyWindow(root));
    puts("SCROLL_SURFACE real client/embedded bounds and DC origins, hidden/invalid native surface rejection PASS");return 0;
}
