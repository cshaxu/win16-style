#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "isolated_process.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"STATUS_CONTROL_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x); return 1; } } while (0)
#define WIDTH 400
#define HEIGHT 64

static unsigned owner_draws;
static ULONG_PTR owner_data;
static HWND status_window;
static LRESULT CALLBACK parent_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_DRAWITEM){
        DRAWITEMSTRUCT *item=(void *)lp;
        if(item && item->hwndItem==status_window){
            ++owner_draws;owner_data=item->itemData;
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
static void capture(HWND w,HDC dc,DWORD *bits,DWORD *out)
{
    RECT clear={0,0,WIDTH,HEIGHT};FillRect(dc,&clear,(HBRUSH)GetStockObject(WHITE_BRUSH));
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();
    memcpy(out,bits,sizeof(styled_pixels));
}
static int worker(int argc,WCHAR **argv)
{
    (void)argv;CHECK(argc==1);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    BOOL (__cdecl *attached)(HWND)=(void *)GetProcAddress(dll,"Ntw16sWindowAttached");
    BOOL (__cdecl *remove_style)(HWND)=(void *)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove_style);
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
    HWND root=CreateWindowW(parent.lpszClassName,L"status protocol",WS_OVERLAPPEDWINDOW,0,0,430,180,NULL,NULL,parent.hInstance,NULL);CHECK(root && attached(root));
    status_window=CreateWindowW(STATUSCLASSNAMEW,L"",WS_CHILD|WS_VISIBLE|CCS_BOTTOM,0,0,400,30,root,(HMENU)(INT_PTR)9,parent.hInstance,NULL);CHECK(status_window);
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
    CHECK(remove_style(status_window) && !attached(status_window));before=owner_draws;capture(status_window,dc,bits,native_pixels);CHECK(owner_draws==before+1 && owner_data==0x12345678);
    unsigned different=0,outside=0;for(int y=0;y<client.bottom;++y)for(int x=0;x<client.right;++x)if(styled_pixels[y*WIDTH+x]!=native_pixels[y*WIDTH+x]){++different;if(!on_frame(x,y,&a)&&!on_frame(x,y,&b))++outside;}
    CHECK(different>0 && outside==0);WCHAR text[32]={0};CHECK(LOWORD(SendMessageW(status_window,SB_GETTEXTW,0,(LPARAM)text))==8 && !wcscmp(text,L"ordinary"));CHECK((HICON)SendMessageW(status_window,SB_GETICON,0,0)==icon);
    printf("STATUS_CONTROL_OK bits=%u changed=%u outside=%u owner_data=%p icon_retained=1\n",(unsigned)sizeof(void*)*8,different,outside,(void *)owner_data);
    CHECK(SelectObject(dc,old));CHECK(DeleteObject(bitmap));CHECK(DeleteDC(dc));CHECK(DestroyWindow(root));CHECK(UnregisterClassW(parent.lpszClassName,parent.hInstance));puts("STATUS_CONTROL_CLEANUP_OK");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
