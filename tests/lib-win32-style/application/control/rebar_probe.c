#include "lib-win32-style/application/control/rebar.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"REBAR_PROBE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static WNDPROC native;
static UINT messages[8];
static unsigned count,checks,stop;
static BOOL ignored_return;
static int owner;
static BOOL valid(HWND w,void *context)
{return context==&owner && IsWindow(w) && ++checks!=stop;}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(count<ARRAYSIZE(messages))messages[count]=m;
    ++count;LRESULT result=CallWindowProcW(native,w,m,a,b);
    return ignored_return && m==RB_GETBANDBORDERS?0:result;
}
static void reset(unsigned fail){count=checks=0;stop=fail;}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_COOL_CLASSES|ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&init));
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSEXW cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,REBARCLASSNAMEW,&cls));native=cls.lpfnWndProc;
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,150,NULL,NULL,module,NULL);CHECK(parent);
    HWND w=CreateWindowW(REBARCLASSNAMEW,L"",WS_CHILD|WS_VISIBLE|RBS_VARHEIGHT|CCS_NODIVIDER,0,0,240,60,parent,NULL,module,NULL);CHECK(w);
    HWND child=CreateWindowW(L"BUTTON",L"",WS_CHILD|WS_VISIBLE,0,0,60,20,w,NULL,module,NULL);CHECK(child);
    WCHAR empty[]=L"";REBARBANDINFOW band={.cbSize=REBARBANDINFOW_V3_SIZE,.fMask=RBBIM_ID|RBBIM_CHILD|RBBIM_CHILDSIZE|RBBIM_SIZE|RBBIM_STYLE|RBBIM_COLORS|RBBIM_TEXT,
        .fStyle=RBBS_GRIPPERALWAYS,.clrFore=CLR_DEFAULT,.clrBack=CLR_DEFAULT,.lpText=empty,.hwndChild=child,.cxMinChild=60,.cyMinChild=20,.cx=160,.wID=201};
    CHECK(SendMessageW(w,RB_INSERTBANDW,(WPARAM)-1,(LPARAM)&band));
    HDC dc=GetDC(w);CHECK(dc);NMCUSTOMDRAW draw={.hdc=dc,.dwItemSpec=201};style_control_rebar_scope scope={w,&owner,observe,valid};style_control_rebar_probe p,zero={0};
    const UINT expected[]={RB_IDTOINDEX,RB_GETRECT,RB_GETBANDINFOW,RB_GETBANDBORDERS,RB_GETBKCOLOR,RB_GETTEXTCOLOR};
    reset(0);CHECK(style_control_rebar_read(&scope,&draw,CCS_VERT,&p));CHECK(count==ARRAYSIZE(expected) && checks==7);
    for(unsigned i=0;i<count;++i)CHECK(messages[i]==expected[i]);
    CHECK(p.child==child && !p.bitmap && !p.text[0] && p.image==-1 && (p.style&RBBS_GRIPPERALWAYS));
    CHECK(style_control_rebar_geometry(&scope,&p));RECT client,child_rect;CHECK(GetClientRect(w,&client) && GetWindowRect(child,&child_rect));
    MapWindowPoints(NULL,w,(POINT *)&child_rect,2);CHECK(EqualRect(&p.client,&client) && EqualRect(&p.child_rect,&child_rect) && p.dpi==GetDpiForWindow(w));
    for(unsigned fail=2;fail<=7;++fail){reset(fail);CHECK(!style_control_rebar_read(&scope,&draw,CCS_VERT,&p));CHECK(count==fail-1 && checks==fail && !memcmp(&p,&zero,sizeof(p)));}
    reset(0);draw.dwItemSpec=999;CHECK(!style_control_rebar_read(&scope,&draw,0,&p) && count==1 && checks==1);draw.dwItemSpec=201;
    HDC other=GetDC(parent);CHECK(other);reset(0);draw.hdc=other;CHECK(!style_control_rebar_read(&scope,&draw,0,&p) && !count);CHECK(ReleaseDC(parent,other));draw.hdc=dc;
    int saved=SaveDC(dc);CHECK(saved && SetViewportOrgEx(dc,1,0,NULL));reset(0);CHECK(!style_control_rebar_read(&scope,&draw,0,&p) && !count);CHECK(RestoreDC(dc,saved));
    reset(0);CHECK(!style_control_rebar_read(&scope,&draw,WS_CHILD,&p) && !count);
    reset(0);ignored_return=TRUE;CHECK(style_control_rebar_read(&scope,&draw,0,&p) && count==6);ignored_return=FALSE;
    style_control_rebar_probe prior=p;reset(1);CHECK(!style_control_rebar_geometry(&scope,&p) && !memcmp(&p,&prior,sizeof(p)));
    p.child=NULL;reset(0);CHECK(!style_control_rebar_geometry(&scope,&p));CHECK(ReleaseDC(w,dc) && DestroyWindow(parent));
    puts("REBAR_PROBE PASS native=1 sequence=1 invalidation=1 dc=1 policy=1 geometry=1 ignored_return=1 cleanup=1");return 0;
}
