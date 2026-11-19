#define _WIN32_WINNT 0x0A00
#include "file_call.h"
#include "file.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/dialog/classic_file_contract.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "lib-win32/process-hook/unwind.h"
#include <cderr.h>

typedef struct file_observation {
    style_dialog_observation dialog;
    DPI_AWARENESS_CONTEXT dpi;
    win2k_file_layout layout;
    classic_file_metrics metrics;
    const style_dialog_windows *windows;
    struct file_observation *previous;
} file_observation;

static _Thread_local file_observation *observing;

static classic_file_metrics file_metrics(const style_dialog_windows *windows)
{
    HMODULE module=windows->resources?windows->resources():NULL;
    return classic_theme_dialog_metrics(module,(classic_dialog_metrics)WIN2000_DIALOG_METRICS).file;
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
        if(call->layout!=WIN2K_FILE_NATIVE && (first || resized) && call->windows->attached(dialog)){
            if(call->layout==WIN2K_FILE_COMPACT && first){
                /* The Shell may create a DPI-unaware dialog in an aware caller. */
                DPI_AWARENESS_CONTEXT prior=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                win2k_file_fit_compact(dialog);
                if(prior)SetThreadDpiAwarenessContext(prior);
            }else if(call->layout==WIN2K_FILE_PLACES){
                DPI_AWARENESS_CONTEXT prior=SetThreadDpiAwarenessContext(call->dpi);
                if(first){
                    MONITORINFO monitor={.cbSize=sizeof(monitor)};
                    if(GetMonitorInfoW(MonitorFromWindow(dialog,MONITOR_DEFAULTTONEAREST),&monitor))
                        win2k_file_fit_places(dialog,&monitor.rcWork,&call->metrics);
                }else win2k_file_fit_place_rows(win2k_file_toolbar(dialog,1184));
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
PROCESS_HOOK_UNWIND_FUNCTION BOOL win2k_file_call_##SUFFIX(TYPE *parameters,BOOL (WINAPI *native)(TYPE *),const style_dialog_windows *windows) \
{ \
    DWORD incoming=GetLastError(); \
    classic_file_metrics metrics={0}; \
    win2k_file_layout layout=WIN2K_FILE_NATIVE; \
    if(parameters && parameters->lStructSize==sizeof(*parameters) && win2k_file_stock(parameters->Flags,parameters->FlagsEx)){ \
        metrics=file_metrics(windows); \
        layout=win2k_file_layout_at(parameters->hwndOwner,parameters->Flags,parameters->FlagsEx,&metrics); \
    } \
    TYPE local; \
    if(layout!=WIN2K_FILE_NATIVE){ \
        local=*parameters;local.Flags|=OFN_ENABLESIZING; \
        if(layout==WIN2K_FILE_COMPACT)local.FlagsEx|=OFN_EX_NOPLACESBAR; \
        else{local.Flags|=OFN_ENABLEHOOK;local.lpfnHook=places_hook;} \
    } \
    file_observation scope={.dpi=GetThreadDpiAwarenessContext(),.layout=layout,.metrics=metrics,.windows=windows,.previous=observing}; \
    process_hook_unwind_scope unwind; \
    if(!style_dialog_observation_begin(&scope.dialog,NULL,observe,FALSE))return FALSE; \
    observing=&scope; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_observation,&scope); \
    SetLastError(incoming); \
    BOOL result=native(layout==WIN2K_FILE_NATIVE?parameters:&local);DWORD error=GetLastError(); \
    PROCESS_HOOK_UNWIND_END(unwind); \
    if(result && layout==WIN2K_FILE_PLACES && !(parameters->Flags&OFN_NOVALIDATE) && \
        places_buffer_too_small(local.lpstrFile,local.nMaxFile,sizeof(*local.lpstrFile),local.nFileOffset))result=FALSE; \
    if(layout!=WIN2K_FILE_NATIVE)style_classic_file_result_##SUFFIX(parameters,&local); \
    SetLastError(error);return result; \
}
FILE_CALL(a,OPENFILENAMEA)
FILE_CALL(w,OPENFILENAMEW)
