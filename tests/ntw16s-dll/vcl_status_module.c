#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "../../src/ntw16s-dll/attachment.h"
#include "isolated_process.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"STATUS_CONTROL_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x); return 1; } } while (0)
#define WIDTH 512
typedef BOOL (WINAPI *fixture_reg_fn)(BOOL);
typedef HWND (WINAPI *fixture_create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *fixture_unreg_fn)(void);
static fixture_create_fn fixture_create;
static HWND create_status(HWND parent){HWND w=fixture_create(parent,FALSE);if(w)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);return w;}
#define HEIGHT 64

static unsigned owner_draws;
static ULONG_PTR owner_data;
static HWND status_window;
static BOOL (__cdecl *remove_style)(HWND);
static BOOL remove_in_draw;
static BOOL mutate_on_query;
static BOOL destroy_on_query;
static LRESULT CALLBACK app_subclass(HWND w,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(message==SB_GETRECT && destroy_on_query){destroy_on_query=FALSE;DestroyWindow(w);return 0;}
    if(message==SB_GETRECT && mutate_on_query){mutate_on_query=FALSE;SendMessageW(w,SB_SETTEXTW,0,(LPARAM)L"changed");}
    return DefSubclassProc(w,message,wp,lp);
}
static LRESULT CALLBACK parent_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_DRAWITEM){
        DRAWITEMSTRUCT *item=(void *)lp;
        if(item && item->hwndItem==status_window){
            ++owner_draws;owner_data=item->itemData;
            if(remove_in_draw){remove_in_draw=FALSE;remove_style(item->hwndItem);}
            HBRUSH brush=CreateSolidBrush(RGB(11,80,140));
            if(brush){FillRect(item->hDC,&item->rcItem,brush);DeleteObject(brush);}
            return TRUE;
        }
    }
    return DefWindowProcW(w,message,wp,lp);
}
static BOOL on_frame(int x,int y,const RECT *r)
{
    return x>=r->left && x<r->right && y>=r->top && y<r->bottom &&
        (x==r->left || x==r->right-1 || y==r->top || y==r->bottom-1);
}
static DWORD styled_pixels[WIDTH*HEIGHT],native_pixels[WIDTH*HEIGHT];
static DWORD live_styled[WIDTH*HEIGHT],live_native[WIDTH*HEIGHT];
static DWORD clipped_styled[WIDTH*HEIGHT],clipped_native[WIDTH*HEIGHT];
static void capture(HWND w,HDC dc,DWORD *bits,DWORD *out)
{
    RECT clear={0,0,WIDTH,HEIGHT};FillRect(dc,&clear,(HBRUSH)GetStockObject(WHITE_BRUSH));
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();
    memcpy(out,bits,sizeof(styled_pixels));
}
static BOOL capture_clip(HWND w,HDC dc,DWORD *bits,DWORD *out)
{
    RECT clear={0,0,WIDTH,HEIGHT};FillRect(dc,&clear,(HBRUSH)GetStockObject(WHITE_BRUSH));
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(IntersectClipRect(dc,0,0,60,HEIGHT)==ERROR){RestoreDC(dc,saved);return FALSE;}
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();memcpy(out,bits,sizeof(clipped_styled));
    return RestoreDC(dc,saved);
}
static BOOL capture_live(HWND w,HDC dc,DWORD *bits,DWORD *out,const RECT *r)
{
    HDC live=GetDC(w);if(!live)return FALSE;
    BOOL ok=BitBlt(dc,0,0,r->right,r->bottom,live,0,0,SRCCOPY);
    ReleaseDC(w,live);if(!ok)return FALSE;
    GdiFlush();memcpy(out,bits,sizeof(live_styled));return TRUE;
}
static int passive_case(const WCHAR *path)
{
    HMODULE dll=LoadLibraryW(path);CHECK(dll);
    ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void *)GetProcAddress(dll,"Ntw16sCreationBegin");
    BOOL (__cdecl *end)(ntw16s_creation_scope *,ntw16s_creation_result *)=(void *)GetProcAddress(dll,"Ntw16sCreationEnd");
    BOOL (__cdecl *is_attached)(HWND)=(void *)GetProcAddress(dll,"Ntw16sWindowAttached");CHECK(begin&&end&&is_attached);
    INITCOMMONCONTROLSEX ic={sizeof(ic),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&ic));
    HMODULE common=GetModuleHandleW(L"comctl32.dll");DLLVERSIONINFO version={.cbSize=sizeof(version)};
    HRESULT (CALLBACK *get_version)(DLLVERSIONINFO *)=(void *)GetProcAddress(common,"DllGetVersion");CHECK(get_version && SUCCEEDED(get_version(&version)));
