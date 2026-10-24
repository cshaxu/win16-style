#include "ntw16s-dll/injection/injection.h"
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STAGING FAIL %d error=%lu\n",__LINE__,GetLastError());goto failed;}}while(0)
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
static int initialized_child(BOOL legacy)
{
    if(legacy){
        BYTE *base=(BYTE *)GetModuleHandleW(NULL);
        IMAGE_DOS_HEADER *dos=(IMAGE_DOS_HEADER *)base;
        IMAGE_NT_HEADERS *nt=(IMAGE_NT_HEADERS *)(base+dos->e_lfanew);
        IMAGE_DATA_DIRECTORY iat=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT];
        if(iat.VirtualAddress || iat.Size ||
           nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress>=nt->OptionalHeader.SizeOfImage)return 45;
    }
    HMODULE dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    typedef BOOL (__cdecl *attached_fn)(HWND);
    attached_fn attached=dll?(attached_fn)(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
    WNDCLASSW cls={.lpfnWndProc=owner,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"ExternalImportStaging"};
    if(!attached || !RegisterClassW(&cls))return 43;
    HWND w=CreateWindowW(cls.lpszClassName,L"Initialized relocated imports",WS_OVERLAPPEDWINDOW,
        0,0,320,200,NULL,NULL,cls.hInstance,NULL);
    BOOL ok=w && attached(w);
    if(w)DestroyWindow(w);
    return ok?42:44;
}
static BYTE *main_image(HANDLE process,IMAGE_DOS_HEADER *dos,IMAGE_NT_HEADERS *nt)
{
    uintptr_t cursor=0;MEMORY_BASIC_INFORMATION region;SIZE_T got;
    for(unsigned i=0;i<65536 && VirtualQueryEx(process,(void *)cursor,&region,sizeof(region));++i){
        if(region.Type==MEM_IMAGE && region.BaseAddress==region.AllocationBase &&
            ReadProcessMemory(process,region.AllocationBase,dos,sizeof(*dos),&got) && got==sizeof(*dos) &&
            dos->e_magic==IMAGE_DOS_SIGNATURE && dos->e_lfanew>=64 && dos->e_lfanew<1024*1024 &&
            ReadProcessMemory(process,(BYTE *)region.AllocationBase+dos->e_lfanew,nt,sizeof(*nt),&got) &&
            got==sizeof(*nt) && nt->Signature==IMAGE_NT_SIGNATURE && !(nt->FileHeader.Characteristics&IMAGE_FILE_DLL))
            return region.AllocationBase;
        uintptr_t next=(uintptr_t)region.BaseAddress+region.RegionSize;
        if(next<=cursor)break;
        cursor=next;
    }
    return NULL;
}
/* Self-contained equivalent of an earlier startup installer's relocated import
 * descriptors. No Detours, broker, installed VDM or sibling binary is needed. */
