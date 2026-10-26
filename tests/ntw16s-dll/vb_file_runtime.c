/* Pure C protocol model, not an actual VB runtime. Never shipped in product. */
#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
static HINSTANCE instance;
static HFONT font;
static WNDPROC list_proc,combo_proc;
static HWND remove_on_draw;
static unsigned notices;
#ifdef VB_FILE_RUNTIME60
static const WCHAR *names[]={L"ThunderRT6FileListBox",L"ThunderRT6DirListBox"};
#else
static const WCHAR *names[]={L"ThunderRT5FileListBox",L"ThunderRT5DirListBox",L"ThunderRT5DriveListBox"};
#endif
static LRESULT CALLBACK files(HWND w,UINT m,WPARAM a,LPARAM b)
{
    WCHAR cls[80];GetClassNameW(w,cls,80);
    WNDPROC native=wcsstr(cls,L"DriveListBox")?combo_proc:list_proc;
    if(m==WM_GETFONT)return (LRESULT)font;
    if(m==WM_SETFONT)return 0;
    LRESULT result=CallWindowProcW(native,w,m,a,b);
    /* USER's raw combo print path omits this model's callback image. Model
     * the runtime's owner-draw print contract explicitly, using public data. */
    if(m==WM_PRINTCLIENT && native==combo_proc){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(GetComboBoxInfo(w,&info)){
            DRAWITEMSTRUCT item={.CtlType=ODT_COMBOBOX,.CtlID=(UINT)GetDlgCtrlID(w),
                .itemID=(UINT)SendMessageW(w,CB_GETCURSEL,0,0),.itemAction=ODA_DRAWENTIRE,
                .hwndItem=w,.hDC=(HDC)a,.rcItem=info.rcItem};
            SendMessageW(GetParent(w),WM_DRAWITEM,item.CtlID,(LPARAM)&item);
        }
    }
    if(m==WM_CREATE)CallWindowProcW(native,w,WM_SETFONT,(WPARAM)font,FALSE);
    return result;
}
static LRESULT CALLBACK parent(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_MEASUREITEM && b){((MEASUREITEMSTRUCT *)b)->itemHeight=20;return TRUE;}
    if(m==WM_COMMAND)++notices;
    if(m==WM_DRAWITEM && b){
        DRAWITEMSTRUCT *item=(void *)b;
        if(item->hwndItem==remove_on_draw){
            remove_on_draw=NULL;
            HMODULE dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
            BOOL (__cdecl *remove)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
            if(!remove || !remove(item->hwndItem))ExitProcess(81);
        }
        int saved=SaveDC(item->hDC);if(!saved)return FALSE;
        FillRect(item->hDC,&item->rcItem,GetStockObject(WHITE_BRUSH));
        RECT picture=item->rcItem;picture.left+=3;picture.top+=3;picture.right=picture.left+12;picture.bottom=picture.top+12;
        SetDCBrushColor(item->hDC,RGB(11,77,193));FillRect(item->hDC,&picture,GetStockObject(DC_BRUSH));
        RECT text=item->rcItem;text.left+=20;WCHAR label[40];swprintf_s(label,40,L"Native item %u",item->itemID);
        SelectObject(item->hDC,font);SetTextColor(item->hDC,RGB(0,0,0));SetBkMode(item->hDC,TRANSPARENT);
        DrawTextW(item->hDC,label,-1,&text,DT_SINGLELINE|DT_VCENTER);
        RestoreDC(item->hDC,saved);return TRUE;
    }
    return DefWindowProcW(w,m,a,b);
}
__declspec(dllexport) BOOL WINAPI VbFileRegister(void)
{
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)(UINT_PTR)VbFileRegister,(HMODULE *)&instance);
    font=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    if(!font)return FALSE;
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=parent,.lpszClassName=L"VBFileFixtureParent"};
    if(!RegisterClassW(&cls))return FALSE;
    for(unsigned i=0;i<ARRAYSIZE(names);++i){
        if(!GetClassInfoW(NULL,i==2?L"COMBOBOX":L"LISTBOX",&cls))return FALSE;
        if(i==2)combo_proc=cls.lpfnWndProc;else list_proc=cls.lpfnWndProc;
        cls.hInstance=instance;cls.lpfnWndProc=files;cls.lpszClassName=names[i];
        if(!RegisterClassW(&cls))return FALSE;
    }
    return TRUE;
}
__declspec(dllexport) void WINAPI VbFileRemoveOnDraw(HWND w){remove_on_draw=w;}
__declspec(dllexport) unsigned WINAPI VbFileNotices(void){return notices;}
__declspec(dllexport) unsigned WINAPI VbFileVersion(void){return ARRAYSIZE(names)==2?6:5;}
__declspec(dllexport) BOOL WINAPI VbFileRelease(void)
{
    for(unsigned i=0;i<ARRAYSIZE(names);++i)if(!UnregisterClassW(names[i],instance))return FALSE;
    if(!UnregisterClassW(L"VBFileFixtureParent",instance))return FALSE;
    return DeleteObject(font);
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved)
{(void)module;(void)reason;(void)reserved;return TRUE;}
