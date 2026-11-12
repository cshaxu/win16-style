#include "lib-style/win16-control/list_paint.h"
#include "lib-style/common/text_metrics.h"
#include <stdio.h>
#include <stdlib.h>
static LRESULT forward(HWND w,WPARAM count,LPARAM values)
{return SendMessageW(w,LB_SETTABSTOPS,count,values);}
#define CHECK(x) do { if(!(x)){fprintf(stderr,"LIST_TABS_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,240,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND list=root?CreateWindowW(L"LISTBOX",L"",WS_CHILD|LBS_USETABSTOPS,0,0,200,80,root,NULL,GetModuleHandleW(NULL),NULL):NULL;
    HFONT font=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Arial");
    CHECK(root && list && font);
    HDC dc=GetDC(list);CHECK(dc);HGDIOBJ old=SelectObject(dc,font);TEXTMETRICW metrics;int average;
    CHECK(old && GetTextMetricsW(dc,&metrics));average=metrics.tmAveCharWidth;
    if(metrics.tmPitchAndFamily&TMPF_FIXED_PITCH)CHECK(style_text_alphabet_average(dc,&average));
    SelectObject(dc,old);ReleaseDC(list,dc);
    BOOL detached=FALSE;int count=0,*positions=NULL,stops[]={32,64};
    win16_control_tabstops_view view={&detached,&font,&count,&positions,forward};
    CHECK(win16_control_set_list_tabs(list,&view,2,(LPARAM)stops));
    CHECK(count==2 && positions && positions[0]==(32*average+2)/4 && positions[1]==(64*average+2)/4);
    int *saved=positions;CHECK(!win16_control_set_list_tabs(list,&view,1,0) && positions==saved && count==2);
    CHECK(win16_control_set_list_tabs(list,&view,0,0) && !positions && !count);
    CHECK(DeleteObject(font) && DestroyWindow(root));
    puts("LIST_TABS_OK proportional=1 forwarding=1 invalid_preserves=1 cleanup=1");return 0;
}
