#include "lib-win32-style/application/control/header_snapshot.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"HEADER SNAPSHOT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_native_state state;
static unsigned action;
static LRESULT CALLBACK query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;LRESULT result=DefSubclassProc(w,m,a,b);
    if(m==HDM_GETITEMW && action){unsigned now=action;action=0;if(now==1)++state.epoch;else if(now==2)instance.detached=TRUE;else ((HDITEMW *)b)->pszText=L"foreign";}
    return result;
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_LISTVIEW_CLASSES};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND header=CreateWindowW(WC_HEADERW,L"",WS_CHILD,0,0,250,26,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(header);
    WCHAR text[651];for(unsigned i=0;i<650;++i)text[i]=i%2?L'X':0x4e2d;text[650]=0;
    HDITEMW item={.mask=HDI_WIDTH|HDI_TEXT|HDI_FORMAT|HDI_LPARAM,.cxy=120,.pszText=text,.fmt=HDF_STRING,.lParam=0x1234};
    CHECK(SendMessageW(header,HDM_INSERTITEMW,0,(LPARAM)&item)==0);item.pszText=L"Second";CHECK(SendMessageW(header,HDM_INSERTITEMW,1,(LPARAM)&item)==1);
    style_header_snapshot snapshot;style_control_native_init(&state);UINT allowed=HDF_STRING|HDF_JUSTIFYMASK|HDF_RTLREADING;
    CHECK(style_header_snapshot_read(header,&instance,&state,allowed,&snapshot));CHECK(snapshot.count==2 && snapshot.items && !wcscmp(snapshot.items[0].text,text) && snapshot.items[0].rect.right>0);
    style_header_snapshot_clear(&snapshot);CHECK(!snapshot.count && !snapshot.items);
    item=(HDITEMW){.mask=HDI_FORMAT,.fmt=HDF_STRING|HDF_OWNERDRAW};CHECK(SendMessageW(header,HDM_SETITEMW,1,(LPARAM)&item));
    CHECK(!style_header_snapshot_read(header,&instance,&state,allowed,&snapshot) && !snapshot.items && !snapshot.count);
    CHECK(style_header_snapshot_read(header,&instance,&state,allowed|HDF_OWNERDRAW,&snapshot));style_header_snapshot_clear(&snapshot);
    item=(HDITEMW){.mask=HDI_FORMAT,.fmt=HDF_STRING};CHECK(SendMessageW(header,HDM_SETITEMW,1,(LPARAM)&item));
    CHECK(SetWindowSubclass(header,query,1,0));
    for(action=1;action<=3;){unsigned next=action+1;CHECK(!style_header_snapshot_read(header,&instance,&state,allowed,&snapshot));CHECK(!snapshot.items && !snapshot.count);instance.detached=FALSE;action=next;}
    item=(HDITEMW){.mask=HDI_LPARAM};CHECK(SendMessageW(header,HDM_GETITEMW,0,(LPARAM)&item) && item.lParam==0x1234);
    CHECK(DestroyWindow(parent));puts("HEADER SNAPSHOT PASS long_unicode=1 policy=1 partial_clear=1 epoch=1 retire=1 foreign_text=1 itemdata=1 cleanup=1");return 0;
}
