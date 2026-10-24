#ifndef CMD_SUPPORT_H
#define CMD_SUPPORT_H
#include <windows.h>
#include <tlhelp32.h>
#include <bcrypt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static BOOL cmd_hash(PCWSTR path,char out[65])
{
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    BCRYPT_ALG_HANDLE algorithm=NULL;BCRYPT_HASH_HANDLE hash=NULL;BYTE bytes[16384],digest[32];DWORD got;
    BOOL ok=FALSE;
    if(file==INVALID_HANDLE_VALUE)return FALSE;
    if(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,NULL,0)<0 ||
       BCryptCreateHash(algorithm,&hash,NULL,0,NULL,0,0)<0)goto done;
    do{if(!ReadFile(file,bytes,sizeof(bytes),&got,NULL))goto done;
        if(got && BCryptHashData(hash,bytes,got,0)<0)goto done;
    }while(got);
    if(BCryptFinishHash(hash,digest,sizeof(digest),0)<0)goto done;
    for(unsigned i=0;i<32;++i)sprintf(out+i*2,"%02x",digest[i]);
    out[64]=0;ok=TRUE;
done:
    if(hash)BCryptDestroyHash(hash);
    if(algorithm)BCryptCloseAlgorithmProvider(algorithm,0);
    CloseHandle(file);return ok;
}
static WORD cmd_machine(PCWSTR path)
{
    IMAGE_DOS_HEADER dos;DWORD read,signature;IMAGE_FILE_HEADER header;LARGE_INTEGER at;
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    WORD result=0;
    if(file==INVALID_HANDLE_VALUE)return 0;
    if(!ReadFile(file,&dos,sizeof(dos),&read,NULL)||read!=sizeof(dos)||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<64)goto done;
    at.QuadPart=dos.e_lfanew;
    if(!SetFilePointerEx(file,at,NULL,FILE_BEGIN)||!ReadFile(file,&signature,4,&read,NULL)||read!=4||signature!=IMAGE_NT_SIGNATURE)goto done;
    if(ReadFile(file,&header,sizeof(header),&read,NULL)&&read==sizeof(header))result=header.Machine;
done:CloseHandle(file);return result;
}
/* Inspect this package's own COFF symbols instead of guessing private addresses
 * or changing the product ABI for a test-only status endpoint. */
