/* Project-authored test witness: exact newly owned GUI thread only.
 * Observes notifications and acknowledges dequeuing a tagged FIFO barrier.
 * Survey requests repaint/print only the explicitly owned target and may open
 * and close its own combo popup. It preserves messages and application returns. */
#include "vb_queue.h"
#include <wchar.h>
#include <string.h>
#include <stdio.h>
static HANDLE mapping,event;
static vb_queue_state *state;
static UINT barrier;
static BOOL sampling;
static HGDIOBJ (WINAPI *select_object)(HDC,HGDIOBJ);
typedef struct import_observer {PVOID volatile *slot;PVOID original,replacement;} import_observer;
static import_observer imports[3];static unsigned import_count;
static BOOL observer_reject(const char *api,const char *stage,HMODULE module,DWORD error)
{
    SetLastError(error);
    fprintf(stderr,"VB_QUEUE_IMPORT_REJECT api=%s stage=%s module=%p error=%lu slots=%u\n",
        api,stage,(void *)module,error,import_count);
    return FALSE;
}
static HGDIOBJ WINAPI observe_select(HDC dc,HGDIOBJ object)
{
    HGDIOBJ result=select_object(dc,object);DWORD error=GetLastError();POINT origin,client={0,0};HWND window=WindowFromDC(dc);
    if(sampling && GetCurrentThreadId()==state->tid && (!window || window==state->window) &&
            result && result!=HGDI_ERROR && GetObjectType(object)==OBJ_FONT && GetCurrentObject(dc,OBJ_FONT)==object &&
            (!window || (GetDCOrgEx(dc,&origin) && ClientToScreen(state->window,&client) && origin.x==client.x && origin.y==client.y))){
        WCHAR face[80];if(GetTextFaceW(dc,80,face)>0){
            if(!wcsncmp(face,L"Win16 System",12)){state->paint_system_font=(HFONT)object;if(!state->paint_system_callsite)state->paint_system_callsite=(ULONG_PTR)__builtin_return_address(0);if(window)++state->paint_system;else ++state->paint_system_memory;}
            else {if(window)++state->paint_native;else ++state->paint_native_memory;}
        }
    }
    SetLastError(error);return result;
}
/* Test-only atomic IAT observation in the owned process; no API entry patch,
 * trampoline replacement, thread freeze or system-wide hook. */
