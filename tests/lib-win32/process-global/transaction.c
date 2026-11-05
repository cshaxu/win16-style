#include "lib-win32/process-global/transaction.h"
#include <stdio.h>
typedef struct test_state {unsigned calls[5];DWORD load,validate,start,ready,apply;} test_state;
static DWORD load(void *p){test_state *s=p;++s->calls[0];return s->load;}
static DWORD validate(void *p){test_state *s=p;++s->calls[1];return s->validate;}
static DWORD start(void *p){test_state *s=p;++s->calls[2];return s->start;}
static DWORD ready(void *p){test_state *s=p;++s->calls[3];return s->ready;}
static DWORD apply(void *p){test_state *s=p;++s->calls[4];return s->apply;}
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
    puts("PROCESS_GLOBAL_TRANSACTION_PASS pending=1 active=1 failure=1 headless=1");return 0;
}
