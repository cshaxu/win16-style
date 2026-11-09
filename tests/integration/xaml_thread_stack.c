#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <stdlib.h>

int wmain(int argc,wchar_t **argv)
{
    if (argc!=3) return 2;
    DWORD pid=wcstoul(argv[1],NULL,10),tid=wcstoul(argv[2],NULL,10);
    if (!pid || !tid) return 2;
    HANDLE process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pid);
    HANDLE thread=OpenThread(THREAD_QUERY_INFORMATION|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME,FALSE,tid);
    if (!process || !thread || GetProcessIdOfThread(thread)!=pid) {
        printf("open_error=%lu\n",(unsigned long)GetLastError());
        if (thread) CloseHandle(thread);
        if (process) CloseHandle(process);
        return 3;
    }
    CONTEXT context={0}; context.ContextFlags=CONTEXT_FULL;
    if (SuspendThread(thread)==(DWORD)-1) { CloseHandle(thread); CloseHandle(process); return 3; }
    BOOL captured=GetThreadContext(thread,&context);
    DWORD resume=ResumeThread(thread);
    if (!captured || resume==(DWORD)-1) { printf("context_or_resume_failed\n"); CloseHandle(thread); CloseHandle(process); return 3; }
    SymSetOptions(SYMOPT_DEFERRED_LOADS|SYMOPT_FAIL_CRITICAL_ERRORS|SYMOPT_NO_PROMPTS);
    if (!SymInitialize(process,"",TRUE)) { printf("symbols_error=%lu\n",(unsigned long)GetLastError()); CloseHandle(thread); CloseHandle(process); return 3; }
    STACKFRAME64 frame={0}; DWORD machine;
#if defined(_WIN64)
    machine=IMAGE_FILE_MACHINE_AMD64; frame.AddrPC.Offset=context.Rip; frame.AddrFrame.Offset=context.Rbp; frame.AddrStack.Offset=context.Rsp;
#else
    machine=IMAGE_FILE_MACHINE_I386; frame.AddrPC.Offset=context.Eip; frame.AddrFrame.Offset=context.Ebp; frame.AddrStack.Offset=context.Esp;
#endif
    frame.AddrPC.Mode=AddrModeFlat; frame.AddrFrame.Mode=AddrModeFlat; frame.AddrStack.Mode=AddrModeFlat;
    for (unsigned int i=0;i<16 && frame.AddrPC.Offset;i++) {
        IMAGEHLP_MODULE64 module={.SizeOfStruct=sizeof(module)};
        char storage[sizeof(SYMBOL_INFO)+256]={0}; SYMBOL_INFO *symbol=(SYMBOL_INFO *)storage;
        symbol->SizeOfStruct=sizeof(*symbol); symbol->MaxNameLen=255; DWORD64 displacement=0;
        BOOL have_module=SymGetModuleInfo64(process,frame.AddrPC.Offset,&module);
        BOOL have_symbol=SymFromAddr(process,frame.AddrPC.Offset,&displacement,symbol);
        printf("frame=%u module=%s rva=%llx symbol=%s offset=%llu\n",i,have_module?module.ModuleName:"?",
               (unsigned long long)(have_module ? frame.AddrPC.Offset-module.BaseOfImage : 0),
               have_symbol?symbol->Name:"?",(unsigned long long)displacement);
        if (!StackWalk64(machine,process,thread,&frame,&context,NULL,SymFunctionTableAccess64,SymGetModuleBase64,NULL)) break;
    }
    SymCleanup(process); CloseHandle(thread); CloseHandle(process); return 0;
}