#ifdef NTW16S_STATUS_V6
    CHECK(version.dwMajorVersion>=6);
#else
    CHECK(version.dwMajorVersion==5);
#endif
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,420,160,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
    HWND w=CreateWindowW(STATUSCLASSNAMEW,L"",WS_CHILD|WS_VISIBLE|CCS_BOTTOM,0,0,400,30,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    ntw16s_creation_result result={0};CHECK(end(scope,&result) && !result.failure && result.unsupported && !is_attached(w));
    CHECK(DestroyWindow(root));CHECK(FreeLibrary(dll));printf("STATUS_PASSIVE_OK bits=%u unattached=1 unsupported=%lu version=%lu.%lu\n",(unsigned)sizeof(void*)*8,result.unsupported,version.dwMajorVersion,version.dwMinorVersion);return 0;
}
static int worker(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"passive"))return passive_case(argv[2]);
    CHECK(argc==2 || (argc==3 && !wcscmp(argv[2],L"aware")));
    if(argc==3)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);
    fixture_reg_fn reg=(void*)GetProcAddress(fixture,"FixtureRegister");fixture_create=(void*)GetProcAddress(fixture,"FixtureCreate");fixture_unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");CHECK(reg && fixture_create && unreg);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    BOOL (__cdecl *attached)(HWND)=(void *)GetProcAddress(dll,"Ntw16sWindowAttached");
    remove_style=(void *)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove_style);
    INITCOMMONCONTROLSEX ic={sizeof(ic),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&ic));
    HMODULE common=GetModuleHandleW(L"comctl32.dll");DLLVERSIONINFO version={.cbSize=sizeof(version)};
    HRESULT (CALLBACK *get_version)(DLLVERSIONINFO *)=(void *)GetProcAddress(common,"DllGetVersion");CHECK(get_version && SUCCEEDED(get_version(&version)));
#ifdef NTW16S_STATUS_V6
    CHECK(version.dwMajorVersion>=6);
#else
    CHECK(version.dwMajorVersion==5);
