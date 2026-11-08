#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
typedef HWND (WINAPI *band_fn)(DWORD,LPCWSTR,LPCWSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,void *,DWORD);
typedef BOOL (__cdecl *attached_fn)(HWND);
static LRESULT CALLBACK custom_frame(HWND w,UINT message,WPARAM wp,LPARAM lp)
{if(message==WM_NCCALCSIZE)return 0;return DefWindowProcW(w,message,wp,lp);}
static LONG client_top(HWND w)
{RECT outer={0};POINT client={0};if(!GetWindowRect(w,&outer)||!ClientToScreen(w,&client))return -1;return client.y-outer.top;}
static BOOL frame_print(HWND w,LONG top)
{
    RECT r={0};if(!GetWindowRect(w,&r)||top<=0)return FALSE;
    int width=r.right-r.left,height=r.bottom-r.top;HDC dc=GetDC(NULL),memory=CreateCompatibleDC(dc);
    HBITMAP bitmap=CreateCompatibleBitmap(dc,width,height);HGDIOBJ old=SelectObject(memory,bitmap);
    RECT all={0,0,width,height};HBRUSH bg=CreateSolidBrush(RGB(211,1,177));FillRect(memory,&all,bg);DeleteObject(bg);
    SendMessageW(w,WM_PRINT,(WPARAM)memory,PRF_NONCLIENT);
    unsigned edge=0,light=0,face=0;
    for(int y=0;y<top && y<height;++y)for(int x=0;x<width;++x){COLORREF c=GetPixel(memory,x,y);
        if(c==RGB(0,0,0))++edge;else if(c==RGB(255,255,255))++light;else if(c==RGB(192,192,192))++face;}
    SelectObject(memory,old);DeleteObject(bitmap);DeleteDC(memory);ReleaseDC(NULL,dc);
    printf("BAND PRINT edge=%u light=%u face=%u\n",edge,light,face);
    return edge>10 && light>100 && face>10;
}
int wmain(int argc,WCHAR **argv)
{
    BOOL expected=argc==2 && !wcscmp(argv[1],L"styled");
    WCHAR desktop_name[80];swprintf(desktop_name,80,L"Win16BandFixture-%lu",GetCurrentProcessId());
    HDESK desktop=CreateDesktopW(desktop_name,NULL,NULL,0,GENERIC_ALL,NULL);
    if(!desktop || !SetThreadDesktop(desktop)){printf("BAND SKIP desktop error=%lu\n",GetLastError());if(desktop)CloseDesktop(desktop);return 77;}
    band_fn create=(band_fn)(uintptr_t)GetProcAddress(GetModuleHandleW(L"user32.dll"),"CreateWindowInBand");
    if(!create){puts("BAND SKIP unavailable");return 77;}
#ifdef _WIN64
    HMODULE style=GetModuleHandleW(L"ntw16s64.dll");
#else
    HMODULE style=GetModuleHandleW(L"ntw16s32.dll");
#endif
    attached_fn attached=style?(attached_fn)(uintptr_t)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;
    WNDCLASSW cls={0};cls.lpfnWndProc=custom_frame;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"Win16BandFixture";
    if(!RegisterClassW(&cls))return 2;
    SetLastError(0);
    HWND w=create(0,cls.lpszClassName,L"band fixture",WS_OVERLAPPEDWINDOW,0,0,320,200,NULL,NULL,cls.hInstance,NULL,0);
    DWORD error=GetLastError();BOOL actual=w && attached && attached(w);
    printf("BAND module=%d window=%d attached=%d expected=%d error=%lu\n",style!=NULL,w!=NULL,actual,expected,error);
    if(w){ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);}
    LONG styled_top=w?client_top(w):-1;
    if(expected && actual){
        if(!frame_print(w,styled_top)){DestroyWindow(w);return 7;}
        attached_fn remove=(attached_fn)(uintptr_t)GetProcAddress(style,"Ntw16sWindowRemove");
        BOOL removed=remove && remove(w),remaining=attached(w);
        LONG restored_top=client_top(w);
        printf("BAND REMOVE removed=%d remaining=%d top=%ld restored=%ld\n",removed,remaining,styled_top,restored_top);
        if(!removed || remaining || styled_top<=0 || restored_top!=0){DestroyWindow(w);return 3;}
    }
    if(!expected && styled_top!=0){if(w)DestroyWindow(w);return 4;}
    if(w)DestroyWindow(w);
    SetLastError(0);
    HWND missing=create(0,L"Win16BandFixtureMissingClass",L"",WS_OVERLAPPEDWINDOW,0,0,20,20,NULL,NULL,cls.hInstance,NULL,0);
    DWORD missing_error=GetLastError();
    printf("BAND INVALID window=%d error=%lu\n",missing!=NULL,missing_error);
    if(missing){DestroyWindow(missing);return 5;}
    if(missing_error!=ERROR_CANNOT_FIND_WND_CLASS)return 6;
    return w && actual==expected && (!expected || attached)?0:1;
}
