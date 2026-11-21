#define _WIN32_WINNT 0x0A00
#include "explorer_file.h"
#include "explorer_file_layout.h"
#include "lib-win32-style/application/dialog/classic_file_contract.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "lib-win32/process-hook/unwind.h"
#include <cderr.h>

typedef struct file_observation {
    style_dialog_observation dialog;
    DPI_AWARENESS_CONTEXT dpi;
    style_explorer_file_layout layout;
    classic_file_metrics metrics;
    const style_dialog_windows *windows;
    struct file_observation *previous;
} file_observation;

static _Thread_local file_observation *observing;

static classic_file_metrics file_metrics(const style_dialog_windows *windows)
{
    HMODULE module=windows->resources?windows->resources():NULL;
    return windows->metrics(module).file;
}

static UINT_PTR CALLBACK places_hook(HWND window,UINT message,WPARAM wp,LPARAM lp)
{(void)window;(void)message;(void)wp;(void)lp;return 0;}

static BOOL places_buffer_too_small(const void *buffer,DWORD capacity,SIZE_T unit,WORD offset)
{
    WORD required;
    if(!buffer || capacity<sizeof(required)/unit || (offset && offset<capacity))return FALSE;
    CopyMemory(&required,buffer,sizeof(required));
    /* The legacy Explorer host can return TRUE with an out-of-range or absent
     * offset and an error-sized buffer. Check the output before querying the error. */
    return required>capacity && CommDlgExtendedError()==FNERR_BUFFERTOOSMALL;
}

static LRESULT CALLBACK observe(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    file_observation *call=observing;
    if(call && code>=0 && lp){
        const CWPRETSTRUCT *event=(const CWPRETSTRUCT *)lp;
        HWND dialog=event->hwnd;
        BOOL first=event->message==WM_WINDOWPOSCHANGED && event->lParam &&
            (((const WINDOWPOS *)event->lParam)->flags&SWP_SHOWWINDOW);
        BOOL resized=event->message==WM_SIZE && event->wParam!=SIZE_MINIMIZED;
        if(call->layout!=STYLE_EXPLORER_FILE_NATIVE && (first || resized) && call->windows->attached(dialog)){
            if(call->layout==STYLE_EXPLORER_FILE_COMPACT && first){
                /* The Shell may create a DPI-unaware dialog in an aware caller. */
                DPI_AWARENESS_CONTEXT prior=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                style_explorer_file_fit_compact(dialog);
                if(prior)SetThreadDpiAwarenessContext(prior);
            }else if(call->layout==STYLE_EXPLORER_FILE_PLACES){
                DPI_AWARENESS_CONTEXT prior=SetThreadDpiAwarenessContext(call->dpi);
                if(first){
                    MONITORINFO monitor={.cbSize=sizeof(monitor)};
                    if(GetMonitorInfoW(MonitorFromWindow(dialog,MONITOR_DEFAULTTONEAREST),&monitor))
                        style_explorer_file_fit_places(dialog,&monitor.rcWork,&call->metrics);
                }else style_explorer_file_fit_place_rows(style_explorer_file_toolbar(dialog,1184));
                if(prior)SetThreadDpiAwarenessContext(prior);
            }
        }
        if(style_message_dialog_initialized(code,lp))call->windows->initialized(dialog);
    }
    SetLastError(incoming);
    return CallNextHookEx(NULL,code,wp,lp);
}

static void end_observation(void *data)
{
    file_observation *scope=data;
    style_dialog_observation_end(&scope->dialog);
    observing=scope->previous;
}

#define FILE_CALL(SUFFIX,TYPE) \
PROCESS_HOOK_UNWIND_FUNCTION BOOL style_explorer_file_##SUFFIX(TYPE *parameters,BOOL (WINAPI *native)(TYPE *),const style_dialog_windows *windows) \
{ \
    DWORD incoming=GetLastError(); \
    classic_file_metrics metrics={0}; \
    style_explorer_file_layout layout=STYLE_EXPLORER_FILE_NATIVE; \
    if(parameters && parameters->lStructSize==sizeof(*parameters) && style_explorer_file_stock(parameters->Flags,parameters->FlagsEx)){ \
        metrics=file_metrics(windows); \
        layout=style_explorer_file_layout_at(parameters->hwndOwner,parameters->Flags,parameters->FlagsEx,&metrics); \
    } \
    TYPE local; \
    if(layout!=STYLE_EXPLORER_FILE_NATIVE){ \
        local=*parameters;local.Flags|=OFN_ENABLESIZING; \
        if(layout==STYLE_EXPLORER_FILE_COMPACT)local.FlagsEx|=OFN_EX_NOPLACESBAR; \
        else{local.Flags|=OFN_ENABLEHOOK;local.lpfnHook=places_hook;} \
    } \
    file_observation scope={.dpi=GetThreadDpiAwarenessContext(),.layout=layout,.metrics=metrics,.windows=windows,.previous=observing}; \
    process_hook_unwind_scope unwind; \
    if(!style_dialog_observation_begin(&scope.dialog,NULL,observe,FALSE))return FALSE; \
    observing=&scope; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_observation,&scope); \
    SetLastError(incoming); \
    BOOL result=native(layout==STYLE_EXPLORER_FILE_NATIVE?parameters:&local);DWORD error=GetLastError(); \
    PROCESS_HOOK_UNWIND_END(unwind); \
    if(result && layout==STYLE_EXPLORER_FILE_PLACES && !(parameters->Flags&OFN_NOVALIDATE) && \
        places_buffer_too_small(local.lpstrFile,local.nMaxFile,sizeof(*local.lpstrFile),local.nFileOffset))result=FALSE; \
    if(layout!=STYLE_EXPLORER_FILE_NATIVE)style_classic_file_result_##SUFFIX(parameters,&local); \
    SetLastError(error);return result; \
}
FILE_CALL(a,OPENFILENAMEA)
FILE_CALL(w,OPENFILENAMEW)
