#include "lib-win32-style/application/control/month.h"
#include "../../../ntw16s-dll/exception_probe.h"
#include <vssym32.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MONTH_SCOPE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static HWND calendar;
static WNDPROC native_month;
static style_instance instance;
static style_runtime_rich_state rich;
static style_control_month_state state;
static style_control_month_scope scope;
static style_control_month_queries queries;
static unsigned mode,counts,headers,nested,failed;
static BOOL current=TRUE;
static BOOL scope_current(void *context)
{if(mode==5)RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);return context==&scope && current;}
static LRESULT CALLBACK month_window(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==MCM_GETCALENDARCOUNT){
        ++counts;if(mode==1)current=FALSE;
        if(mode==4 && !nested){++nested;unsigned before=counts;style_control_month_query(&scope,&queries);if(counts!=before)failed=1;}
    }
    if(m==MCM_GETCALENDARGRIDINFO && b){MCGRIDINFO *grid=(void *)b;
        if(grid->dwPart==MCGIP_CALENDARHEADER){++headers;if(mode==2)instance.detached=TRUE;}
        if(mode==3 && grid->dwPart==MCGIP_PREV)return FALSE;
    }
    return CallWindowProcW(native_month,w,m,a,b);
}
static LRESULT CALLBACK parent_window(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_APP+1){style_control_month_query(&scope,&queries);return 0;}return DefWindowProcW(w,m,a,b);}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_DATE_CLASSES};CHECK(InitCommonControlsEx(&init));HMODULE module=GetModuleHandleW(NULL);
    WNDCLASSEXW original={.cbSize=sizeof(original)};CHECK(GetClassInfoExW(NULL,MONTHCAL_CLASSW,&original));native_month=original.lpfnWndProc;
    WNDCLASSEXW alias=original;alias.hInstance=module;alias.lpfnWndProc=month_window;alias.lpszClassName=L"TMonthCalendar";alias.hbrBackground=NULL;alias.hIcon=alias.hIconSm=NULL;alias.lpszMenuName=NULL;CHECK(RegisterClassExW(&alias));
    WNDCLASSW cls={.hInstance=module,.lpfnWndProc=parent_window,.lpszClassName=L"MonthScopeParent"};CHECK(RegisterClassW(&cls));
    HWND parent=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,400,400,NULL,NULL,module,NULL);CHECK(parent);
    calendar=CreateWindowW(alias.lpszClassName,L"",WS_CHILD,0,0,250,200,parent,NULL,module,NULL);CHECK(calendar);
    RECT minimum;CHECK(SendMessageW(calendar,MCM_GETMINREQRECT,0,(LPARAM)&minimum));CHECK(SetWindowPos(calendar,NULL,0,0,minimum.right,minimum.bottom,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE));
    rich=(style_runtime_rich_state){.candidate=TRUE,.month=TRUE,.unicode=TRUE,.witness=TRUE,.parent=parent,.owner=module};
    scope=(style_control_month_scope){.context=&scope,.window=calendar,.instance=&instance,.rich=&rich,.state=&state,.rejected_styles=MCS_MULTISELECT,.current=scope_current};
    for(mode=0;mode<=4;++mode){
        current=TRUE;instance.detached=FALSE;state=(style_control_month_state){0};counts=headers=nested=failed=0;
        style_control_month_query(&scope,&queries);CHECK(!queries.active && !failed && counts>=1);
        if(mode==1)CHECK(!headers && !state.header_known && !state.header_valid);
        else if(mode==2)CHECK(headers>=1 && instance.detached && !state.header_known && !state.header_valid);
        else {CHECK(state.header_valid && state.header_known && state.header.right>state.header.left);CHECK(state.nav_valid==(mode!=3));}
        if(mode==4)CHECK(nested==1);
    }
    mode=5;current=TRUE;instance.detached=FALSE;CHECK(test_unwind(parent,WM_APP+1,0,0));CHECK(!queries.active);
    mode=0;style_control_month_query(&scope,&queries);CHECK(state.header_valid && state.nav_valid && !queries.active);
    HMODULE native=GetModuleHandleW(L"comctl32.dll");CHECK(native);const void *caller=(const void *)GetProcAddress(native,"InitCommonControlsEx");CHECK(caller);
    HDC dc=GetDC(calendar);CHECK(dc);HTHEME theme=GetWindowTheme(calendar);RECT cell={5,5,30,25},text=cell,clip={6,6,28,24};text.right-=2;
    style_control_month_selection selected;
    style_control_month_selection_begin(&scope,theme,dc,MC_GRIDCELLBACKGROUND,MCGCB_SELECTED,&cell,&clip,caller);CHECK(state.selected);
    CHECK(style_control_month_selection_take(&scope,theme,dc,MC_GRIDCELL,6,&text,0,caller,&selected) && !state.selected && EqualRect(&selected.rect,&cell) && selected.has_clip && EqualRect(&selected.clip,&clip));
    CHECK(!style_control_month_selection_take(&scope,theme,dc,MC_GRIDCELL,6,&text,0,caller,&selected));
    style_control_month_selection_begin(&scope,theme,dc,MC_GRIDCELLBACKGROUND,MCGCB_SELECTED,&cell,NULL,caller);CHECK(state.selected);
    CHECK(!style_control_month_selection_take(&scope,(HTHEME)((UINT_PTR)theme^0x100),dc,MC_GRIDCELL,6,&text,0,caller,&selected) && !state.selected);
    style_control_month_selection_begin(&scope,theme,dc,MC_GRIDCELLBACKGROUND,MCGCB_SELECTED,&cell,NULL,parent_window);CHECK(!state.selected);
    RECT title={4,17,221,46},previous={4,23,20,39},next={205,23,221,39};CHECK(style_control_month_title_fits(&title,&previous,&next,73));CHECK(!style_control_month_title_fits(&title,&previous,&next,210));CHECK(!style_control_month_title_fits(&title,&previous,&next,-1));
    CHECK(ReleaseDC(calendar,dc));CHECK(DestroyWindow(calendar));CHECK(DestroyWindow(parent));CHECK(UnregisterClassW(alias.lpszClassName,module));CHECK(UnregisterClassW(cls.lpszClassName,module));
    puts("MONTH_SCOPE PASS native_geometry=1 current=1 detached=1 partial_commit=1 nested=1 native_unwind=1 later_query=1 selection_pair=1 caller=1 title_fit=1 no_style=1 cleanup=1");return 0;
}
