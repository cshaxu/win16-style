#include <windows.h>
/* Self-contained protocol fixture, not a VB runtime replacement. The test
 * module deliberately registers the known class names under its own HMODULE.
 * Native BUTTON/EDIT retain their real USER message/notification contracts. */
static HINSTANCE instance;
static WNDPROC button_proc,edit_proc;
static HFONT native_font;
static WORD digit_glyphs[5],unmapped_glyph;
static LRESULT CALLBACK button(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_GETFONT)return (LRESULT)native_font;
    if(m==WM_SETFONT)return 0;
    LRESULT result=CallWindowProcW(button_proc,w,m,a,b);
    if(m==WM_CREATE)CallWindowProcW(button_proc,w,WM_SETFONT,(WPARAM)native_font,FALSE);
    return result;
}
/* Independent C protocol fixture: deliberately has no USER BUTTON superclass.
 * It is not evidence about an actual VB Value implementation. Real Actor is
 * validated separately; this class makes a missing exact-alias regression fail. */
typedef struct checkbox_state {UINT check,state;} checkbox_state;
static HWND destroy_on_font,destroyed_checkbox;
static unsigned late_checkbox_calls;
__declspec(dllexport) void WINAPI VbCheckboxRetirementProbe(HWND w)
{destroy_on_font=w;destroyed_checkbox=NULL;late_checkbox_calls=0;}
__declspec(dllexport) unsigned WINAPI VbCheckboxLateCalls(void)
{return late_checkbox_calls;}
static void checkbox_paint(HWND w,HDC dc,checkbox_state *s)
{
    RECT r;GetClientRect(w,&r);FillRect(dc,&r,GetSysColorBrush(COLOR_BTNFACE));
    RECT glyph={0,(r.bottom-13)/2,13,(r.bottom+13)/2};
    UINT flags=DFCS_BUTTONCHECK|(s->check?DFCS_CHECKED:0)|(s->state&BST_PUSHED?DFCS_PUSHED:0)|(!IsWindowEnabled(w)?DFCS_INACTIVE:0);
    if(GetWindowLongPtrW(w,GWL_STYLE)&BS_PUSHLIKE){glyph=r;flags=DFCS_BUTTONPUSH;}
    DrawFrameControl(dc,&glyph,DFC_BUTTON,flags);WCHAR text[100];GetWindowTextW(w,text,100);
    HGDIOBJ old=SelectObject(dc,native_font);SetBkMode(dc,TRANSPARENT);r.left=17;DrawTextW(dc,text,-1,&r,DT_SINGLELINE|DT_VCENTER);SelectObject(dc,old);
}
static LRESULT CALLBACK checkbox(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(w==destroyed_checkbox)++late_checkbox_calls;
    checkbox_state *s=(checkbox_state *)GetWindowLongPtrW(w,GWLP_USERDATA);
    if(m==WM_NCCREATE){s=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s));if(!s)return FALSE;SetWindowLongPtrW(w,GWLP_USERDATA,(LONG_PTR)s);return TRUE;}
    if(!s)return DefWindowProcW(w,m,a,b);
    if(m==WM_GETFONT)return (LRESULT)native_font;
    if(m==WM_SETFONT){if(w==destroy_on_font){destroy_on_font=NULL;DestroyWindow(w);destroyed_checkbox=w;}return 0;}
    if(m==WM_GETDLGCODE)return DLGC_BUTTON|DLGC_WANTCHARS|(((GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_RADIOBUTTON)?DLGC_RADIOBUTTON:0);
    if(m==BM_GETCHECK)return s->check;
    if(m==BM_GETSTATE)return s->check|s->state|(GetFocus()==w?BST_FOCUS:0);
    if(m==BM_SETCHECK){s->check=(UINT)a%3;InvalidateRect(w,NULL,TRUE);return 0;}
    if(m==BM_SETSTATE){s->state=a?BST_PUSHED:0;InvalidateRect(w,NULL,TRUE);return 0;}
    if(m==BM_CLICK){if((GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_RADIOBUTTON){WCHAR name[80];for(HWND sibling=GetWindow(GetParent(w),GW_CHILD);sibling;sibling=GetWindow(sibling,GW_HWNDNEXT))if(GetClassNameW(sibling,name,80) && !wcscmp(name,L"ThunderRT6OptionButton"))SendMessageW(sibling,BM_SETCHECK,sibling==w?BST_CHECKED:BST_UNCHECKED,0);}else s->check=(s->check+1)%3;InvalidateRect(w,NULL,TRUE);SendMessageW(GetParent(w),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(w),BN_CLICKED),(LPARAM)w);return 0;}
    if(m==WM_KEYDOWN && a==VK_SPACE){s->state=BST_PUSHED;return 0;}
    if(m==WM_KEYUP && a==VK_SPACE){s->state=0;return SendMessageW(w,BM_CLICK,0,0);}
    if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);checkbox_paint(w,dc,s);EndPaint(w,&ps);return 0;}
    if(m==WM_PRINTCLIENT){checkbox_paint(w,(HDC)a,s);return 0;}
    if(m==WM_NCDESTROY){SetWindowLongPtrW(w,GWLP_USERDATA,0);HeapFree(GetProcessHeap(),0,s);}
    return DefWindowProcW(w,m,a,b);
}
static LRESULT CALLBACK edit(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_GETFONT)return (LRESULT)native_font;
    if(m==WM_SETFONT)return 0;
    LRESULT result=CallWindowProcW(edit_proc,w,m,a,b);
    if(m==WM_CREATE)CallWindowProcW(edit_proc,w,WM_SETFONT,(WPARAM)native_font,FALSE);
    return result;
}
static void paint(HWND w,HDC dc)
{
    RECT r;GetClientRect(w,&r);FillRect(dc,&r,GetSysColorBrush(COLOR_BTNFACE));
    HFONT old=SelectObject(dc,native_font);SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,RGB(0,0,0));RECT label={16,32,180,56};
    DrawTextA(dc,"Windowless label",-1,&label,DT_SINGLELINE);
    INT advances[5]={8,8,8,8,8};
    ExtTextOutW(dc,16,56,ETO_GLYPH_INDEX,NULL,(const WCHAR *)digit_glyphs,5,advances);
    /* A valid Greek glyph outside CP1252, not an invalid 0xffff glyph ID. */
    ExtTextOutW(dc,140,80,ETO_GLYPH_INDEX,NULL,(const WCHAR *)&unmapped_glyph,1,NULL);
    SetPixelV(dc,24,72,GetCurrentObject(dc,OBJ_FONT)==native_font?RGB(219,31,67):RGB(0,0,0));
    SelectObject(dc,old);
}
static LRESULT CALLBACK container(HWND w,UINT m,WPARAM a,LPARAM b)
{
    /* Real VB owns its HFONT and may ignore WM_SETFONT. Reproduce that gate. */
    if(m==WM_GETFONT)return (LRESULT)native_font;
    if(m==WM_SETFONT)return 0;
    if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);paint(w,dc);EndPaint(w,&ps);return 0;}
    if(m==WM_PRINTCLIENT){paint(w,(HDC)a);return 0;}
    return DefWindowProcW(w,m,a,b);
}
__declspec(dllexport) BOOL WINAPI VbRegister(void)
{
    WNDCLASSW cls={0};
    native_font=CreateFontW(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    if(!native_font)return FALSE;
    HDC dc=CreateCompatibleDC(NULL);if(!dc)return FALSE;
    HGDIOBJ old=SelectObject(dc,native_font);
    BOOL mapped=GetGlyphIndicesW(dc,L"01234",5,digit_glyphs,GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR &&
        GetGlyphIndicesW(dc,L"\x03a9",1,&unmapped_glyph,GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR && unmapped_glyph!=0xffff;
    SelectObject(dc,old);DeleteDC(dc);if(!mapped)return FALSE;
    GetClassInfoW(NULL,L"BUTTON",&cls);button_proc=cls.lpfnWndProc;
    cls.hInstance=instance;cls.lpfnWndProc=button;cls.lpszClassName=L"ThunderRT6CommandButton";
    if(!RegisterClassW(&cls))return FALSE;
    ZeroMemory(&cls,sizeof(cls));cls.hInstance=instance;cls.lpfnWndProc=checkbox;
    cls.lpszClassName=L"ThunderRT6CheckBox";if(!RegisterClassW(&cls))return FALSE;
    cls.lpszClassName=L"ThunderRT6OptionButton";if(!RegisterClassW(&cls))return FALSE;
    cls.lpszClassName=L"ThunderRT5CheckBox";if(!RegisterClassW(&cls))return FALSE; /* wrong-runtime negative */
    GetClassInfoW(NULL,L"EDIT",&cls);edit_proc=cls.lpfnWndProc;
    cls.hInstance=instance;cls.lpfnWndProc=edit;cls.lpszClassName=L"ThunderRT6TextBox";
    if(!RegisterClassW(&cls))return FALSE;
    ZeroMemory(&cls,sizeof(cls));cls.hInstance=instance;cls.lpfnWndProc=container;
    cls.hCursor=LoadCursorW(NULL,IDC_ARROW);cls.lpszClassName=L"ThunderRT6Frame";
    return RegisterClassW(&cls)!=0;
}
__declspec(dllexport) void WINAPI VbRelease(void)
{
    UnregisterClassW(L"ThunderRT6Frame",instance);UnregisterClassW(L"ThunderRT6TextBox",instance);
    UnregisterClassW(L"ThunderRT6OptionButton",instance);UnregisterClassW(L"ThunderRT5CheckBox",instance);UnregisterClassW(L"ThunderRT6CheckBox",instance);UnregisterClassW(L"ThunderRT6CommandButton",instance);DeleteObject(native_font);native_font=NULL;
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved)
{(void)reserved;if(reason==DLL_PROCESS_ATTACH)instance=module;return TRUE;}
