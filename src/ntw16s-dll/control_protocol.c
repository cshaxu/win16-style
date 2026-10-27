#include "control_protocol.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <limits.h>
#include <wchar.h>

struct ntw16s_control_protocol {
    struct ntw16s_control_protocol *next;
    HWND window;
    unsigned role,refs;
    BOOL runtime_trackbar,runtime_toolbar;
    HMODULE native_module;
};
static _Thread_local ntw16s_control_protocol *bindings;
static const WCHAR key[]=L"ntw16s.private.control-protocol.v1";
/* These official OCX wrappers expose the matching standard control message
 * surface. Admission remains exact: the child and reflector must share one
 * known COMCTL32.OCX/MSCOMCTL.OCX owner; no class prefix is accepted. */
static unsigned ocx_common_control_owner(HWND w,HMODULE owner,const WCHAR *name,WNDCLASSEXW *native)
{
    WCHAR path[MAX_PATH],parent_name[64];HWND parent=GetParent(w);
    const WCHAR *base;
    unsigned role=!lstrcmpW(name,L"TreeViewWndClass") || !lstrcmpW(name,L"TreeView20WndClass")?6:
        !lstrcmpW(name,L"ProgressBarWndClass") || !lstrcmpW(name,L"ProgressBar20WndClass")?7:
        !lstrcmpW(name,L"SliderWndClass") || !lstrcmpW(name,L"Slider20WndClass")?8:0;
    if(!role || !owner ||
        !GetModuleFileNameW(owner,path,ARRAYSIZE(path)))return FALSE;
    base=wcsrchr(path,L'\\');base=base?base+1:path;
    if((lstrcmpiW(base,L"MSCOMCTL.OCX") && lstrcmpiW(base,L"COMCTL32.OCX")) || !parent ||
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)!=owner ||
        !GetClassNameW(parent,parent_name,ARRAYSIZE(parent_name)) ||
        lstrcmpW(parent_name,L"CtlFrameWork_ReflectWindow"))return FALSE;
    native->cbSize=sizeof(*native);
    return GetClassInfoExW(owner,name,native)?role:0;
}
/* Old COMCTL32.OCX hosts a real ToolbarWindow32 below its own wrapper.  The
 * direct parent and both registered module identities are the contract; a
 * class prefix alone is never enough to admit this nested common control. */
