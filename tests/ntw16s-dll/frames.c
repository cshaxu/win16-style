#include "ntw16s-dll/ntw16s.h"
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FRAME FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_APP+8)return 808;return DefWindowProcW(w,m,a,b);}
static void client_screen(HWND w,RECT *r)
{GetClientRect(w,r);MapWindowPoints(w,NULL,(POINT *)r,2);}

static int frame_worker(int argc,WCHAR **argv)
{
    static const DWORD styles[]={WS_OVERLAPPEDWINDOW,WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        WS_POPUP|WS_THICKFRAME,WS_POPUP|WS_BORDER,WS_POPUP,WS_POPUP|WS_CAPTION|WS_SYSMENU};
    static const DWORD excluded[]={WS_EX_CLIENTEDGE,WS_EX_TOOLWINDOW,WS_EX_LAYERED};
    WCHAR text[80];
    WNDCLASSW cls={0};unsigned i;HWND w;RECT before,after;win16_style_palette palette;
    (void)argc;(void)argv;
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    cls.lpfnWndProc=owner;cls.lpszClassName=L"Win16FrameForms";cls.hInstance=GetModuleHandleW(NULL);
    CHECK(RegisterClassW(&cls));
    for(i=0;i<sizeof(styles)/sizeof(styles[0]);++i){
        DWORD ex=i==5?WS_EX_DLGMODALFRAME:0;LRESULT hit=0;POINT center;
        w=CreateWindowExW(ex,cls.lpszClassName,L"Unchanged title",styles[i],100,100,420,280,
            NULL,NULL,cls.hInstance,NULL);CHECK(w);
        client_screen(w,&before);CHECK(win16_style_install(w,NULL));client_screen(w,&after);
        CHECK(EqualRect(&before,&after));
        CHECK(GetWindowTextW(w,text,80) && !wcscmp(text,L"Unchanged title"));
        CHECK(SendMessageW(w,WM_APP+8,0,0)==808);
        center.x=(after.left+after.right)/2;center.y=(after.top+after.bottom)/2;
        CHECK(win16_style_hit_test(w,MAKELPARAM(center.x,center.y),&hit) && hit==HTCLIENT);
        CHECK(SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(center.x,center.y))==hit);
        win16_style_default_palette(&palette);palette.active=RGB(0,0,128);
        CHECK(win16_style_set_palette(w,&palette));
        CHECK(SetWindowTextW(w,L"Dynamic native title") && GetWindowTextW(w,text,80) && !wcscmp(text,L"Dynamic native title"));
        if(i==0){
            HWND native=CreateWindowW(cls.lpszClassName,L"Native state oracle",styles[i],
                100,100,420,280,NULL,NULL,cls.hInstance,NULL);CHECK(native);
            /* Restoring a minimized, previously maximized window may retain
             * maximization. Compare USER's actual state, not a guessed rule. */
            ShowWindow(native,SW_SHOWMAXIMIZED);ShowWindow(w,SW_SHOWMAXIMIZED);
            CHECK(IsZoomed(w)==IsZoomed(native) && IsIconic(w)==IsIconic(native));
            ShowWindow(native,SW_MINIMIZE);ShowWindow(w,SW_MINIMIZE);
            CHECK(IsZoomed(w)==IsZoomed(native) && IsIconic(w)==IsIconic(native));
            ShowWindow(native,SW_RESTORE);ShowWindow(w,SW_RESTORE);
            CHECK(IsZoomed(w)==IsZoomed(native) && IsIconic(w)==IsIconic(native));
            ShowWindow(native,SW_SHOWNORMAL);ShowWindow(w,SW_SHOWNORMAL);
            CHECK(!IsIconic(native) && !IsZoomed(native) && !IsIconic(w) && !IsZoomed(w));
            CHECK(DestroyWindow(native));
        }
        client_screen(w,&before);CHECK(win16_style_remove(w));client_screen(w,&after);
        CHECK(EqualRect(&before,&after));CHECK(!win16_style_remove(w) && GetLastError()==ERROR_NOT_FOUND);
        CHECK(DestroyWindow(w));
    }
    for(i=0;i<sizeof(excluded)/sizeof(excluded[0]);++i){
        w=CreateWindowExW(excluded[i],cls.lpszClassName,L"Unsupported form",WS_OVERLAPPEDWINDOW,
            100,100,420,280,NULL,NULL,cls.hInstance,NULL);CHECK(w);
        client_screen(w,&before);CHECK(!win16_style_install(w,NULL) && GetLastError()==ERROR_NOT_SUPPORTED);
        client_screen(w,&after);CHECK(EqualRect(&before,&after) && DestroyWindow(w));
    }
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("FRAME PASS six forms, client screen preservation, same geometry hit test, title/palette, maximize/minimize/restore, three exclusions");
    return 0;
}

int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,frame_worker);}