static DWORD cmd_symbol(PCWSTR path,const char *wanted)
{
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    BYTE *bytes=NULL;DWORD size,got,result=0;
    if(file==INVALID_HANDLE_VALUE)return 0;
    size=GetFileSize(file,NULL);
    if(size==INVALID_FILE_SIZE||size>16*1024*1024)goto done;
    bytes=malloc(size);if(!bytes||!ReadFile(file,bytes,size,&got,NULL)||got!=size||size<sizeof(IMAGE_DOS_HEADER))goto done;
    IMAGE_DOS_HEADER dos;memcpy(&dos,bytes,sizeof(dos));
    if(dos.e_lfanew<64||(DWORD)dos.e_lfanew>size-24)goto done;
    IMAGE_FILE_HEADER header;memcpy(&header,bytes+dos.e_lfanew+4,sizeof(header));
    uint64_t symbols=header.PointerToSymbolTable,strings=symbols+(uint64_t)header.NumberOfSymbols*IMAGE_SIZEOF_SYMBOL;
    uint64_t sections=(DWORD)dos.e_lfanew+24u+header.SizeOfOptionalHeader;
    if(strings+4>size||sections+(uint64_t)header.NumberOfSections*IMAGE_SIZEOF_SECTION_HEADER>size)goto done;
    for(DWORD i=0;i<header.NumberOfSymbols;){
        IMAGE_SYMBOL symbol;memcpy(&symbol,bytes+symbols+(uint64_t)i*IMAGE_SIZEOF_SYMBOL,sizeof(symbol));
        char shortname[9]={0};const char *name;
        if(symbol.N.Name.Short){memcpy(shortname,symbol.N.ShortName,8);name=shortname;}
        else{DWORD offset=symbol.N.Name.Long;if(strings+offset>=size)goto done;name=(char *)bytes+strings+offset;
            if(!memchr(name,0,size-(SIZE_T)(strings+offset)))goto done;}
        if(*name=='_')++name;
        if(!strcmp(name,wanted)&&symbol.SectionNumber>0&&symbol.SectionNumber<=header.NumberOfSections){
            IMAGE_SECTION_HEADER section;memcpy(&section,bytes+sections+(symbol.SectionNumber-1u)*IMAGE_SIZEOF_SECTION_HEADER,sizeof(section));
            result=section.VirtualAddress+symbol.Value;break;
        }
        i+=1u+symbol.NumberOfAuxSymbols;
    }
done:free(bytes);CloseHandle(file);return result;
}
typedef struct cmd_process {
    DWORD pid,parent;HANDLE handle;WORD machine;WCHAR path[32768];
    BOOL is_cmd,ntw,nthook,ntw_initialized;DWORD exit_code;
} cmd_process;
typedef struct cmd_inventory {cmd_process processes[48];unsigned count;HANDLE job;FILE *report;PCWSTR package;} cmd_inventory;
static BOOL cmd_snapshot(cmd_inventory *inventory)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);PROCESSENTRY32W e={.dwSize=sizeof(e)};
    if(snapshot==INVALID_HANDLE_VALUE)return FALSE;
    if(Process32FirstW(snapshot,&e))do{
        BOOL seen=FALSE;for(unsigned i=0;i<inventory->count;++i)if(inventory->processes[i].pid==e.th32ProcessID)seen=TRUE;
        if(seen)continue;
        HANDLE process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,e.th32ProcessID);
        if(!process)continue;
        BOOL owned=FALSE;IsProcessInJob(process,inventory->job,&owned);
        if(!owned){CloseHandle(process);continue;}
        if(inventory->count==48){CloseHandle(process);CloseHandle(snapshot);return FALSE;}
        cmd_process *p=&inventory->processes[inventory->count++];p->pid=e.th32ProcessID;p->parent=e.th32ParentProcessID;p->handle=process;
        DWORD capacity=ARRAYSIZE(p->path);
        if(!QueryFullProcessImageNameW(process,0,p->path,&capacity)){CloseHandle(snapshot);return FALSE;}
        p->machine=cmd_machine(p->path);const WCHAR *name=wcsrchr(p->path,L'\\');name=name?name+1:p->path;
        p->is_cmd=!_wcsicmp(name,L"cmd.exe");char hash[65]="unavailable";cmd_hash(p->path,hash);
        fprintf(inventory->report,"PROCESS pid=%lu creator=%lu image=%ls machine=%04x sha256=%s\n",p->pid,p->parent,p->path,p->machine,hash);
    }while(Process32NextW(snapshot,&e));
    CloseHandle(snapshot);
    for(unsigned i=0;i<inventory->count;++i){cmd_process *p=&inventory->processes[i];
        if(WaitForSingleObject(p->handle,0)==WAIT_OBJECT_0){GetExitCodeProcess(p->handle,&p->exit_code);continue;}
        HANDLE modules=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,p->pid);MODULEENTRY32W m={.dwSize=sizeof(m)};
        /* A newly created WOW64 process has no enumerable 32-bit loader list
         * yet. Retry under the caller's existing deadline; a marker cannot
         * pass the later initialized-module assertions on its own. */
        if(modules==INVALID_HANDLE_VALUE)continue;
        if(Module32FirstW(modules,&m))do{
            BOOL ntw=!_wcsicmp(m.szModule,L"ntw16s32.dll")||!_wcsicmp(m.szModule,L"ntw16s64.dll");
            BOOL nthook=!_wcsicmp(m.szModule,L"nthook32.dll")||!_wcsicmp(m.szModule,L"nthook64.dll");
            if(!ntw&&!nthook)continue;
            if((ntw&&p->ntw&&p->ntw_initialized)||(nthook&&p->nthook))continue;
            char hash[65];if(!cmd_hash(m.szExePath,hash)){CloseHandle(modules);return FALSE;}
            if(ntw){WCHAR expected[32768];char wanted[65];
                if(swprintf_s(expected,ARRAYSIZE(expected),L"%ls\\ntw16s%u.dll",inventory->package,p->machine==IMAGE_FILE_MACHINE_AMD64?64u:32u)<0 ||
                   !cmd_hash(expected,wanted)||strcmp(hash,wanted)){CloseHandle(modules);return FALSE;}}
            fprintf(inventory->report,"DLL pid=%lu path=%ls sha256=%s\n",p->pid,m.szExePath,hash);
            if(nthook)p->nthook=TRUE;
            if(ntw){DWORD committed=0,count=0,flags=0;SIZE_T got;
                DWORD a=cmd_symbol(m.szExePath,"committed"),b=cmd_symbol(m.szExePath,"count"),c=cmd_symbol(m.szExePath,"ntw16s_process_context");
                if(!a||!b||!c||!ReadProcessMemory(p->handle,m.modBaseAddr+a,&committed,4,&got)||
                   !ReadProcessMemory(p->handle,m.modBaseAddr+b,&count,4,&got)||!ReadProcessMemory(p->handle,m.modBaseAddr+c,&flags,4,&got)){
                    DWORD error=GetLastError();
                    /* A short-lived owned helper can exit between the module
                     * snapshot and RPM. That is not a failed cmd launch. Keep
                     * its native exit status, without inventing initialization. */
                    if(WaitForSingleObject(p->handle,0)==WAIT_OBJECT_0){
                        GetExitCodeProcess(p->handle,&p->exit_code);
                        fprintf(inventory->report,"OBSERVE exited during state read pid=%lu read_error=%lu exit=%lu initialized_unconfirmed=1\n",p->pid,error,p->exit_code);
                        continue;
                    }
                    fprintf(inventory->report,"OBSERVE live state read failed pid=%lu win32=%lu\n",p->pid,error);
                    CloseHandle(modules);return FALSE;
                }
                p->ntw=TRUE;p->ntw_initialized=committed==1&&count>=2&&(flags&3)==3;
                fprintf(inventory->report,"NTW STATE pid=%lu loaded=1 policy=%lu committed=%lu hooks=%lu propagation_initialized=%d\n",p->pid,flags,committed,count,p->ntw_initialized);
            }
        }while(Module32NextW(modules,&m));
        CloseHandle(modules);
    }
    fflush(inventory->report);return TRUE;
}
static void cmd_inventory_close(cmd_inventory *inventory)
{for(unsigned i=0;i<inventory->count;++i)CloseHandle(inventory->processes[i].handle);}
#endif
