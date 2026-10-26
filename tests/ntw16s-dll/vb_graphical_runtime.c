/* Project-authored native owner-draw protocol model, not an actual VB runtime. */
#include <windows.h>
static HINSTANCE instance;
static HFONT source;
static WNDPROC native_button;
static unsigned mode;
static SIZE measured;
static RECT api_rect;static DWORD api_error,api_flags;
#ifdef VB_GRAPHICAL_RUNTIME60
static const WCHAR name[]=L"ThunderRT6CommandButton";
#else
static const WCHAR name[]=L"ThunderRT5CommandButton";
#endif
static LRESULT CALLBACK button(HWND w,UINT m,WPARAM a,LPARAM b)
{return CallWindowProcW(native_button,w,m,a,b);}
__declspec(dllexport) BOOL WINAPI VbGraphicalRegister(void)
{
    WNDCLASSW cls;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)(UINT_PTR)VbGraphicalRegister,(HMODULE *)&instance) ||
            !GetClassInfoW(NULL,L"BUTTON",&cls))return FALSE;
    native_button=cls.lpfnWndProc;cls.lpfnWndProc=button;cls.hInstance=instance;cls.lpszClassName=name;
    source=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    return source && RegisterClassW(&cls);
}
__declspec(dllexport) HFONT WINAPI VbGraphicalFont(void){return source;}
__declspec(dllexport) void WINAPI VbGraphicalMode(unsigned value){mode=value;}
__declspec(dllexport) SIZE WINAPI VbGraphicalMeasured(void){return measured;}
__declspec(dllexport) void WINAPI VbGraphicalApiState(RECT *rect,DWORD *error,DWORD *flags)
{*rect=api_rect;*error=api_error;*flags=api_flags;}
__declspec(dllexport) LRESULT WINAPI VbGraphicalDraw(const DRAWITEMSTRUCT *item)
{
    if(mode==1){mode=0;
        HMODULE style=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *remove)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowRemove"):NULL;
        if(!remove || !remove(item->hwndItem))ExitProcess(91);
    }
    int saved=SaveDC(item->hDC);if(!saved)ExitProcess(92);
    RECT rect=item->rcItem;UINT flags=DFCS_BUTTONPUSH;
    if(item->itemState&ODS_SELECTED)flags|=DFCS_PUSHED;
    if(item->itemState&ODS_DISABLED)flags|=DFCS_INACTIVE;
    if(mode==2)flags|=DFCS_FLAT;
    if(mode==3)InflateRect(&rect,-1,-1);
    if(mode==4)flags|=DFCS_ADJUSTRECT;
    SetLastError(0x5a17);
    if(!DrawFrameControl(item->hDC,&rect,DFC_BUTTON,flags))ExitProcess(93);
    api_error=GetLastError();api_rect=rect;api_flags=flags;
    SelectObject(item->hDC,(HFONT)SendMessageW(item->hwndItem,WM_GETFONT,0,0));
    RECT metric={0};DrawTextA(item->hDC,"Graphical",-1,&metric,DT_CALCRECT|DT_SINGLELINE);
    measured=(SIZE){metric.right,metric.bottom};
    RECT text=item->rcItem;text.left+=20;text.top+=5;SetBkMode(item->hDC,TRANSPARENT);
    DrawTextA(item->hDC,"Graphical",-1,&text,DT_SINGLELINE);
    RECT image={7,7,16,16};SetDCBrushColor(item->hDC,RGB(11,77,193));FillRect(item->hDC,&image,GetStockObject(DC_BRUSH));
    RestoreDC(item->hDC,saved);return TRUE;
}
__declspec(dllexport) BOOL WINAPI VbGraphicalRelease(void)
{return UnregisterClassW(name,instance) && DeleteObject(source);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved)
{(void)h;(void)reason;(void)reserved;return TRUE;}
