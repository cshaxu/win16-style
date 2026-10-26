/* Project-authored test witness: exact newly owned GUI thread only.
 * Observes notifications and acknowledges dequeuing a tagged FIFO barrier.
 * Survey requests repaint/print only the explicitly owned target and may open
 * and close its own combo popup. It preserves messages and application returns. */
#include "vb_queue.h"
#include <wchar.h>
static HANDLE mapping,event;
static vb_queue_state *state;
static UINT barrier;
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
    RedrawWindow(state->window,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_FRAME);
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
                if(state->action==VB_QUEUE_SURVEY || state->action==VB_QUEUE_POPUP_SURVEY)survey();
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
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID reserved)
{
    (void)instance;(void)reserved;
    if(reason==DLL_PROCESS_DETACH){
        if(state)UnmapViewOfFile(state);
        if(event)CloseHandle(event);
        if(mapping)CloseHandle(mapping);
    }
    return TRUE;
}
