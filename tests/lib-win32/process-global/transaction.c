#include "lib-win32/process-global/transaction.h"
#include <stdio.h>
typedef struct test_state {unsigned calls[5];DWORD load,validate,start,ready,apply;} test_state;
static DWORD load(void *p){test_state *s=p;++s->calls[0];return s->load;}
static DWORD validate(void *p){test_state *s=p;++s->calls[1];return s->validate;}
static DWORD start(void *p){test_state *s=p;++s->calls[2];return s->start;}
static DWORD ready(void *p){test_state *s=p;++s->calls[3];return s->ready;}
static DWORD apply(void *p){test_state *s=p;++s->calls[4];return s->apply;}
typedef struct run_state {HANDLE event;unsigned begins,advances,applies;DWORD status;BOOL alive,retry,empty,stuck;} run_state;
static BOOL run_active(void *p){return ((run_state *)p)->alive;}
static DWORD run_begin(void *p){run_state *s=p;return ++s->begins==1?ERROR_NOT_READY:ERROR_SUCCESS;}
static DWORD run_advance(void *p){run_state *s=p;return ++s->advances==1 || (s->stuck && s->advances<10000)?ERROR_IO_PENDING:s->status;}
static unsigned run_pending(void *p,HANDLE handles[2]){run_state *s=p;if(s->empty)return 0;handles[0]=s->event;return 1;}
static BOOL run_retry(void *p,DWORD status){return ((run_state *)p)->retry && status==ERROR_TIMEOUT;}
static DWORD run_apply(void *p){++((run_state *)p)->applies;return ERROR_SUCCESS;}
int main(void)
{
    static const win32_process_global_transaction_ops ops={load,validate,start,ready,apply};
    test_state s={0};DWORD phase=WIN32_PROCESS_GLOBAL_LOADING,control=0;s.load=ERROR_TIMEOUT;
    if(win32_process_global_transaction_advance(&phase,&control,TRUE,&s,&ops)!=ERROR_IO_PENDING || phase!=WIN32_PROCESS_GLOBAL_LOADING || s.calls[1])return 1;
    s.load=0;s.ready=ERROR_TIMEOUT;
    if(win32_process_global_transaction_advance(&phase,&control,TRUE,&s,&ops)!=ERROR_IO_PENDING || phase!=WIN32_PROCESS_GLOBAL_STARTING || s.calls[2]!=1 || s.calls[4])return 2;
    s.ready=0;s.apply=ERROR_NOT_READY;
    if(win32_process_global_transaction_advance(&phase,&control,TRUE,&s,&ops)!=ERROR_NOT_READY || phase!=WIN32_PROCESS_GLOBAL_ACTIVE || control!=ERROR_NOT_READY || s.calls[4]!=1)return 3;
    if(win32_process_global_transaction_advance(&phase,&control,TRUE,&s,&ops)!=ERROR_NOT_READY || s.calls[4]!=1)return 4;
    phase=WIN32_PROCESS_GLOBAL_LOADING;control=0;s.validate=ERROR_ACCESS_DENIED;
    if(win32_process_global_transaction_advance(&phase,&control,FALSE,&s,&ops)!=ERROR_ACCESS_DENIED || phase!=WIN32_PROCESS_GLOBAL_LOADING || s.calls[2]!=1)return 5;
    s.validate=0;phase=WIN32_PROCESS_GLOBAL_LOADING;
    if(win32_process_global_transaction_advance(&phase,&control,FALSE,&s,&ops) || phase!=WIN32_PROCESS_GLOBAL_ACTIVE || s.calls[4]!=1)return 6;
    if(win32_process_global_transaction_advance(NULL,&control,FALSE,&s,&ops)!=ERROR_INVALID_PARAMETER)return 7;
    HANDLE quit=CreateEventW(NULL,TRUE,FALSE,NULL),stop=CreateEventW(NULL,TRUE,FALSE,NULL),event=CreateEventW(NULL,TRUE,TRUE,NULL);
    if(!quit || !stop || !event)return 8;
    const win32_process_global_transaction_run_ops run_ops={run_active,run_begin,run_advance,run_pending,run_retry,run_apply};
    run_state r={.event=event,.alive=TRUE,.status=ERROR_TIMEOUT,.retry=TRUE};
    DWORD result=win32_process_global_transaction_run(quit,stop,GetTickCount64()+1000,GetTickCount64()+1000,TRUE,&run_ops,&r);
    if(result || r.begins!=2 || r.advances!=2 || r.applies!=1)return 9;
    r=(run_state){.event=event,.alive=TRUE};SetEvent(quit);
    if(win32_process_global_transaction_run(quit,stop,GetTickCount64()+1000,GetTickCount64()+1000,TRUE,&run_ops,&r)!=ERROR_CANCELLED)return 10;
    ResetEvent(quit);ResetEvent(event);r=(run_state){.event=event,.alive=TRUE,.begins=1};
    if(win32_process_global_transaction_run(quit,stop,GetTickCount64(),GetTickCount64(),FALSE,&run_ops,&r)!=ERROR_TIMEOUT)return 11;
    r=(run_state){.event=event,.alive=TRUE,.begins=1,.empty=TRUE};
    if(win32_process_global_transaction_run(quit,stop,GetTickCount64()+1000,GetTickCount64(),FALSE,&run_ops,&r)!=ERROR_INVALID_STATE)return 12;
    SetEvent(event);r=(run_state){.event=event,.alive=TRUE,.begins=1,.stuck=TRUE};
    if(win32_process_global_transaction_run(quit,stop,GetTickCount64(),GetTickCount64(),FALSE,&run_ops,&r)!=ERROR_TIMEOUT)return 13;
    CloseHandle(event);CloseHandle(stop);CloseHandle(quit);
    puts("PROCESS_GLOBAL_TRANSACTION_PASS pending=1 active=1 failure=1 headless=1 run_retry=1 cancel=1 timeout=1 invalid=1");return 0;
}