static unsigned ocx_toolbar_owner(HWND w,HMODULE owner,const WCHAR *name,WNDCLASSEXW *native,HMODULE *native_owner)
{
    WCHAR path[MAX_PATH],parent_name[64];HWND parent=GetParent(w);HMODULE wrapper;const WCHAR *base;
    if(lstrcmpW(name,TOOLBARCLASSNAMEW) || !owner || !parent ||
        !GetClassNameW(parent,parent_name,ARRAYSIZE(parent_name)) || lstrcmpW(parent_name,L"ToolbarWndClass"))return 0;
    wrapper=(HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE);
    if(!wrapper || !GetModuleFileNameW(wrapper,path,ARRAYSIZE(path)))return 0;
    base=wcsrchr(path,L'\\');base=base?base+1:path;
    if(lstrcmpiW(base,L"COMCTL32.OCX") && lstrcmpiW(base,L"MSCOMCTL.OCX"))return 0;
    native->cbSize=sizeof(*native);
    if(!GetClassInfoExW(NULL,TOOLBARCLASSNAMEW,native) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)(UINT_PTR)native->lpfnWndProc,native_owner) || owner!=*native_owner ||
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native->cbWndExtra)return 0;
    return 10;
}
static BOOL current(const ntw16s_control_protocol *b)
{return IsWindow(b->window) && GetPropW(b->window,key)==(HANDLE)b;}
BOOL ntw16s_control_protocol_is(HWND w,const WCHAR *name)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && current(b))return (b->role==1 && !lstrcmpiW(name,L"BUTTON")) ||
            (b->role==2 && !lstrcmpiW(name,L"COMBOBOX")) || (b->role==3 && !lstrcmpiW(name,WC_TABCONTROLW)) ||
            (b->role==4 && !lstrcmpiW(name,WC_HEADERW)) || (b->role==5 && !lstrcmpiW(name,WC_LISTVIEWW)) || (b->role==6 && !lstrcmpiW(name,WC_TREEVIEWW)) || (b->role==7 && !lstrcmpiW(name,PROGRESS_CLASSW)) || (b->role==8 && !lstrcmpiW(name,TRACKBAR_CLASSW)) || (b->role==9 && !lstrcmpiW(name,UPDOWN_CLASSW)) ||
            (b->role==11 && !lstrcmpiW(name,L"EDIT")) || (b->role==12 && !lstrcmpiW(name,L"LISTBOX"));
    return FALSE;
}
BOOL ntw16s_control_protocol_toolbar(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && b->role==10 && current(b))return TRUE;
    return FALSE;
}
BOOL ntw16s_control_protocol_runtime_toolbar(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && b->role==10 && b->runtime_toolbar && current(b))return TRUE;
    return FALSE;
}
BOOL ntw16s_control_protocol_runtime_trackbar(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && b->role==8 && b->runtime_trackbar && current(b))return TRUE;
    return FALSE;
}
BOOL ntw16s_control_protocol_legacy_progress(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && b->role==13 && current(b))return TRUE;
    return FALSE;
}
HMODULE ntw16s_control_protocol_list_module(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && b->role==5 && current(b))return b->native_module;
    return NULL;
}
HMODULE ntw16s_control_protocol_tree_module(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && b->role==6 && current(b))return b->native_module;
    return NULL;
}
HMODULE ntw16s_control_protocol_updown_module(HWND w)
{
    for(ntw16s_control_protocol *b=bindings;b;b=b->next)
        if(b->window==w && b->role==9 && current(b))return b->native_module;
    return NULL;
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
static ntw16s_control_protocol *bind_protocol(HWND w,HMODULE verified_owner,unsigned verified_role)
{
    WCHAR name[64];unsigned role;DWORD process;WNDCLASSEXW native_class={.cbSize=sizeof(native_class)};
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE),toolbar_native=NULL;
    if(GetWindowThreadProcessId(w,&process)!=GetCurrentThreadId() || process!=GetCurrentProcessId() ||
        !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || GetPropW(w,key))return NULL;
    /* No prefix aliases or private VCL object/procedure decoding. All roles
     * require initialized public protocol evidence, even a tab's real name. */
    unsigned ocx_role=ocx_common_control_owner(w,owner,name,&native_class);
    unsigned toolbar_role=ocx_toolbar_owner(w,owner,name,&native_class,&toolbar_native);
    role=(!lstrcmpW(name,L"TButton") || (verified_owner && (!lstrcmpW(name,L"TGroupButton") || !lstrcmpW(name,L"TRadioButton"))))?1:!lstrcmpW(name,L"TComboBox")?2:
        (!lstrcmpW(name,WC_TABCONTROLW) || !lstrcmpW(name,L"TPageControl") || !lstrcmpW(name,L"TTabControl"))?3:
        (!lstrcmpW(name,WC_HEADERW) || !lstrcmpW(name,L"THeaderControl"))?4:
        (!lstrcmpW(name,WC_LISTVIEWW) || !lstrcmpW(name,L"TListView"))?5:
        (!lstrcmpW(name,WC_TREEVIEWW) || !lstrcmpW(name,L"TTreeView") || !lstrcmpW(name,L"TShellTreeView"))?6:
        (!lstrcmpW(name,PROGRESS_CLASSW) || !lstrcmpW(name,L"TProgressBar"))?7:
        (!lstrcmpW(name,TRACKBAR_CLASSW) || !lstrcmpW(name,L"TTrackBar"))?8:
        (!lstrcmpW(name,UPDOWN_CLASSW) || !lstrcmpW(name,L"TUpDown"))?9:
        (verified_owner && !lstrcmpW(name,L"TToolBar"))?10:!lstrcmpW(name,L"TEdit")?11:!lstrcmpW(name,L"TListBox")?12:ocx_role?ocx_role:toolbar_role;
    if(!role)return NULL;
    if(verified_owner && (role!=verified_role || (role==8 && lstrcmpW(name,L"TTrackBar"))))return NULL;
    if(role>=3 && role<=9 && !ocx_role && !toolbar_role && !GetClassInfoExW(NULL,role==3?WC_TABCONTROLW:role==4?WC_HEADERW:role==5?WC_LISTVIEWW:role==6?WC_TREEVIEWW:role==7?PROGRESS_CLASSW:role==8?TRACKBAR_CLASSW:UPDOWN_CLASSW,&native_class))return NULL;
    HMODULE native_owner=ocx_role?owner:toolbar_role?toolbar_native:NULL;
    /* Global common-control classes can report hInstance == NULL. Resolve
     * the registered native procedure's image, not a guessed DLL filename. */
    if(role>=3 && role<=9 && !ocx_role && !toolbar_role && !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)(UINT_PTR)native_class.lpfnWndProc,&native_owner))return NULL;
    if(owner!=(verified_owner?verified_owner:(ocx_role||toolbar_role)?native_owner:((role==4 && !lstrcmpW(name,WC_HEADERW)) || (role==3 && !lstrcmpW(name,WC_TABCONTROLW)) || (role==5 && !lstrcmpW(name,WC_LISTVIEWW)) || (role==6 && !lstrcmpW(name,WC_TREEVIEWW)) || (role==7 && !lstrcmpW(name,PROGRESS_CLASSW)) || (role==8 && !lstrcmpW(name,TRACKBAR_CLASSW)) || (role==9 && !lstrcmpW(name,UPDOWN_CLASSW)))?native_owner:GetModuleHandleW(NULL)))return NULL;
    if(role==7 && !ocx_role){
        DLLVERSIONINFO version={.cbSize=sizeof(version)};
        HRESULT (CALLBACK *get_version)(DLLVERSIONINFO *)=(void*)GetProcAddress(native_owner,"DllGetVersion");
        if(!get_version || FAILED(get_version(&version)))return NULL;
        if(version.dwMajorVersion==5 && !lstrcmpW(name,L"TProgressBar") &&
            (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_STATICEDGE))role=13;
        else if(version.dwMajorVersion<6)return NULL;
    }
    if(role==3 && ((GetWindowLongPtrW(w,GWL_STYLE)&(TCS_BOTTOM|TCS_VERTICAL)) ||
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native_class.cbWndExtra))return NULL;
    if(role>=4 && role<=9 && GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native_class.cbWndExtra)return NULL;
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
    }else if(role==11){
        WNDCLASSEXW native={.cbSize=sizeof(native)};RECT format={0};
        LRESULT limit=SendMessageW(w,EM_GETLIMITTEXT,0,0);
        LRESULT selection=current(b)?SendMessageW(w,EM_GETSEL,0,0):-1;
        if(current(b))SendMessageW(w,EM_GETRECT,0,(LPARAM)&format);
        valid=current(b) && GetClassInfoExW(NULL,L"EDIT",&native) &&
            GetClassLongPtrW(w,GCL_CBWNDEXTRA)>=(ULONG_PTR)native.cbWndExtra &&
            limit>0 && selection!=-1 && format.right>format.left && format.bottom>format.top &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId();
    }else if(role==12){
        WNDCLASSEXW native={.cbSize=sizeof(native)};DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        LRESULT count=SendMessageW(w,LB_GETCOUNT,0,0);
        LRESULT selected=current(b)?SendMessageW(w,LB_GETCURSEL,0,0):-2;
        LRESULT height=current(b)?SendMessageW(w,LB_GETITEMHEIGHT,0,0):0;
        valid=current(b) && GetClassInfoExW(NULL,L"LISTBOX",&native) &&
            GetClassLongPtrW(w,GCL_CBWNDEXTRA)>=(ULONG_PTR)native.cbWndExtra &&
            !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA)) && count>=0 &&
            (selected==LB_ERR || (selected>=0 && selected<count)) && height>0 &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId();
    }else if(role==13){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
        PBRANGE range={INT_MAX,INT_MIN};LRESULT low=SendMessageW(w,PBM_GETRANGE,TRUE,(LPARAM)&range);
        LRESULT high=current(b)?SendMessageW(w,PBM_GETRANGE,FALSE,0):0;
        LRESULT pos=current(b)?SendMessageW(w,PBM_GETPOS,0,0):0;
        LRESULT state=current(b)?SendMessageW(w,PBM_GETSTATE,0,0):-1;
        valid=current(b) && (ex&WS_EX_STATICEDGE) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_CLIENTEDGE)) &&
            !(style&(PBS_MARQUEE|WS_HSCROLL|WS_VSCROLL)) && range.iLow<=range.iHigh &&
            GetClassLongPtrW(w,GCL_CBWNDEXTRA)>=(ULONG_PTR)native_class.cbWndExtra &&
            (int)(INT_PTR)low==range.iLow && (int)(INT_PTR)high==range.iHigh &&
            (int)(INT_PTR)pos>=range.iLow && (int)(INT_PTR)pos<=range.iHigh && state==0 &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId();
    }else if(role==3){
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
    }else if(role==4){
        RECT rect={0,0,301,203};WINDOWPOS position={0};HDLAYOUT layout={&rect,&position};
        HDHITTESTINFO hit={.pt={-30000,-30000}};
        LRESULT count=SendMessageW(w,HDM_GETITEMCOUNT,0,0);
        LRESULT outside=current(b)?SendMessageW(w,HDM_HITTEST,0,(LPARAM)&hit):0;
        LRESULT laid=current(b)?SendMessageW(w,HDM_LAYOUT,0,(LPARAM)&layout):0;
        valid=current(b) && count>=0 && count<=INT_MAX && outside==-1 && laid &&
            position.x==0 && position.y==0 && position.cx==301 && position.cy>0 && position.cy<=203 &&
            rect.left==0 && rect.top==position.cy && rect.right==301 && rect.bottom==203;
    }
    if(role==5){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        LRESULT count=SendMessageW(w,LVM_GETITEMCOUNT,0,0);
        LRESULT selected=current(b)?SendMessageW(w,LVM_GETSELECTEDCOUNT,0,0):-1;
        HWND header=current(b)?(HWND)SendMessageW(w,LVM_GETHEADER,0,0):NULL;
        WCHAR header_name[64];
        valid=current(b) && (style&LVS_TYPEMASK)==LVS_REPORT && !(style&(LVS_OWNERDRAWFIXED|LVS_OWNERDATA)) &&
            count>=0 && count<=INT_MAX && selected>=0 && selected<=count && header &&
            GetParent(header)==w && GetWindowThreadProcessId(header,NULL)==GetCurrentThreadId() &&
            GetClassNameW(header,header_name,ARRAYSIZE(header_name)) &&
            !lstrcmpW(header_name,WC_HEADERW) &&
            (HMODULE)GetClassLongPtrW(header,GCLP_HMODULE)==native_owner;
        if(valid)b->native_module=native_owner;
    }
    if(role==6){
        LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE),ex=GetWindowLongPtrW(w,GWL_EXSTYLE);
        TVHITTESTINFO hit={.pt={-30000,-30000}};
        LRESULT count=SendMessageW(w,TVM_GETCOUNT,0,0);
        LRESULT indent=current(b)?SendMessageW(w,TVM_GETINDENT,0,0):0;
        LRESULT height=current(b)?SendMessageW(w,TVM_GETITEMHEIGHT,0,0):0;
        LRESULT root=current(b)?SendMessageW(w,TVM_GETNEXTITEM,TVGN_ROOT,0):0;
        LRESULT outside=current(b)?SendMessageW(w,TVM_HITTEST,0,(LPARAM)&hit):1;
        LRESULT code=current(b)?SendMessageW(w,WM_GETDLGCODE,0,0):0;
        valid=current(b) && (code&DLGC_WANTARROWS) && count>=0 && count<=INT_MAX &&
            indent>0 && indent<=32767 && height>0 && height<=32767 && (!count || root) &&
            !outside && !hit.hItem && (hit.flags&(TVHT_NOWHERE|TVHT_ABOVE|TVHT_TOLEFT)) &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
            !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) && (style&WS_CAPTION)!=WS_CAPTION && !(style&WS_THICKFRAME);
        if(valid)b->native_module=native_owner;
    }
    if(role==7){
        PBRANGE range={INT_MAX,INT_MIN};LRESULT low=SendMessageW(w,PBM_GETRANGE,TRUE,(LPARAM)&range);
        LRESULT high=current(b)?SendMessageW(w,PBM_GETRANGE,FALSE,0):0;
        int position=current(b)?(int)(INT_PTR)SendMessageW(w,PBM_GETPOS,0,0):0;
        LRESULT state=current(b)?SendMessageW(w,PBM_GETSTATE,0,0):0;
        valid=current(b) && range.iLow<=range.iHigh && (int)(INT_PTR)low==range.iLow && (int)(INT_PTR)high==range.iHigh &&
            position>=range.iLow && position<=range.iHigh && state>=PBST_NORMAL && state<=PBST_PAUSED &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId();
        /* v5 returns0 for unsupported state/color getters. Do not guess
         * application colors or read undocumented native object storage. */
        if(valid)b->native_module=native_owner;
    }
    if(role==8){
        RECT thumb={0},channel={0};
        LRESULT low=SendMessageW(w,TBM_GETRANGEMIN,0,0);
        LRESULT high=current(b)?SendMessageW(w,TBM_GETRANGEMAX,0,0):0;
        LRESULT position=current(b)?SendMessageW(w,TBM_GETPOS,0,0):0;
        if(current(b))SendMessageW(w,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);
        if(current(b))SendMessageW(w,TBM_GETCHANNELRECT,0,(LPARAM)&channel);
        valid=current(b) && low<=high && position>=low && position<=high &&
            thumb.right>thumb.left && thumb.bottom>thumb.top && channel.right>channel.left && channel.bottom>channel.top &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId();
        if(valid)b->native_module=native_owner;
    }
    if(role==9){
        int low=INT_MAX,high=INT_MIN;SendMessageW(w,UDM_GETRANGE32,(WPARAM)&low,(LPARAM)&high);
        LRESULT base=current(b)?SendMessageW(w,UDM_GETBASE,0,0):0;
        valid=current(b) && !(low==INT_MAX && high==INT_MIN) && (base==10 || base==16) &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId();
        if(valid)b->native_module=native_owner;
    }
    if(role==10){
        LRESULT count=SendMessageW(w,TB_BUTTONCOUNT,0,0);SIZE size={0};
        valid=current(b) && count>=0 && count<=1024 &&
            GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
            SendMessageW(w,TB_GETMAXSIZE,0,(LPARAM)&size) && size.cx>=0 && size.cy>=0;
        /* The OCX can populate public buttons after in-place activation. The
         * painter validates every draw pass and emits no guessed pixels. */
        if(valid)b->native_module=native_owner;
    }
    if(!valid){ntw16s_control_protocol_release(b);return NULL;}
    b->role=role;b->runtime_trackbar=verified_owner && role==8;b->runtime_toolbar=verified_owner && role==10;return b;
}
ntw16s_control_protocol *ntw16s_control_protocol_bind(HWND w)
{return bind_protocol(w,NULL,0);}
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_toolbar(HWND w,HMODULE owner)
{return owner?bind_protocol(w,owner,10):NULL;}
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_group_button(HWND w,HMODULE owner)
{return owner?bind_protocol(w,owner,1):NULL;}
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_edit(HWND w,HMODULE owner)
{return owner?bind_protocol(w,owner,11):NULL;}
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_tab(HWND w,HMODULE owner)
{return owner?bind_protocol(w,owner,3):NULL;}
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_trackbar(HWND w,HMODULE owner)
{return owner?bind_protocol(w,owner,8):NULL;}
