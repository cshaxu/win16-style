#include "ntwshk-dll/injection.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PAYLOAD FAIL %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    BYTE raw[65536]={0},copy[65536];ntwshk_packet h={0};ntwshk_context context;
    static const WCHAR directory[]=L"C:\\Package\\",module[]=L"example-xp";
    DWORD machine=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    h.version=2;h.machine=machine;h.flags=1;h.directory_offset=sizeof(h);
    h.directory_bytes=sizeof(directory);h.reserved1=sizeof(module);h.bytes=sizeof(h)+sizeof(directory)+sizeof(module);
    memcpy(raw,&h,sizeof(h));memcpy(raw+sizeof(h),directory,sizeof(directory));memcpy(raw+sizeof(h)+sizeof(directory),module,sizeof(module));
    CHECK(!ntwshk_packet_parse(raw,h.bytes,machine,&context));
    {WCHAR path[64];CHECK(!ntwshk_context_library(&context,IMAGE_FILE_MACHINE_I386,path,64));CHECK(!wcscmp(path,L"C:\\Package\\example-xp32.dll"));
    CHECK(!ntwshk_context_library(&context,IMAGE_FILE_MACHINE_AMD64,path,64));CHECK(!wcscmp(path,L"C:\\Package\\example-xp64.dll"));
    CHECK(ntwshk_context_library(&context,machine,path,3)==ERROR_FILENAME_EXCED_RANGE);}
    CHECK(ntwshk_packet_parse(raw,sizeof(h)-1,machine,&context)==ERROR_INVALID_DATA);
    CHECK(ntwshk_packet_parse(raw,65537,machine,&context)==ERROR_INVALID_DATA);
    for(unsigned i=0;i<8;++i){memcpy(copy,raw,h.bytes);((DWORD *)copy)[i]^=0x80000000u;CHECK(ntwshk_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);}
    memcpy(copy,raw,h.bytes);((DWORD *)copy)[3]=NTWSHK_POLICY_STYLE|NTWSHK_POLICY_GLOBAL_SESSION;
    CHECK(ntwshk_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    ((DWORD *)copy)[6]=123;CHECK(!ntwshk_packet_parse(copy,h.bytes,machine,&context));CHECK(context.session_cookie==123);
    memcpy(copy,raw,h.bytes);((WCHAR *)(copy+sizeof(h)+sizeof(directory)))[1]=L'\\';CHECK(ntwshk_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    memcpy(copy,raw,h.bytes);copy[h.bytes-2]=1;CHECK(ntwshk_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    puts("PAYLOAD PASS generic module, machine, bounds, policy, path, termination");return 0;
}
