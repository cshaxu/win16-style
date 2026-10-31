#include <windows.h>

static HINSTANCE instance;
__declspec(dllexport) BOOL WINAPI VbGlobalDraw(HDC dc,unsigned mode);
static LRESULT CALLBACK form_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_PRINTCLIENT)return VbGlobalDraw((HDC)a,0);
    return DefWindowProcW(w,m,a,b);
}

__declspec(dllexport) BOOL WINAPI VbGlobalRegister(void)
{
    WNDCLASSW cls={0};cls.hInstance=instance;cls.lpfnWndProc=form_proc;cls.lpszClassName=L"ThunderRT6FormDC";
    return RegisterClassW(&cls)!=0;
}
__declspec(dllexport) BOOL WINAPI VbGlobalRelease(void)
{return UnregisterClassW(L"ThunderRT6FormDC",instance);}

__declspec(dllexport) BOOL WINAPI VbGlobalDraw(HDC dc,unsigned mode)
{
    char text[]="Windowless label";RECT rect={4,4,180,36};
    switch(mode){
    case 0:return DrawTextA(dc,text,-1,&rect,DT_SINGLELINE)!=0;
    case 1:return DrawTextExA(dc,text,-1,&rect,DT_SINGLELINE,NULL)!=0;
    case 2:return TextOutA(dc,4,4,text,16);
    case 3:return ExtTextOutA(dc,4,4,0,NULL,text,16,NULL);
    default:return FALSE;
    }
}
__declspec(dllexport) DWORD WINAPI VbGlobalHash(HWND w)
{
    HDC dc=CreateCompatibleDC(NULL);if(!dc)return 0;
    BITMAPINFO info={0};void *pixels=NULL;info.bmiHeader.biSize=sizeof(info.bmiHeader);
    info.bmiHeader.biWidth=200;info.bmiHeader.biHeight=-40;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    HFONT font=CreateFontW(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    if(!bitmap || !pixels || !font){if(font)DeleteObject(font);if(bitmap)DeleteObject(bitmap);DeleteDC(dc);return 0;}
    HGDIOBJ old_bitmap=SelectObject(dc,bitmap),old_font=SelectObject(dc,font);
    RECT rect={0,0,200,40};FillRect(dc,&rect,(HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(0,0,0));
    LRESULT drawn=SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,0);GdiFlush();DWORD hash=2166136261u;
    if(drawn)for(size_t i=0;i<200u*40u;++i)hash=(hash^((DWORD *)pixels)[i])*16777619u;
    SelectObject(dc,old_font);SelectObject(dc,old_bitmap);DeleteObject(font);DeleteObject(bitmap);DeleteDC(dc);
    hash&=0x7fffffffu;return drawn?(hash?hash:1):0;
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved)
{(void)reserved;if(reason==DLL_PROCESS_ATTACH)instance=module;return TRUE;}