static BOOL observe_api_import(HMODULE module,const char *api,PVOID expected,PVOID replacement)
{
    BYTE *base=(BYTE *)module;IMAGE_DOS_HEADER *dos=(void *)base;
    if(!module || dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 || dos->e_lfanew>4096)return observer_reject(api,"dos",module,ERROR_BAD_EXE_FORMAT);
    IMAGE_NT_HEADERS *nt=(void *)(base+dos->e_lfanew);DWORD size=nt->OptionalHeader.SizeOfImage;
    if(nt->Signature!=IMAGE_NT_SIGNATURE)return observer_reject(api,"nt",module,ERROR_BAD_EXE_FORMAT);
    IMAGE_DATA_DIRECTORY directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if(!directory.VirtualAddress || directory.Size<sizeof(IMAGE_IMPORT_DESCRIPTOR) || directory.VirtualAddress>size || directory.Size>size-directory.VirtualAddress)return observer_reject(api,"directory",module,ERROR_BAD_EXE_FORMAT);
    IMAGE_IMPORT_DESCRIPTOR *entry=(void *)(base+directory.VirtualAddress);
    for(unsigned i=0;i<directory.Size/sizeof(*entry) && entry[i].Name;++i){
        DWORD names=entry[i].OriginalFirstThunk,addresses=entry[i].FirstThunk;
        if(!names || !addresses || names>=size || addresses>=size)continue;
        for(unsigned j=0;j<(size-names)/sizeof(IMAGE_THUNK_DATA) && j<(size-addresses)/sizeof(IMAGE_THUNK_DATA);++j){
            IMAGE_THUNK_DATA *name=(void *)(base+names+j*sizeof(*name)),*address=(void *)(base+addresses+j*sizeof(*address));
            if(!name->u1.AddressOfData)break;
            if(IMAGE_SNAP_BY_ORDINAL(name->u1.Ordinal) || name->u1.AddressOfData>=size)continue;
            IMAGE_IMPORT_BY_NAME *symbol=(void *)(base+name->u1.AddressOfData);
            size_t length=strlen(api)+1;
            if(size-name->u1.AddressOfData<sizeof(WORD)+length || memcmp(symbol->Name,api,length))continue;
            if(import_count==ARRAYSIZE(imports) || (PVOID)(UINT_PTR)address->u1.Function!=expected){
                WCHAR path[MAX_PATH]=L"";HMODULE owner=NULL;
                if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        (LPCWSTR)(UINT_PTR)address->u1.Function,&owner))GetModuleFileNameW(owner,path,MAX_PATH);
                fprintf(stderr,"VB_QUEUE_IMPORT_MISMATCH api=%s module=%p actual=%p expected=%p slots=%u actual_owner=%ls\n",
                    api,(void *)module,(void *)(UINT_PTR)address->u1.Function,expected,import_count,path);
                return observer_reject(api,import_count==ARRAYSIZE(imports)?"capacity":"expected-address",module,ERROR_INVALID_ADDRESS);
            }
            PVOID volatile *slot=(void *)&address->u1.Function;DWORD protect,unused;
            if(!VirtualProtect((void *)slot,sizeof(*slot),PAGE_READWRITE,&protect))return observer_reject(api,"protect-write",module,GetLastError());
            PVOID old=InterlockedCompareExchangePointer(slot,replacement,expected);
            BOOL restored=VirtualProtect((void *)slot,sizeof(*slot),protect,&unused);
            if(old!=expected){
                WCHAR path[MAX_PATH]=L"";HMODULE owner=NULL;
                if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        (LPCWSTR)old,&owner))GetModuleFileNameW(owner,path,MAX_PATH);
                fprintf(stderr,"VB_QUEUE_IMPORT_RACE api=%s module=%p slot=%p actual=%p expected=%p actual_owner=%ls\n",
                    api,(void *)module,(void *)slot,old,expected,path);
                return observer_reject(api,"compare-exchange",module,ERROR_INVALID_ADDRESS);
            }
            imports[import_count++]=(import_observer){slot,old,replacement};
            if(!restored)return observer_reject(api,"protect-restore",module,GetLastError());
            return TRUE;
        }
    }
    return observer_reject(api,"import-not-found",module,ERROR_PROC_NOT_FOUND);
}
static BOOL observe_import(HMODULE module)
{return observe_api_import(module,"SelectObject",(PVOID)(UINT_PTR)select_object,(PVOID)(UINT_PTR)observe_select);}
static BOOL unobserve_imports(void)
{
    BOOL ok=TRUE;
    while(import_count){import_observer *entry=&imports[--import_count];DWORD protect,unused;
        if(!VirtualProtect((void *)entry->slot,sizeof(*entry->slot),PAGE_READWRITE,&protect)){
            fprintf(stderr,"VB_QUEUE_IMPORT_RESTORE_FAILED stage=protect-write slot=%p error=%lu\n",(void *)entry->slot,GetLastError());ok=FALSE;continue;
        }
        if(InterlockedCompareExchangePointer(entry->slot,entry->original,entry->replacement)!=entry->replacement){
            fprintf(stderr,"VB_QUEUE_IMPORT_RESTORE_FAILED stage=compare-exchange slot=%p error=%lu\n",(void *)entry->slot,ERROR_INVALID_ADDRESS);ok=FALSE;
        }
        if(!VirtualProtect((void *)entry->slot,sizeof(*entry->slot),protect,&unused)){
            fprintf(stderr,"VB_QUEUE_IMPORT_RESTORE_FAILED stage=protect-restore slot=%p error=%lu\n",(void *)entry->slot,GetLastError());ok=FALSE;
        }
    }
    return ok;
}
static BOOL (WINAPI *draw_frame)(HDC,LPRECT,UINT,UINT);
static BOOL (WINAPI *bit_blt)(HDC,int,int,int,int,HDC,int,int,DWORD);
static int (WINAPI *draw_text)(HDC,LPCSTR,int,LPRECT,UINT);
static BOOL WINAPI observe_frame(HDC dc,LPRECT rect,UINT type,UINT flags)
{
    DWORD error=GetLastError();++state->graphical_frames;state->graphical_type=type;state->graphical_state=flags;
    state->graphical_dc_window=WindowFromDC(dc);if(rect)state->graphical_rect=*rect;
    SetLastError(error);return draw_frame(dc,rect,type,flags);
}
static BOOL WINAPI observe_blt(HDC dc,int x,int y,int cx,int cy,HDC from,int sx,int sy,DWORD rop)
{++state->graphical_blits;return bit_blt(dc,x,y,cx,cy,from,sx,sy,rop);}
static int WINAPI observe_draw_text(HDC dc,LPCSTR text,int count,LPRECT rect,UINT format)
{++state->graphical_text;return draw_text(dc,text,count,rect,format);}
static void graphical_survey(void)
{
    state->graphical_frames=state->graphical_blits=state->graphical_text=0;
    state->graphical_type=state->graphical_state=0;state->graphical_dc_window=NULL;SetRectEmpty(&state->graphical_rect);
    HMODULE runtime=(HMODULE)GetClassLongPtrW(state->window,GCLP_HMODULE);
    draw_frame=(void *)GetProcAddress(GetModuleHandleW(L"USER32.dll"),"DrawFrameControl");
    draw_text=(void *)GetProcAddress(GetModuleHandleW(L"USER32.dll"),"DrawTextA");
    bit_blt=(void *)GetProcAddress(GetModuleHandleW(L"GDI32.dll"),"BitBlt");
    const char *stage="resolve";
    BOOL ok=draw_frame && draw_text && bit_blt;
    if(ok){stage="DrawFrameControl";ok=observe_api_import(runtime,"DrawFrameControl",(PVOID)(UINT_PTR)draw_frame,(PVOID)(UINT_PTR)observe_frame);}
    if(ok){stage="DrawTextA";ok=observe_api_import(runtime,"DrawTextA",(PVOID)(UINT_PTR)draw_text,(PVOID)(UINT_PTR)observe_draw_text);}
    if(ok){stage="BitBlt";ok=observe_api_import(runtime,"BitBlt",(PVOID)(UINT_PTR)bit_blt,(PVOID)(UINT_PTR)observe_blt);}
    if(ok)RedrawWindow(state->window,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    DWORD error=GetLastError();unsigned installed_slots=import_count;state->paint_restored=unobserve_imports();state->paint_probe_error=ok && state->paint_restored?0:ERROR_INVALID_STATE;
    if(state->paint_probe_error)fprintf(stderr,"VB_QUEUE_GRAPHICAL_OBSERVER_FAILED module=%p stage=%s install=%u restored=%ld installed_slots=%u native_error=%lu\n",
        (void *)runtime,stage,ok,state->paint_restored,installed_slots,error);
}
static BOOL (WINAPI *text_out)(HDC,int,int,LPCSTR,int);
static BOOL (WINAPI *ext_text_out)(HDC,int,int,UINT,const RECT *,LPCSTR,UINT,const INT *);
static LONG (WINAPI *tabbed_text_out)(HDC,int,int,LPCSTR,int,int,const INT *,int);
static BOOL WINAPI observe_print(HDC dc,int x,int y,LPCSTR text,int count)
{
    DWORD entry=GetLastError();
    if(GetCurrentThreadId()==state->tid && text && count>=11 && !memcmp(text,"Photo Album",11)){
        ++state->print_calls;state->print_alignment=GetTextAlign(dc);state->print_api=1;
    }
    SetLastError(entry);return text_out(dc,x,y,text,count);
}
static BOOL WINAPI observe_print_ext(HDC dc,int x,int y,UINT options,const RECT *rect,LPCSTR text,UINT count,const INT *dx)
{
    DWORD entry=GetLastError();
    if(GetCurrentThreadId()==state->tid && text && count>=11 && !memcmp(text,"Photo Album",11)){
        ++state->print_calls;state->print_alignment=GetTextAlign(dc);state->print_api=2;
    }
    SetLastError(entry);return ext_text_out(dc,x,y,options,rect,text,count,dx);
}
static LONG WINAPI observe_print_tabbed(HDC dc,int x,int y,LPCSTR text,int count,int stops,const INT *tabs,int origin)
{
    DWORD entry=GetLastError();
    if(GetCurrentThreadId()==state->tid && text && count>=11 && !memcmp(text,"Photo Album",11)){
        ++state->print_calls;state->print_alignment=GetTextAlign(dc);state->print_api=3;
    }
    SetLastError(entry);return tabbed_text_out(dc,x,y,text,count,stops,tabs,origin);
}
static void print_sample(void)
{
    state->print_calls=0;state->print_alignment=state->print_api=0;state->paint_probe_error=0;state->paint_restored=0;
    text_out=(void *)GetProcAddress(GetModuleHandleW(L"GDI32.dll"),"TextOutA");
    ext_text_out=(void *)GetProcAddress(GetModuleHandleW(L"GDI32.dll"),"ExtTextOutA");
    tabbed_text_out=(void *)GetProcAddress(GetModuleHandleW(L"USER32.dll"),"TabbedTextOutA");
    HMODULE runtime=(HMODULE)GetClassLongPtrW(state->window,GCLP_HMODULE);
    BOOL ok=text_out && ext_text_out && observe_api_import(runtime,"TextOutA",(PVOID)(UINT_PTR)text_out,(PVOID)(UINT_PTR)observe_print) &&
        observe_api_import(runtime,"ExtTextOutA",(PVOID)(UINT_PTR)ext_text_out,(PVOID)(UINT_PTR)observe_print_ext) &&
        tabbed_text_out && observe_api_import(runtime,"TabbedTextOutA",(PVOID)(UINT_PTR)tabbed_text_out,(PVOID)(UINT_PTR)observe_print_tabbed);
    if(ok)SendMessageW(state->window,BM_CLICK,0,0);
    state->paint_restored=unobserve_imports();
    if(!ok || !state->paint_restored)state->paint_probe_error=ERROR_INVALID_STATE;
}
static void paint_survey(HMODULE style)
{
    state->paint_messages=state->paint_system=state->paint_native=state->paint_probe_error=state->paint_restored=0;
    state->paint_system_memory=state->paint_native_memory=0;
    state->paint_system_callsite=0;
    state->paint_system_font=NULL;
    /* Warm native backing surfaces/font leases before observing one repaint. */
    RedrawWindow(state->window,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    select_object=(void *)GetProcAddress(GetModuleHandleW(L"GDI32.dll"),"SelectObject");
    HMODULE runtime=(HMODULE)GetClassLongPtrW(state->window,GCLP_HMODULE);
    BOOL ok=select_object && observe_import(runtime) && (!style || style==runtime || observe_import(style));
    sampling=ok;
    /* Client repaint only: do not count caption/nonclient font selection. */
    if(ok)RedrawWindow(state->window,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    sampling=FALSE;BOOL restored=unobserve_imports();state->paint_restored=restored;
    if(!ok || !restored)state->paint_probe_error=ERROR_INVALID_STATE;
}
typedef struct font_trace {BOOL fonts[1024],system[1024],selected,saved[64];unsigned depth;} font_trace;
static int CALLBACK font_record(HDC dc,HANDLETABLE *table,const ENHMETARECORD *record,int count,LPARAM data)
{
    (void)dc;(void)table;(void)count;font_trace *trace=(font_trace *)data;
    if(record->iType==EMR_EXTCREATEFONTINDIRECTW){
        const EMREXTCREATEFONTINDIRECTW *font=(const void *)record;
        ++state->font_records;
        BOOL system=!wcsncmp(font->elfw.elfLogFont.lfFaceName,L"Win16 System",12);
        if(system)++state->system_fonts;
        if(font->ihFont<1024){trace->fonts[font->ihFont]=TRUE;trace->system[font->ihFont]=system;}
    }else if(record->iType==EMR_SELECTOBJECT){
        DWORD object=((const EMRSELECTOBJECT *)record)->ihObject;
        if(object<1024 && trace->fonts[object])trace->selected=trace->system[object];
        else if(object&0x80000000u){DWORD stock=object&0x7fffffffu;
            if(stock==SYSTEM_FONT || stock==SYSTEM_FIXED_FONT || stock==ANSI_FIXED_FONT || stock==ANSI_VAR_FONT || stock==DEFAULT_GUI_FONT)trace->selected=FALSE;}
    }else if(record->iType==EMR_SAVEDC){
        if(trace->depth<64){trace->saved[trace->depth]=trace->selected;++trace->depth;}
    }else if(record->iType==EMR_RESTOREDC){
        LONG relative=((const EMRRESTOREDC *)record)->iRelative;
        LONG slot=relative<0?(LONG)trace->depth+relative:relative-1;
        if(slot>=0 && slot<(LONG)trace->depth){trace->selected=trace->saved[slot];trace->depth=(unsigned)slot;}
    }else if(record->iType==EMR_EXTTEXTOUTW || record->iType==EMR_EXTTEXTOUTA || record->iType==EMR_POLYTEXTOUTA || record->iType==EMR_POLYTEXTOUTW){
        if(trace->selected)++state->system_text;else ++state->native_text;
    }
    return 1;
}
static void survey(void)
{
    HMODULE module=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=module?(void *)GetProcAddress(module,"Ntw16sWindowAttached"):NULL;
    state->attached=attached?attached(state->window):-1;
    state->font_records=state->system_fonts=state->draw_items=state->popup_dc=state->system_text=state->native_text=0;
    state->draw_dc_window=NULL;state->draw_type=0;
    if(state->action==VB_QUEUE_GRAPHICAL_SURVEY)graphical_survey();
    else if(state->action==VB_QUEUE_PAINT_SURVEY)paint_survey(module);
    else RedrawWindow(state->window,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_FRAME);
    if(state->action==VB_QUEUE_POPUP_SURVEY){
        COMBOBOXINFO combo={.cbSize=sizeof(combo)};
        if(GetComboBoxInfo(state->window,&combo)){
            SendMessageW(state->window,CB_SHOWDROPDOWN,TRUE,0);
            RedrawWindow(combo.hwndList,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
            SendMessageW(state->window,CB_SHOWDROPDOWN,FALSE,0);
        }
    }
    HDC dc=CreateEnhMetaFileW(NULL,NULL,NULL,NULL);
    if(dc){
        SendMessageW(state->window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
        HENHMETAFILE image=CloseEnhMetaFile(dc);
        if(image){font_trace trace={0};EnumEnhMetaFile(NULL,image,font_record,&trace,NULL);DeleteEnhMetaFile(image);}
    }
}
static BOOL witness_connect(void)
{
    if(!state){
        WCHAR name[128];
        if(!GetEnvironmentVariableW(VB_QUEUE_ENV,name,ARRAYSIZE(name)))return FALSE;
        mapping=OpenFileMappingW(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,name);
        if(!mapping)return FALSE;
        state=MapViewOfFile(mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*state));
        if(!state){CloseHandle(mapping);mapping=NULL;return FALSE;}
        barrier=RegisterWindowMessageW(VB_QUEUE_MESSAGE);
        event=OpenEventW(EVENT_MODIFY_STATE,FALSE,state->event_name);
    }
    return event && barrier && state->size==sizeof(*state) &&
        state->pid==GetCurrentProcessId() && state->tid==GetCurrentThreadId();
}
__declspec(dllexport) LRESULT CALLBACK VbQueueGet(int code,WPARAM a,LPARAM b)
{
    DWORD error=GetLastError();
    if(code>=0 && a==PM_REMOVE && witness_connect()){
        const MSG *message=(const MSG *)b;
        if(message->message==barrier && message->hwnd==state->window){
            if(message->wParam!=state->generation)InterlockedIncrement(&state->rejected);
            else {
                if(state->action==VB_QUEUE_SURVEY || state->action==VB_QUEUE_POPUP_SURVEY || state->action==VB_QUEUE_PAINT_SURVEY || state->action==VB_QUEUE_GRAPHICAL_SURVEY)survey();
                if(state->action==VB_QUEUE_PRINT_SAMPLE)print_sample();
                InterlockedIncrement(&state->acknowledgements);
                InterlockedExchange(&state->completed,(LONG)state->generation);
                SetEvent(event);
            }
        }
    }
    SetLastError(error);return CallNextHookEx(NULL,code,a,b);
}
__declspec(dllexport) LRESULT CALLBACK VbQueueSent(int code,WPARAM a,LPARAM b)
{
    DWORD error=GetLastError();
    if(code>=0 && witness_connect()){
        const CWPSTRUCT *message=(const CWPSTRUCT *)b;
        if(sampling && message->hwnd==state->window && message->message==WM_PAINT)++state->paint_messages;
        if(message->message==WM_DRAWITEM && message->hwnd==state->root && message->lParam){
            const DRAWITEMSTRUCT *item=(const void *)message->lParam;
            if(item->hwndItem==state->window){
                ++state->draw_items;state->draw_type=item->CtlType;
                HWND dc_window=WindowFromDC(item->hDC);
                if(dc_window){
                    state->draw_dc_window=dc_window;
                    COMBOBOXINFO combo={.cbSize=sizeof(combo)};
                    if(GetComboBoxInfo(state->window,&combo) && dc_window==combo.hwndList)++state->popup_dc;
                }
            }
        }
        if(message->hwnd==state->root && (HWND)message->lParam==state->window){
            if(message->message==WM_COMMAND){
                state->command_code=HIWORD(message->wParam);
                InterlockedIncrement(&state->commands);
            }else if(message->message==WM_HSCROLL || message->message==WM_VSCROLL){
                state->scroll_code=LOWORD(message->wParam);
                InterlockedIncrement(message->message==WM_HSCROLL?&state->hscroll:&state->vscroll);
            }
        }
    }
    SetLastError(error);return CallNextHookEx(NULL,code,a,b);
}
/* The owning GUI thread calls this after removing both hooks. Windows may
 * retain its hook module reference until later message dispatch; release the
 * witness's mapping/event independently of that loader lifetime. */
__declspec(dllexport) BOOL WINAPI VbQueueDisconnect(void)
{
    if(sampling || import_count)return FALSE;
    BOOL ok=TRUE;
    if(state){ok=UnmapViewOfFile(state) && ok;state=NULL;}
    if(event){ok=CloseHandle(event) && ok;event=NULL;}
    if(mapping){ok=CloseHandle(mapping) && ok;mapping=NULL;}
    barrier=0;return ok;
}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID reserved)
{
    (void)instance;(void)reserved;
    if(reason==DLL_PROCESS_DETACH){
        VbQueueDisconnect();
    }
    return TRUE;
}