static BOOL external_case(const WCHAR *self,ntw16s_context *context,unsigned mode)
{
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    WCHAR command[NTW16S_PATH_CAP+4];IMAGE_DOS_HEADER dos;IMAGE_NT_HEADERS nt;
    IMAGE_IMPORT_DESCRIPTOR descriptors[1025]={0};SIZE_T got;BOOL result=FALSE;
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\"",self)>0);
    CHECK(CreateProcessW(self,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&child));
    BYTE *base=main_image(child.hProcess,&dos,&nt);CHECK(base);
    DWORD rva=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    unsigned count=0;
    for(;count<1024;++count){
        CHECK(ReadProcessMemory(child.hProcess,base+rva+count*sizeof(*descriptors),&descriptors[count],sizeof(*descriptors),&got));
        CHECK(got==sizeof(*descriptors));
        if(!descriptors[count].Name && !descriptors[count].FirstThunk && !descriptors[count].OriginalFirstThunk)break;
    }
    CHECK(count>0 && count<1024);
    if(mode==2)for(unsigned i=1;i<1025;++i)descriptors[i]=descriptors[0];
    SIZE_T bytes=mode==2?sizeof(descriptors):(count+1)*sizeof(*descriptors);
    uintptr_t at=((uintptr_t)base+nt.OptionalHeader.SizeOfImage+0xffffu)&~(uintptr_t)0xffffu;
    void *remote=NULL;
    for(unsigned i=0;i<256 && !remote;++i,at+=0x10000u){
        if(at<(uintptr_t)base || at-(uintptr_t)base>UINT32_MAX-bytes)break;
        remote=VirtualAllocEx(child.hProcess,(void *)at,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    }
    CHECK(remote && (uintptr_t)remote-(uintptr_t)base>=nt.OptionalHeader.SizeOfImage);
    CHECK(WriteProcessMemory(child.hProcess,remote,descriptors,bytes,&got) && got==bytes);
    nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress=
        mode==3?UINT32_MAX-7u:(DWORD)((uintptr_t)remote-(uintptr_t)base);
    nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size=(DWORD)bytes;
    DWORD protect,unused;
    CHECK(VirtualProtectEx(child.hProcess,base+dos.e_lfanew,sizeof(nt),PAGE_READWRITE,&protect));
    CHECK(WriteProcessMemory(child.hProcess,base+dos.e_lfanew,&nt,sizeof(nt),&got) && got==sizeof(nt));
    CHECK(VirtualProtectEx(child.hProcess,base+dos.e_lfanew,sizeof(nt),protect,&unused));
    if(mode==1)CHECK(VirtualProtectEx(child.hProcess,remote,bytes,PAGE_NOACCESS,&unused));
    DWORD error=ntw16s_install(child.hProcess,context,CreateProcessW);
    if(mode){
        CHECK(error!=ERROR_SUCCESS);
        if(mode>=2)CHECK(error==ERROR_BAD_EXE_FORMAT);
        CHECK(WaitForSingleObject(child.hProcess,0)==WAIT_TIMEOUT);
    }else{
        if(error)fprintf(stderr,"EXTERNAL IMPORT INSTALL error=%lu\n",error);
        CHECK(!error);
        CHECK(SuspendThread(child.hThread)==1 && ResumeThread(child.hThread)==2);
        CHECK(ResumeThread(child.hThread)==1);
        CHECK(WaitForSingleObject(child.hProcess,10000)==WAIT_OBJECT_0);
        DWORD code;CHECK(GetExitCodeProcess(child.hProcess,&code) && code==42);
    }
    printf("STAGING EXTERNAL PASS bits=%u mode=%u result=%lu initialized=%u\n",(unsigned)(sizeof(void *)*8),mode,error,!mode);
    result=TRUE;
failed:
    if(child.hProcess){TerminateProcess(child.hProcess,1);WaitForSingleObject(child.hProcess,2000);CloseHandle(child.hProcess);}
    if(child.hThread)CloseHandle(child.hThread);
    return result;
}
/* DoubleKiller omits the IAT directory as well as OriginalFirstThunk. Isolate
 * the missing directory from the zero-OFT shape in two private image copies.
 * No installed target or fixture file is modified. */
static BOOL legacy_iat_case(const WCHAR *self,ntw16s_context *context,unsigned mode)
{
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    WCHAR command[NTW16S_PATH_CAP+4];IMAGE_DOS_HEADER dos;IMAGE_NT_HEADERS nt;
    IMAGE_IMPORT_DESCRIPTOR descriptors[1025]={0};SIZE_T got;BOOL result=FALSE;DWORD protect,unused;
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --legacy-child",self)>0);
    CHECK(CreateProcessW(self,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&child));
    BYTE *base=main_image(child.hProcess,&dos,&nt);CHECK(base);
    DWORD rva=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;unsigned count=0;
    for(;count<1024;++count){
        CHECK(ReadProcessMemory(child.hProcess,base+rva+count*sizeof(*descriptors),&descriptors[count],sizeof(*descriptors),&got));
        CHECK(got==sizeof(*descriptors));
        if(!descriptors[count].Name && !descriptors[count].FirstThunk && !descriptors[count].OriginalFirstThunk)break;
        if(mode==1)descriptors[count].OriginalFirstThunk=0;
    }
    CHECK(count>0 && count<1024);
    if(mode==2)descriptors[0].FirstThunk=nt.OptionalHeader.SizeOfImage-1;
    if(mode==3){
        uintptr_t unterminated[1024];for(unsigned i=0;i<ARRAYSIZE(unterminated);++i)unterminated[i]=1;
        CHECK(nt.OptionalHeader.SizeOfImage>0x1000+sizeof(unterminated));
        descriptors[0].FirstThunk=0x1000;
        CHECK(VirtualProtectEx(child.hProcess,base+0x1000,sizeof(unterminated),PAGE_READWRITE,&protect));
        CHECK(WriteProcessMemory(child.hProcess,base+0x1000,unterminated,sizeof(unterminated),&got) && got==sizeof(unterminated));
        CHECK(VirtualProtectEx(child.hProcess,base+0x1000,sizeof(unterminated),protect,&unused));
    }
    SIZE_T bytes=(count+1)*sizeof(*descriptors);
    CHECK(VirtualProtectEx(child.hProcess,base+rva,bytes,PAGE_READWRITE,&protect));
    CHECK(WriteProcessMemory(child.hProcess,base+rva,descriptors,bytes,&got) && got==bytes);
    CHECK(VirtualProtectEx(child.hProcess,base+rva,bytes,protect,&unused));
    nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT]=(IMAGE_DATA_DIRECTORY){0};
    CHECK(VirtualProtectEx(child.hProcess,base+dos.e_lfanew,sizeof(nt),PAGE_READWRITE,&protect));
    CHECK(WriteProcessMemory(child.hProcess,base+dos.e_lfanew,&nt,sizeof(nt),&got) && got==sizeof(nt));
    CHECK(VirtualProtectEx(child.hProcess,base+dos.e_lfanew,sizeof(nt),protect,&unused));
    DWORD install_error=ntw16s_install(child.hProcess,context,CreateProcessW);
    if(mode>=2){
        CHECK(install_error==ERROR_BAD_EXE_FORMAT);
        IMAGE_NT_HEADERS after;
        CHECK(ReadProcessMemory(child.hProcess,base+dos.e_lfanew,&after,sizeof(after),&got) && got==sizeof(after));
        CHECK(!memcmp(&nt,&after,sizeof(nt)) && WaitForSingleObject(child.hProcess,0)==WAIT_TIMEOUT);
        CHECK(SuspendThread(child.hThread)==1 && ResumeThread(child.hThread)==2);
        printf("LEGACY IAT REJECT PASS bits=%u mode=%u error=%lu headers_unchanged=1 suspended=1\n",(unsigned)(sizeof(void *)*8),mode,install_error);
        result=TRUE;goto failed;
    }
    if(install_error)fprintf(stderr,"LEGACY STAGING install error=0x%08lx\n",install_error);
    CHECK(!install_error);
    CHECK(ResumeThread(child.hThread)==1 && WaitForSingleObject(child.hProcess,10000)==WAIT_OBJECT_0);
    DWORD code;CHECK(GetExitCodeProcess(child.hProcess,&code));
    if(code!=42)fprintf(stderr,"LEGACY STAGING child exit=0x%08lx\n",code);
    CHECK(code==42);
    printf("LEGACY IAT PASS bits=%u zero_oft=%u initialized=1 metadata_restored=1\n",(unsigned)(sizeof(void *)*8),mode);result=TRUE;
