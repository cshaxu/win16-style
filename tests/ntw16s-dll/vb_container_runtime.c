/* Project-authored public-window protocol model; not an actual VB runtime. */
#include <windows.h>
#include <stdio.h>
static HINSTANCE instance;
static HFONT source_font;
#ifdef VB_CONTAINER_RUNTIME60
static const WCHAR class_name[]=L"ThunderRT6PictureBoxDC";
#else
static const WCHAR class_name[]=L"ThunderRT5PictureBox";
#endif
typedef struct model {HFONT font;SIZE measured;unsigned clicks,keys;BOOL retire;} model;
static void render(HWND w,HDC dc,model *p)
{
    if(p->retire){p->retire=FALSE;
        HMODULE dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *remove)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
        if(!remove || !remove(w))ExitProcess(81);
    }
    int saved=SaveDC(dc);if(!saved)ExitProcess(82);
    RECT client;GetClientRect(w,&client);FillRect(dc,&client,GetStockObject(WHITE_BRUSH));
    SelectObject(dc,p->font);SetTextColor(dc,RGB(0,0,0));SetBkMode(dc,TRANSPARENT);
    RECT measure={0};DrawTextA(dc,"Label sample",-1,&measure,DT_SINGLELINE|DT_CALCRECT);
    p->measured=(SIZE){measure.right,measure.bottom};
    TextOutA(dc,8,5,"Label sample",12);
    TabbedTextOutA(dc,8,28,"Application Print",17,0,NULL,0);
    SendMessageW(GetParent(w),WM_APP+42,(WPARAM)dc,(LPARAM)w);
    RECT image={154,6,166,18};SetDCBrushColor(dc,RGB(11,77,193));FillRect(dc,&image,GetStockObject(DC_BRUSH));
    SelectObject(dc,GetStockObject(DC_PEN));SetDCPenColor(dc,RGB(200,9,21));MoveToEx(dc,150,35,NULL);LineTo(dc,166,35);
    RestoreDC(dc,saved);
}
static LRESULT CALLBACK picture(HWND w,UINT m,WPARAM a,LPARAM b)
{
    model *p=(void *)GetWindowLongPtrW(w,GWLP_USERDATA);
    if(m==WM_NCCREATE){p=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*p));if(!p)return FALSE;p->font=source_font;SetWindowLongPtrW(w,GWLP_USERDATA,(LONG_PTR)p);}
    if(!p)return DefWindowProcW(w,m,a,b);
    if(m==WM_GETFONT)return (LRESULT)p->font;
    if(m==WM_SETFONT){p->font=(HFONT)a;if(b)InvalidateRect(w,NULL,TRUE);return 0;}
    if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);render(w,dc,p);EndPaint(w,&ps);return 0;}
    if(m==WM_PRINTCLIENT){render(w,(HDC)a,p);return 0;}
    if(m==WM_LBUTTONDOWN){++p->clicks;SetFocus(w);SendMessageW(GetParent(w),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(w),0),(LPARAM)w);return 0;}
    if(m==WM_KEYDOWN){++p->keys;SendMessageW(GetParent(w),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(w),1),(LPARAM)w);return 0;}
    if(m==WM_NCDESTROY){LRESULT result=DefWindowProcW(w,m,a,b);SetWindowLongPtrW(w,GWLP_USERDATA,0);HeapFree(GetProcessHeap(),0,p);return result;}
    return DefWindowProcW(w,m,a,b);
}
__declspec(dllexport) BOOL WINAPI VbContainerRegister(void)
{
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)(UINT_PTR)VbContainerRegister,(HMODULE *)&instance);
    source_font=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=picture,.lpszClassName=class_name,.hbrBackground=GetStockObject(WHITE_BRUSH)};
#ifdef VB_CONTAINER_RUNTIME60
    cls.style=CS_OWNDC;
#endif
    return source_font && RegisterClassW(&cls);
}
__declspec(dllexport) unsigned WINAPI VbContainerVersion(void)
{
#ifdef VB_CONTAINER_RUNTIME60
    return 6;
#else
    return 5;
#endif
}
__declspec(dllexport) BOOL WINAPI VbContainerInspect(HWND w,SIZE *measured,unsigned *clicks,unsigned *keys)
{model *p=(void *)GetWindowLongPtrW(w,GWLP_USERDATA);if(!p)return FALSE;*measured=p->measured;*clicks=p->clicks;*keys=p->keys;return TRUE;}
__declspec(dllexport) void WINAPI VbContainerRetire(HWND w)
{model *p=(void *)GetWindowLongPtrW(w,GWLP_USERDATA);if(p)p->retire=TRUE;}
__declspec(dllexport) BOOL WINAPI VbContainerRelease(void)
{return UnregisterClassW(class_name,instance) && DeleteObject(source_font);}
#ifdef VB_CONTAINER_RUNTIME60
static const WCHAR mdi_class[]=L"ThunderRT6FormDC";
#else
static const WCHAR mdi_class[]=L"ThunderRT5Form";
#endif
static LRESULT CALLBACK mdi_child(HWND w,UINT msg,WPARAM a,LPARAM b)
{return DefMDIChildProcW(w,msg,a,b);}
__declspec(dllexport) BOOL WINAPI VbMdiRegister(void)
{
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)(UINT_PTR)VbMdiRegister,(HMODULE *)&instance);
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=mdi_child,.lpszClassName=mdi_class,.hbrBackground=GetStockObject(WHITE_BRUSH)};
    return RegisterClassW(&cls)!=0;
}
__declspec(dllexport) BOOL WINAPI VbMdiRelease(void)
{return UnregisterClassW(mdi_class,instance);}
#ifdef VB_CONTAINER_RUNTIME60
__declspec(dllexport) BOOL WINAPI VbUserEdgeRegister(void)
{
    WNDCLASSW cls={.style=CS_OWNDC,.hInstance=instance,.lpfnWndProc=picture,.lpszClassName=L"ThunderRT6UserControlDC",.hbrBackground=GetStockObject(WHITE_BRUSH)};
    return RegisterClassW(&cls)!=0;
}
__declspec(dllexport) BOOL WINAPI VbUserEdgeRelease(void)
{return UnregisterClassW(L"ThunderRT6UserControlDC",instance);}
#endif
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved)
{(void)h;(void)reason;(void)reserved;return TRUE;}
