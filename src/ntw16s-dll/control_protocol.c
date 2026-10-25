#include "control_protocol.h"
#include <commctrl.h>
#include <stdlib.h>
#include <limits.h>

struct ntw16s_control_protocol {
    struct ntw16s_control_protocol *next;
    HWND window;
    unsigned role,refs;
};
static _Thread_local ntw16s_control_protocol *bindings;
static const WCHAR key[]=L"ntw16s.private.control-protocol.v1";
static BOOL current(const ntw16s_control_protocol *b)
{return IsWindow(b->window) && GetPropW(b->window,key)==(HANDLE)b;}
BOOL ntw16s_control_protocol_is(HWND w,const WCHAR *name)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && current(b))return (b->role==1 && !lstrcmpiW(name,L"BUTTON")) ||
            (b->role==2 && !lstrcmpiW(name,L"COMBOBOX")) || (b->role==3 && !lstrcmpiW(name,WC_TABCONTROLW));
    return FALSE;
}
ntw16s_control_protocol *ntw16s_control_protocol_acquire(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)if(b->window==w && current(b)){
        if(b->refs==UINT_MAX){SetLastError(ERROR_ARITHMETIC_OVERFLOW);return NULL;}
        ++b->refs;return b;
    }
    return NULL;
}
void ntw16s_control_protocol_release(ntw16s_control_protocol *b)
{
    if(!b)return;
    ntw16s_control_protocol **link=&bindings;
    while(*link && *link!=b)link=&(*link)->next;
    if(!*link)return;
    if(--b->refs)return;
    *link=b->next;
    if(current(b))RemovePropW(b->window,key);
    free(b);
}
ntw16s_control_protocol *ntw16s_control_protocol_bind(HWND w)
{
    WCHAR name[64];unsigned role;DWORD process;
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(GetWindowThreadProcessId(w,&process)!=GetCurrentThreadId() || process!=GetCurrentProcessId() ||
        !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || GetPropW(w,key))return NULL;
    /* No prefix aliases or private VCL object/procedure decoding. All roles
     * require initialized public protocol evidence, even a tab's real name. */
    role=!lstrcmpW(name,L"TButton")?1:!lstrcmpW(name,L"TComboBox")?2:
        (!lstrcmpW(name,WC_TABCONTROLW) || !lstrcmpW(name,L"TPageControl"))?3:0;
    if(!role)return NULL;
    WNDCLASSEXW tab_class={.cbSize=sizeof(tab_class)};
    if(role==3 && !GetClassInfoExW(NULL,WC_TABCONTROLW,&tab_class))return NULL;
    HMODULE native_owner=NULL;
    /* Global common-control classes can report hInstance == NULL. Resolve
     * the registered native procedure's image, not a guessed DLL filename. */
    if(role==3 && !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)(UINT_PTR)tab_class.lpfnWndProc,&native_owner))return NULL;
    if(owner!=((role==3 && !lstrcmpW(name,WC_TABCONTROLW))?native_owner:GetModuleHandleW(NULL)))return NULL;
    if(role==3 && ((GetWindowLongPtrW(w,GWL_STYLE)&(TCS_BOTTOM|TCS_VERTICAL)) ||
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)tab_class.cbWndExtra))return NULL;
    ntw16s_control_protocol *b=calloc(1,sizeof(*b));
    if(!b){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    b->window=w;b->refs=1;b->next=bindings;bindings=b;
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
    }else if(role==2){
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
    }else {
        RECT input={0,0,300,200},display=input,inverse;TC_HITTESTINFO hit={.pt={-30000,-30000}};
        LRESULT declared=SendMessageW(w,WM_GETDLGCODE,0,0);
        LRESULT count=current(b)?SendMessageW(w,TCM_GETITEMCOUNT,0,0):-1;
        LRESULT selected=current(b)?SendMessageW(w,TCM_GETCURSEL,0,0):-2;
        if(current(b) && (declared&DLGC_WANTARROWS) && count>=0 && count<=INT_MAX &&
            selected>=-1 && (selected<count) && (count || selected==-1)){
            LRESULT outside=SendMessageW(w,TCM_HITTEST,0,(LPARAM)&hit);
            if(current(b) && outside==-1 && (hit.flags&TCHT_NOWHERE)){
                SendMessageW(w,TCM_ADJUSTRECT,FALSE,(LPARAM)&display);inverse=display;
                if(current(b))SendMessageW(w,TCM_ADJUSTRECT,TRUE,(LPARAM)&inverse);
                valid=current(b) && display.left>input.left && display.right<input.right &&
                    display.bottom<input.bottom && EqualRect(&input,&inverse);
            }
        }
    }
    if(!valid){ntw16s_control_protocol_release(b);return NULL;}
    b->role=role;return b;
}