failed:
    if(child.hProcess){TerminateProcess(child.hProcess,1);WaitForSingleObject(child.hProcess,2000);CloseHandle(child.hProcess);}
    if(child.hThread)CloseHandle(child.hThread);
    return result;
}
int wmain(int argc,WCHAR **argv)
{
    WCHAR self[NTW16S_PATH_CAP],command[NTW16S_PATH_CAP+4];ntw16s_context context;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};WORD machine;
    DWORD expected=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;int result=1;
    if(argc==1)return initialized_child(FALSE);
    if(argc==2 && !wcscmp(argv[1],L"--legacy-child"))return initialized_child(TRUE);
    CHECK((argc==2 || argc==3) && !wcscmp(argv[1],L"--parent"));
    CHECK(!ntw16s_context_paths(&context,NULL));
    if(argc==3)CHECK(!wcscpy_s(context.directory,NTW16S_PATH_CAP,argv[2]));
    CHECK(GetModuleFileNameW(NULL,self,NTW16S_PATH_CAP));
    CHECK(!ntw16s_validate_image(self,(WORD)expected,FALSE));
    CHECK(ntw16s_validate_image(self,(WORD)expected,TRUE)==ERROR_BAD_EXE_FORMAT);
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\"",self)>0);
    CHECK(CreateProcessW(self,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&child));
    CHECK(!ntw16s_process_machine(child.hProcess,&machine) && machine==expected);
    CHECK(SuspendThread(child.hThread)==1 && ResumeThread(child.hThread)==2);
    context.flags=0x80000001u;
    CHECK(ntw16s_install(child.hProcess,&context,CreateProcessW)==ERROR_INVALID_DATA);
    context.flags=1;
    CHECK(!ntw16s_install(child.hProcess,&context,CreateProcessW));
    CHECK(SuspendThread(child.hThread)==1 && ResumeThread(child.hThread)==2);
    CHECK(WaitForSingleObject(child.hProcess,0)==WAIT_TIMEOUT);
    puts("STAGING PASS actual same-width target, payload/imports staged, original suspension retained; initialization not claimed");result=0;
failed:
    if(child.hProcess){TerminateProcess(child.hProcess,1);WaitForSingleObject(child.hProcess,2000);CloseHandle(child.hProcess);}
    if(child.hThread)CloseHandle(child.hThread);
    if(!result)for(unsigned mode=0;mode<4;++mode)if(!external_case(self,&context,mode))return 1;
    if(!result)for(unsigned mode=0;mode<4;++mode)if(!legacy_iat_case(self,&context,mode))return 1;
    return result;
}