#endif
    printf("STATUS_CONTROL_VERSION bits=%u major=%lu minor=%lu\n",(unsigned)sizeof(void*)*8,version.dwMajorVersion,version.dwMinorVersion);
    WNDCLASSW parent={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent_proc,.lpszClassName=L"StatusControlParent"};CHECK(RegisterClassW(&parent));
    HWND root=CreateWindowW(parent.lpszClassName,L"status protocol",WS_OVERLAPPEDWINDOW,0,0,430,180,NULL,NULL,parent.hInstance,NULL);CHECK(root && attached(root));ShowWindow(root,SW_SHOWNA);
    CHECK(reg(TRUE));HWND bogus=create_status(root);CHECK(bogus && !attached(bogus));CHECK(DestroyWindow(bogus) && unreg());CHECK(reg(FALSE));
    status_window=create_status(root);CHECK(status_window);
    if(!attached(status_window)){fprintf(stderr,"STATUS_CONTROL_UNATTACHED bits=%u\n",(unsigned)sizeof(void*)*8);return 1;}
    int edges[2]={120,-1};CHECK(SendMessageW(status_window,SB_SETPARTS,2,(LPARAM)edges));
    CHECK(SendMessageW(status_window,SB_SETTEXTW,0,(LPARAM)L"ordinary"));
    CHECK(SendMessageW(status_window,SB_SETTEXTW,1|SBT_OWNERDRAW,(LPARAM)0x12345678));
    HICON icon=LoadIconW(NULL,IDI_INFORMATION);CHECK(icon && SendMessageW(status_window,SB_SETICON,0,(LPARAM)icon));
    RECT a={0},b={0},client={0};CHECK(SendMessageW(status_window,SB_GETRECT,0,(LPARAM)&a));CHECK(SendMessageW(status_window,SB_GETRECT,1,(LPARAM)&b));
    CHECK(GetClientRect(status_window,&client) && client.right<=WIDTH && client.bottom<=HEIGHT);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=WIDTH;info.bmiHeader.biHeight=-HEIGHT;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    DWORD *bits=NULL;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,(void **)&bits,NULL,0);CHECK(bitmap&&bits);HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old);
    ShowWindow(root,SW_SHOWNA);unsigned before=owner_draws;capture(status_window,dc,bits,styled_pixels);CHECK(owner_draws==before+1 && owner_data==0x12345678);
    CHECK(RedrawWindow(status_window,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));CHECK(capture_live(status_window,dc,bits,live_styled,&client));
    before=owner_draws;CHECK(capture_clip(status_window,dc,bits,clipped_styled));CHECK(owner_draws==before);
    CHECK(remove_style(status_window) && !attached(status_window));before=owner_draws;capture(status_window,dc,bits,native_pixels);CHECK(owner_draws==before+1 && owner_data==0x12345678);
    CHECK(RedrawWindow(status_window,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW));CHECK(capture_live(status_window,dc,bits,live_native,&client));
    before=owner_draws;CHECK(capture_clip(status_window,dc,bits,clipped_native));CHECK(owner_draws==before);
    unsigned different=0,outside=0;for(int y=0;y<client.bottom;++y)for(int x=0;x<client.right;++x)if(styled_pixels[y*WIDTH+x]!=native_pixels[y*WIDTH+x]){++different;if(!on_frame(x,y,&a)&&!on_frame(x,y,&b))++outside;}
    CHECK(different>0 && outside==0);WCHAR text[32]={0};CHECK(LOWORD(SendMessageW(status_window,SB_GETTEXTW,0,(LPARAM)text))==8 && !wcscmp(text,L"ordinary"));CHECK((HICON)SendMessageW(status_window,SB_GETICON,0,0)==icon);
    unsigned live_changed=0,live_outside=0;for(int y=0;y<client.bottom;++y)for(int x=0;x<client.right;++x)if(live_styled[y*WIDTH+x]!=live_native[y*WIDTH+x]){++live_changed;if(!on_frame(x,y,&a)&&!on_frame(x,y,&b))++live_outside;}
    CHECK(live_changed>0 && live_outside==0);printf("STATUS_LIVE_OK bits=%u changed=%u outside=%u\n",(unsigned)sizeof(void*)*8,live_changed,live_outside);
    unsigned clip_changed=0,clip_outside=0;for(int y=0;y<HEIGHT;++y)for(int x=0;x<WIDTH;++x)if(clipped_styled[y*WIDTH+x]!=clipped_native[y*WIDTH+x]){++clip_changed;if(x>=60)++clip_outside;}
    CHECK(clip_changed>0 && clip_outside==0);printf("STATUS_CLIP_OK bits=%u changed=%u outside=%u\n",(unsigned)sizeof(void*)*8,clip_changed,clip_outside);
    printf("STATUS_CONTROL_OK bits=%u changed=%u outside=%u owner_data=%p icon_retained=1\n",(unsigned)sizeof(void*)*8,different,outside,(void *)owner_data);
    CHECK(DestroyWindow(status_window));status_window=NULL;
    const UINT native_modes[]={SBT_NOBORDERS,SBT_POPOUT};
    for(unsigned i=0;i<ARRAYSIZE(native_modes);++i){
        status_window=create_status(root);CHECK(status_window&&attached(status_window));
        CHECK(SendMessageW(status_window,SB_SETTEXTW,native_modes[i],(LPARAM)L"native"));capture(status_window,dc,bits,styled_pixels);
        CHECK(remove_style(status_window)&&!attached(status_window));capture(status_window,dc,bits,native_pixels);
        CHECK(!memcmp(styled_pixels,native_pixels,sizeof(styled_pixels)));CHECK(DestroyWindow(status_window));status_window=NULL;
    }
    status_window=create_status(root);CHECK(status_window&&attached(status_window));
    SendMessageW(status_window,SB_SIMPLE,TRUE,0);CHECK(SendMessageW(status_window,SB_ISSIMPLE,0,0));CHECK(SendMessageW(status_window,SB_SETTEXTW,SB_SIMPLEID,(LPARAM)L"simple"));
    capture(status_window,dc,bits,styled_pixels);CHECK(remove_style(status_window)&&!attached(status_window));capture(status_window,dc,bits,native_pixels);
    CHECK(memcmp(styled_pixels,native_pixels,sizeof(styled_pixels)));CHECK(DestroyWindow(status_window));status_window=NULL;
    status_window=create_status(root);CHECK(status_window&&attached(status_window));
    CHECK(SendMessageW(status_window,SB_SETTEXTW,SBT_OWNERDRAW,(LPARAM)0x12345678));remove_in_draw=TRUE;capture(status_window,dc,bits,styled_pixels);
    CHECK(!remove_in_draw && !attached(status_window) && owner_data==0x12345678);CHECK(DestroyWindow(status_window));status_window=NULL;
    status_window=create_status(root);CHECK(status_window&&attached(status_window));
    CHECK(SetWindowSubclass(status_window,app_subclass,1,0));CHECK(SendMessageW(status_window,SB_SETTEXTW,0,(LPARAM)L"before"));
    mutate_on_query=TRUE;capture(status_window,dc,bits,styled_pixels);CHECK(!mutate_on_query && attached(status_window));
    CHECK(remove_style(status_window)&&!attached(status_window));capture(status_window,dc,bits,native_pixels);CHECK(!memcmp(styled_pixels,native_pixels,sizeof(styled_pixels)));
    CHECK(RemoveWindowSubclass(status_window,app_subclass,1));CHECK(DestroyWindow(status_window));status_window=NULL;
    status_window=create_status(root);CHECK(status_window&&attached(status_window));
    CHECK(SetWindowSubclass(status_window,app_subclass,1,0));HWND doomed=status_window;destroy_on_query=TRUE;capture(status_window,dc,bits,styled_pixels);CHECK(!destroy_on_query && !IsWindow(doomed));status_window=NULL;
    CHECK(unreg());CHECK(FreeLibrary(fixture));
    WNDCLASSW fake={.hInstance=parent.hInstance,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"TStatusBar"};CHECK(RegisterClassW(&fake));
    HWND impostor=CreateWindowW(fake.lpszClassName,L"",WS_CHILD,0,0,100,25,root,NULL,fake.hInstance,NULL);CHECK(impostor&&!attached(impostor));CHECK(DestroyWindow(impostor));CHECK(UnregisterClassW(fake.lpszClassName,fake.hInstance));
    fake.lpszClassName=STATUSCLASSNAMEW;SetLastError(ERROR_SUCCESS);ATOM shadow=RegisterClassW(&fake);DWORD registration_error=GetLastError();
    if(shadow){
        impostor=CreateWindowW(fake.lpszClassName,L"",WS_CHILD,0,0,100,25,root,NULL,fake.hInstance,NULL);CHECK(impostor);
        CHECK((HMODULE)GetClassLongPtrW(impostor,GCLP_HMODULE)==parent.hInstance && !attached(impostor));
        CHECK(DestroyWindow(impostor));CHECK(UnregisterClassW(fake.lpszClassName,fake.hInstance));
    }else CHECK(registration_error==ERROR_CLASS_ALREADY_EXISTS);
    printf("STATUS_BOUNDARY_OK noborders=1 popout=1 simple=1 reentrant_remove=1 mutate_query=1 destroy_query=1 fake_class=1 shadow_test=%u dpi=%u aware=%u\n",shadow!=0,GetDpiForWindow(root),argc==3);
    CHECK(SelectObject(dc,old));CHECK(DeleteObject(bitmap));CHECK(DeleteDC(dc));CHECK(DestroyWindow(root));CHECK(UnregisterClassW(parent.lpszClassName,parent.hInstance));puts("VCL_STATUS_MODULE_OK");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
