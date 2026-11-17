#include "lib-win32-style/application/control/status_paint.h"
#include <uxtheme.h>
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STATUS IDENTITY FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_status_state state;
static unsigned action;
static BOOL text_supported(LPCWSTR text,int count){return count>0 && text[0]!=L'#';}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;LRESULT result=DefSubclassProc(w,m,a,b);
    if(m==SB_GETRECT && action){unsigned now=action;action=0;if(now==1)++state.epoch;else instance.detached=TRUE;}
    return result;
}
int main(void)
{
    BOOL invalid=FALSE;
    style_control_status_invalidate(WM_PAINT,&invalid,&state);CHECK(!invalid && !state.epoch);
    style_control_status_invalidate(SB_SETTEXTW,&invalid,&state);CHECK(!invalid && state.epoch==1);
    style_control_status_invalidate(WM_THEMECHANGED,&invalid,&state);CHECK(invalid && state.epoch==1);
    invalid=FALSE;style_control_status_invalidate(WM_SETFONT,&invalid,&state);CHECK(invalid && state.epoch==2);
    INITCOMMONCONTROLSEX init={.dwSize=sizeof(init),.dwICC=ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND status=CreateWindowW(STATUSCLASSNAMEW,L"",WS_CHILD,0,0,200,20,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(status);
    HMODULE native=NULL;CHECK(style_control_status_identity(status,NULL,NULL,NULL,&native) && native);
    int parts[]={80,160};CHECK(SendMessageW(status,SB_SETPARTS,2,(LPARAM)parts));
    CHECK(SendMessageW(status,SB_SETTEXTW,0,(LPARAM)L"First"));
    CHECK(SendMessageW(status,SB_SETTEXTW,1|SBT_NOBORDERS,(LPARAM)L"Second"));
    state.epoch=3;style_control_status_snapshot snapshot;
    style_control_status_read(status,&instance,&state,NULL,&snapshot);
    CHECK(!snapshot.simple && snapshot.text_count==2 && snapshot.count==1 && EqualRect(&snapshot.text_panes[0],&snapshot.panes[0]));
    instance.detached=TRUE;style_control_status_read(status,&instance,&state,NULL,&snapshot);
    CHECK(!snapshot.simple && !snapshot.text_count && !snapshot.count);
    instance.detached=FALSE;CHECK(SetWindowSubclass(status,observe,1,0));
    for(unsigned mode=1;mode<=2;++mode){action=mode;unsigned old_epoch=state.epoch;
        style_control_status_read(status,&instance,&state,NULL,&snapshot);
        CHECK(!snapshot.text_count && !snapshot.count && snapshot.epoch==old_epoch);
        CHECK(mode==1?state.epoch!=snapshot.epoch:instance.detached);instance.detached=FALSE;
    }
    CHECK(RemoveWindowSubclass(status,observe,1));
    /* Exercise the production scope against native messages/DCs, no renderer. */
    CHECK(SendMessageW(status,SB_SETTEXTW,1,(LPARAM)L"Second"));
    SetWindowLongPtrW(status,GWL_STYLE,GetWindowLongPtrW(status,GWL_STYLE)|SBARS_SIZEGRIP);
    state.candidate=TRUE;
    style_control_status_scope paint={.context=&instance,.window=status,.instance=&instance,.state=&state};
    style_control_status_read(status,&instance,&state,NULL,&paint.snapshot);CHECK(paint.snapshot.count==2);
    HDC dc=GetDC(status);CHECK(dc);const void *native_caller=(const void *)GetProcAddress(native,"InitCommonControlsEx");CHECK(native_caller);
    HMODULE theme_module=LoadLibraryW(L"uxtheme.dll");CHECK(theme_module);const void *theme_caller=(const void *)GetProcAddress(theme_module,"GetThemeColor");CHECK(theme_caller);
    RECT pane=paint.snapshot.panes[1],text=pane,result;InflateRect(&text,-1,-1);
    CHECK(style_control_status_text_frame(&paint,dc,&text,0,native_caller,&result) && EqualRect(&result,&pane));
    CHECK(!style_control_status_text_frame(&paint,dc,&text,DT_CALCRECT,native_caller,&result));
    CHECK(!style_control_status_text_frame(&paint,dc,&text,0,main,&result));
    CHECK(style_control_status_frame(&paint,dc,&pane,native_caller,FALSE,NULL));
    CHECK(!style_control_status_frame(&paint,dc,&pane,native_caller,TRUE,parent));
    CHECK(!style_control_status_frame(&paint,dc,&pane,native_caller,TRUE,NULL));
    CHECK(style_control_status_frame(&paint,dc,&pane,native_caller,TRUE,status));
    HICON icon=LoadIconW(NULL,MAKEINTRESOURCEW(32512));CHECK(icon);
    CHECK(SendMessageW(status,SB_SETICON,1,(LPARAM)icon));
    CHECK(!style_control_status_text_frame(&paint,dc,&text,0,native_caller,&result));
    CHECK(style_control_status_icon_frame(&paint,dc,pane.left,pane.top,16,16,icon,native_caller,&result) && EqualRect(&result,&pane));
    CHECK(!style_control_status_icon_frame(&paint,dc,pane.left,pane.top,0,16,icon,native_caller,&result));
    CHECK(SendMessageW(status,SB_SETICON,1,0));
    style_control_status_themes themes={0};style_control_status_theme outer,nested;
    CHECK(!style_control_status_theme_begin(&themes,&outer,NULL,dc,0,0,L"First",-1,0,&text,native_caller,text_supported));
    CHECK(style_control_status_theme_begin(&themes,&outer,&paint,dc,0,0,L"First",-1,0,&text,native_caller,text_supported));
    CHECK(themes.current==&outer && outer.valid && outer.paint==&paint && outer.length==5);
    CHECK(style_control_status_text_matches(&paint,&outer,dc,L"First",-1,0,theme_caller,text_supported));
    CHECK(!style_control_status_text_matches(&paint,&outer,dc,L"First",-1,0,native_caller,text_supported));
    style_control_status_scope replaced=paint;CHECK(!style_control_status_text_matches(&replaced,&outer,dc,L"First",-1,0,theme_caller,text_supported));
    CHECK(style_control_status_theme_begin(&themes,&nested,NULL,dc,0,0,L"First",-1,0,&text,native_caller,text_supported) && !nested.valid);
    CHECK(!style_control_status_text_matches(&paint,themes.current,dc,L"First",-1,0,theme_caller,text_supported));
    style_control_status_theme_end(&themes,&outer);CHECK(themes.current==&nested);
    style_control_status_theme_end(&themes,&nested);CHECK(themes.current==&outer);
    style_control_status_theme_end(&themes,&outer);CHECK(!themes.current);
    WCHAR copied[]={0x4e2d,0};
    CHECK(style_control_status_theme_begin(&themes,&outer,&paint,dc,0,0,copied,-1,0,&text,native_caller,text_supported));
    CHECK(outer.valid && outer.text[0]==0x4e2d);copied[0]=L'A';CHECK(outer.text[0]==0x4e2d);
    style_control_status_theme_end(&themes,&outer);
    CHECK(style_control_status_theme_begin(&themes,&outer,&paint,dc,0,0,L"#no",-1,0,&text,native_caller,text_supported) && !outer.valid);
    style_control_status_theme_end(&themes,&outer);
    WCHAR boundary[514];for(unsigned i=0;i<513;++i)boundary[i]=L'A';boundary[513]=0;
    CHECK(style_control_status_theme_begin(&themes,&outer,&paint,dc,0,0,boundary,512,0,&text,native_caller,text_supported) && outer.valid && outer.length==512);
    CHECK(style_control_status_text_matches(&paint,&outer,dc,boundary,512,0,theme_caller,text_supported));style_control_status_theme_end(&themes,&outer);
    CHECK(style_control_status_theme_begin(&themes,&outer,&paint,dc,0,0,boundary,-1,0,&text,native_caller,text_supported) && !outer.valid);style_control_status_theme_end(&themes,&outer);
    ++state.epoch;CHECK(!style_control_status_frame(&paint,dc,&pane,native_caller,FALSE,NULL));
    CHECK(!style_control_status_text_frame(&paint,dc,&text,0,native_caller,&result));--state.epoch;
    instance.detached=TRUE;CHECK(!style_control_status_frame(&paint,dc,&pane,native_caller,FALSE,NULL));instance.detached=FALSE;
    CHECK(ReleaseDC(status,dc));CHECK(FreeLibrary(theme_module));
    puts("STATUS_SCOPE PASS native_queries=1 text_icon=1 dc=1 caller=1 epoch=1 detached=1 pair=1 nested_barrier=1 copied_text=1 policy=1 bounds=1 cleanup=1");

    WNDCLASSEXW cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,STATUSCLASSNAMEW,&cls));
    cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"TestStatusAlias";cls.style&=~CS_GLOBALCLASS;
    CHECK(RegisterClassExW(&cls));HWND alias=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,200,20,parent,NULL,cls.hInstance,NULL);CHECK(alias);
    state=(style_control_status_state){.candidate=TRUE};
    CHECK(!style_control_status_identity(alias,&instance,&state,cls.lpszClassName,NULL));
    CHECK(!style_control_status_settle(alias,WM_PAINT,&instance,&state,cls.lpszClassName) && !state.attempted);
    state.witness_owner=cls.hInstance;SetLastError(0x1234);
    CHECK(!style_control_status_settle(alias,WM_PAINT,&instance,&state,L"WrongAlias") && state.attempted && !state.installed && GetLastError()==0x1234);
    CHECK(!style_control_status_settle(alias,WM_PAINT,&instance,&state,cls.lpszClassName) && !state.installed);
    state.attempted=FALSE;SetLastError(0x5678);
    CHECK(style_control_status_settle(alias,WM_PAINT,&instance,&state,cls.lpszClassName) && state.installed && state.parent==parent && GetLastError()==0x5678);
    CHECK(!style_control_status_settle(alias,WM_PAINT,&instance,&state,cls.lpszClassName));
    state.parent=status;CHECK(!style_control_status_identity(alias,&instance,&state,cls.lpszClassName,NULL));state.parent=parent;
    instance.detached=TRUE;CHECK(!style_control_status_identity(alias,&instance,&state,cls.lpszClassName,NULL));instance.detached=FALSE;
    CHECK(!style_control_status_identity(parent,NULL,NULL,NULL,NULL));
    LONG_PTR ex=GetWindowLongPtrW(status,GWL_EXSTYLE);SetWindowLongPtrW(status,GWL_EXSTYLE,ex|WS_EX_LAYOUTRTL);
    CHECK(!style_control_status_identity(status,NULL,NULL,NULL,NULL));SetWindowLongPtrW(status,GWL_EXSTYLE,ex);
    CHECK(style_control_status_identity(status,NULL,NULL,NULL,NULL));CHECK(DestroyWindow(parent));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("STATUS IDENTITY PASS native=1 module=1 wrong_class=1 rtl=1 owned_snapshot=1 epoch_retire=1 first_paint=1 no_retry=1 last_error=1 parent=1");return 0;
}
