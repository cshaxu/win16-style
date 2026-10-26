/* Project-authored test witness: exact newly owned GUI thread only.
 * Observes notifications and acknowledges dequeuing a tagged FIFO barrier.
 * Does not change messages, window procedures or application return values. */
#include "vb_queue.h"
static HANDLE mapping,event;
static vb_queue_state *state;
static UINT barrier;
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
    if(code>=0 && a==PM_REMOVE && witness_connect()){
        const MSG *message=(const MSG *)b;
        if(message->message==barrier && message->hwnd==state->window){
            if(message->wParam!=state->generation)InterlockedIncrement(&state->rejected);
            else {
                InterlockedIncrement(&state->acknowledgements);
                InterlockedExchange(&state->completed,(LONG)state->generation);
                SetEvent(event);
            }
        }
    }
    return CallNextHookEx(NULL,code,a,b);
}
__declspec(dllexport) LRESULT CALLBACK VbQueueSent(int code,WPARAM a,LPARAM b)
{
    if(code>=0 && witness_connect()){
        const CWPSTRUCT *message=(const CWPSTRUCT *)b;
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
    return CallNextHookEx(NULL,code,a,b);
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
