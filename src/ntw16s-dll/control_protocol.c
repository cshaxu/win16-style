#include "control_protocol.h"
#include <commctrl.h>
#include <stdlib.h>

struct ntw16s_control_protocol {
    struct ntw16s_control_protocol *next;
    HWND window;
    unsigned role;
};
static _Thread_local ntw16s_control_protocol *bindings;
static const WCHAR key[]=L"ntw16s.private.control-protocol.v1";
static BOOL current(const ntw16s_control_protocol *b)
{return IsWindow(b->window) && GetPropW(b->window,key)==(HANDLE)b;}
BOOL ntw16s_control_protocol_is(HWND w,const WCHAR *name)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && current(b))return (b->role==1 && !lstrcmpiW(name,L"BUTTON")) ||
            (b->role==2 && !lstrcmpiW(name,L"COMBOBOX"));
    return FALSE;
}
void ntw16s_control_protocol_release(ntw16s_control_protocol *b)
{
    if(!b)return;
    ntw16s_control_protocol **link=&bindings;
    while(*link && *link!=b)link=&(*link)->next;
    if(!*link)return;
    *link=b->next;
    if(current(b))RemovePropW(b->window,key);
    free(b);
}
ntw16s_control_protocol *ntw16s_control_protocol_bind(HWND w)
{
    WCHAR name[64];unsigned role;DWORD process;
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(GetWindowThreadProcessId(w,&process)!=GetCurrentThreadId() || process!=GetCurrentProcessId() ||
        owner!=GetModuleHandleW(NULL) || !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || GetPropW(w,key))return NULL;
    /* No prefix aliases, no private VCL object/procedure decoding. These two
     * registered names still require initialized public protocol evidence. */
    role=!lstrcmpW(name,L"TButton")?1:!lstrcmpW(name,L"TComboBox")?2:0;
    if(!role)return NULL;
    ntw16s_control_protocol *b=calloc(1,sizeof(*b));
    if(!b){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    b->window=w;b->next=bindings;bindings=b;
    if(!SetPropW(w,key,(HANDLE)b)){ntw16s_control_protocol_release(b);return NULL;}
    BOOL valid=FALSE;
    if(role==1){
        WNDCLASSEXW native={.cbSize=sizeof(native)};SIZE ideal={0};
        LRESULT declared=SendMessageW(w,WM_GETDLGCODE,0,0);
        if(current(b) && (declared&DLGC_BUTTON) && GetClassInfoExW(NULL,L"BUTTON",&native) &&
            GetClassLongPtrW(w,GCL_CBWNDEXTRA)>=(ULONG_PTR)native.cbWndExtra){
            LRESULT sized=SendMessageW(w,BCM_GETIDEALSIZE,0,(LPARAM)&ideal);
            if(current(b) && sized && ideal.cx>0 && ideal.cy>0 && ideal.cx<=32767 && ideal.cy<=32767){
                LRESULT check=SendMessageW(w,BM_GETCHECK,0,0);
                if(current(b) && check>=BST_UNCHECKED && check<=BST_INDETERMINATE){
                    LRESULT state=SendMessageW(w,BM_GETSTATE,0,0);
                    valid=current(b) && !(state&~(BST_CHECKED|BST_INDETERMINATE|BST_PUSHED|BST_FOCUS|BST_HOT|BST_DROPDOWNPUSHED));
                }
            }
        }
    }else {
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(GetComboBoxInfo(w,&info) && current(b) && info.hwndCombo==w && info.hwndItem && info.hwndList &&
            (info.hwndItem==w || GetParent(info.hwndItem)==w) &&
            GetWindowThreadProcessId(info.hwndItem,NULL)==GetCurrentThreadId() &&
            GetWindowThreadProcessId(info.hwndList,NULL)==GetCurrentThreadId()){
            LRESULT count=SendMessageW(w,CB_GETCOUNT,0,0);
            if(current(b) && count>=0){
                LRESULT selected=SendMessageW(w,CB_GETCURSEL,0,0);
                valid=current(b) && (selected==CB_ERR || (selected>=0 && selected<count));
            }
        }
    }
    if(!valid){ntw16s_control_protocol_release(b);return NULL;}
    b->role=role;return b;
}
