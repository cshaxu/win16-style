#include "lib-style/common/control_status.h"
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STATUS IDENTITY FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
static BOOL identity(HWND window,void *owner,HMODULE *native)
{return window==(HWND)owner && style_control_status_identity(window,NULL,native);}
int main(void)
{
    BOOL invalid=FALSE;unsigned epoch=0;
    style_control_status_invalidate(WM_PAINT,&invalid,&epoch);CHECK(!invalid && !epoch);
    style_control_status_invalidate(SB_SETTEXTW,&invalid,&epoch);CHECK(!invalid && epoch==1);
    style_control_status_invalidate(WM_THEMECHANGED,&invalid,&epoch);CHECK(invalid && epoch==1);
    invalid=FALSE;style_control_status_invalidate(WM_SETFONT,&invalid,&epoch);CHECK(invalid && epoch==2);
    INITCOMMONCONTROLSEX init={.dwSize=sizeof(init),.dwICC=ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND status=CreateWindowW(STATUSCLASSNAMEW,L"",WS_CHILD,0,0,200,20,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(status);
    HMODULE native=NULL;CHECK(style_control_status_identity(status,NULL,&native) && native);
    int parts[]={80,160};CHECK(SendMessageW(status,SB_SETPARTS,2,(LPARAM)parts));
    CHECK(SendMessageW(status,SB_SETTEXTW,0,(LPARAM)L"First"));
    CHECK(SendMessageW(status,SB_SETTEXTW,1|SBT_NOBORDERS,(LPARAM)L"Second"));
    BOOL detached=FALSE,simple=FALSE;unsigned snapshot_epoch=3,text_count=0,count=0;
    int simple_length=0;WCHAR simple_text[513]={0};RECT text_panes[64],panes[64];
    style_control_status_snapshot_view snapshot={status,&detached,&snapshot_epoch,3,&native,&simple,
        &simple_length,simple_text,&text_count,text_panes,&count,panes,identity};
    style_control_status_snapshot(status,&snapshot);
    CHECK(!simple && text_count==2 && count==1 && EqualRect(&text_panes[0],&panes[0]));
    detached=TRUE;text_count=count=0;style_control_status_snapshot(status,&snapshot);
    CHECK(!simple && !text_count && !count);
    CHECK(!style_control_status_identity(parent,NULL,NULL));
    LONG_PTR ex=GetWindowLongPtrW(status,GWL_EXSTYLE);SetWindowLongPtrW(status,GWL_EXSTYLE,ex|WS_EX_LAYOUTRTL);
    CHECK(!style_control_status_identity(status,NULL,NULL));SetWindowLongPtrW(status,GWL_EXSTYLE,ex);
    CHECK(style_control_status_identity(status,NULL,NULL));CHECK(DestroyWindow(parent));
    puts("STATUS IDENTITY PASS native=1 module=1 wrong_class=1 rtl=1");return 0;
}
