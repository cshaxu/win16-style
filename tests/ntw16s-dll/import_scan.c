#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ntw16s-dll/import_scan.h"

#define CHECK(x) do{if(!(x)){printf("IMPORT_SCAN FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static const ntw16s_import_query query[]={{"USER32.dll","DrawTextA"}};

static void fixture(BYTE *image,size_t size)
{
    ZeroMemory(image,size);
    IMAGE_DOS_HEADER *dos=(void *)image;dos->e_magic=IMAGE_DOS_SIGNATURE;dos->e_lfanew=0x80;
    IMAGE_NT_HEADERS *nt=(void *)(image+0x80);nt->Signature=IMAGE_NT_SIGNATURE;
    nt->FileHeader.Machine=sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    nt->FileHeader.SizeOfOptionalHeader=sizeof(nt->OptionalHeader);
    nt->OptionalHeader.Magic=sizeof(void*)==8?IMAGE_NT_OPTIONAL_HDR64_MAGIC:IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    nt->OptionalHeader.SizeOfImage=(DWORD)size;nt->OptionalHeader.NumberOfRvaAndSizes=IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress=0x200;
    nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size=2*sizeof(IMAGE_IMPORT_DESCRIPTOR);
    IMAGE_IMPORT_DESCRIPTOR *d=(void *)(image+0x200);d->Name=0x280;d->OriginalFirstThunk=0x300;d->FirstThunk=0x380;
    memcpy(image+0x280,"USER32.dll",11);
    IMAGE_THUNK_DATA *names=(void *)(image+0x300);names[0].u1.AddressOfData=0x400;
    memcpy(image+0x402,"DrawTextA",10);
    void **slots=(void **)(image+0x380);slots[0]=(void *)(UINT_PTR)0x12345678;
}

int wmain(int argc,WCHAR **argv)
{
    BYTE *image=VirtualAlloc(NULL,8192,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);CHECK(image);
    void **slot=(void **)1;fixture(image,8192);
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_SUCCESS);
    CHECK(slot==(void **)(image+0x380) && *slot==(void *)(UINT_PTR)0x12345678);
    IMAGE_NT_HEADERS *nt=(void *)(image+0x80);IMAGE_IMPORT_DESCRIPTOR *d=(void *)(image+0x200);
    const ntw16s_import_query two[]={{"USER32.dll","DrawTextA"},{"USER32.dll","Missing"}};
    void **pair[2]={(void **)1,(void **)1};
    CHECK(ntw16s_import_slots((HMODULE)image,two,2,pair)==ERROR_PROC_NOT_FOUND && !pair[0] && !pair[1]);
    ((IMAGE_DOS_HEADER *)image)->e_magic=0;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    fixture(image,8192);
    d->Name=8191;image[8191]='x';slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    fixture(image,8192);d->FirstThunk=8191;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    fixture(image,8192);nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size=sizeof(*d);slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    fixture(image,8192);((IMAGE_THUNK_DATA *)(image+0x300))[0].u1.AddressOfData=8191;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    fixture(image,8192);memcpy(image+0x280,"GDI32.dll",10);slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_PROC_NOT_FOUND && !slot);
    fixture(image,8192);((IMAGE_THUNK_DATA *)(image+0x300))[0].u1.Ordinal=IMAGE_ORDINAL_FLAG|17;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_PROC_NOT_FOUND && !slot);
    fixture(image,8192);((IMAGE_THUNK_DATA *)(image+0x300))[1].u1.AddressOfData=0x400;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    fixture(image,8192);CHECK(VirtualFree(image+4096,4096,MEM_DECOMMIT));
    d->FirstThunk=4096;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress=4090;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)image,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    CHECK(VirtualFree(image,0,MEM_RELEASE));
    BYTE *address=VirtualAlloc(NULL,131072,MEM_RESERVE,PAGE_NOACCESS);CHECK(address);
    CHECK(VirtualFree(address,0,MEM_RELEASE));
    BYTE *first=VirtualAlloc(address,65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);CHECK(first==address);
    BYTE *second=VirtualAlloc(address+65536,65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);CHECK(second==address+65536);
    fixture(first,65536);nt=(void *)(first+0x80);d=(void *)(first+0x200);
    nt->OptionalHeader.SizeOfImage=131072;d->FirstThunk=65536;slot=(void **)1;
    CHECK(ntw16s_import_slots((HMODULE)first,query,1,&slot)==ERROR_BAD_EXE_FORMAT && !slot);
    CHECK(VirtualFree(second,0,MEM_RELEASE) && VirtualFree(first,0,MEM_RELEASE));
    const ntw16s_import_query live[]={{"KERNEL32.dll","GetCurrentProcessId"}};slot=NULL;
    CHECK(GetCurrentProcessId()!=0);
    CHECK(ntw16s_import_slots(GetModuleHandleW(NULL),live,1,&slot)==ERROR_SUCCESS && slot && *slot);
    printf("IMPORT_SCAN PASS bits=%u synthetic=13 live=1\n",(unsigned)(sizeof(void*)*8));
    if(argc==2 && !lstrcmpW(argv[1],L"--vb6")){
        const ntw16s_import_query vb[]={{"USER32.dll","DrawTextA"},{"USER32.dll","DrawTextExA"},
            {"GDI32.dll","TextOutA"},{"GDI32.dll","ExtTextOutA"}};void **vbslots[4]={0};
        HMODULE runtime=LoadLibraryExW(L"MSVBVM60.DLL",NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!runtime){printf("IMPORT_SCAN VB6 SKIP error=%lu\n",GetLastError());return 77;}
        ULONGLONG start=GetTickCount64();DWORD error=ntw16s_import_slots(runtime,vb,ARRAYSIZE(vb),vbslots);
        CHECK(error==ERROR_SUCCESS);
        for(size_t i=0;i<ARRAYSIZE(vb);++i){HMODULE lib=GetModuleHandleA(vb[i].library);
            MEMORY_BASIC_INFORMATION region;
            CHECK(lib && vbslots[i] && *vbslots[i]==(void *)GetProcAddress(lib,vb[i].symbol));
            CHECK(VirtualQuery(vbslots[i],&region,sizeof(region))==sizeof(region));
            printf("IMPORT_SCAN VB6 SLOT name=%s protect=%08lx\n",vb[i].symbol,region.Protect);}
        printf("IMPORT_SCAN VB6 PASS slots=4 elapsed_ms=%llu\n",(unsigned long long)(GetTickCount64()-start));
        FreeLibrary(runtime);
    }else if(argc!=1)return 2;
    return 0;
}
