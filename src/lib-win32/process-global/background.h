#ifndef WIN32_PROCESS_GLOBAL_BACKGROUND_H
#define WIN32_PROCESS_GLOBAL_BACKGROUND_H
#include <windows.h>
/* Zero-initialized state and callback context must outlive every scheduled
 * worker. prepare runs on the caller thread; run and successful destroy run
 * on the worker thread. Failed scheduling destroys on the caller thread.
 * prepare's result is owned by the scheduler after it succeeds.
 * A subsequent schedule may begin after run returns while destroy is running;
 * callback context and task destruction must tolerate this overlap. */
typedef struct win32_process_global_background {volatile LONG running,completed;} win32_process_global_background;
typedef struct win32_process_global_background_ops {
    void *(*prepare)(DWORD key,void *context);
    void (*run)(DWORD key,void *task,void *context);
    void (*destroy)(void *task,void *context);
} win32_process_global_background_ops;
void win32_process_global_background_mark(win32_process_global_background *,DWORD);
/* Returns TRUE only if key was already marked complete; FALSE means pending,
 * busy, or failure. No work is performed from DllMain by this API. */
BOOL win32_process_global_background_schedule(win32_process_global_background *,DWORD,const win32_process_global_background_ops *,void *);
#endif
