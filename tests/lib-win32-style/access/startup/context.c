#include "lib-win32-style/access/startup/install.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PAYLOAD FAIL %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    BYTE raw[65536]={0},copy[65536];style_startup_packet h={0};style_startup_context context;
    static const WCHAR directory[]=L"C:\\Package\\",module[]=L"example-xp";
    DWORD machine=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    h.version=2;h.machine=machine;h.flags=1;h.directory_offset=sizeof(h);
    h.directory_bytes=sizeof(directory);h.reserved1=sizeof(module);h.bytes=sizeof(h)+sizeof(directory)+sizeof(module);
    memcpy(raw,&h,sizeof(h));memcpy(raw+sizeof(h),directory,sizeof(directory));memcpy(raw+sizeof(h)+sizeof(directory),module,sizeof(module));
    CHECK(!style_startup_packet_parse(raw,h.bytes,machine,&context));
    {WCHAR path[64];CHECK(!style_startup_library(&context,IMAGE_FILE_MACHINE_I386,path,64));CHECK(!wcscmp(path,L"C:\\Package\\example-xp32.dll"));
    CHECK(!style_startup_library(&context,IMAGE_FILE_MACHINE_AMD64,path,64));CHECK(!wcscmp(path,L"C:\\Package\\example-xp64.dll"));
    CHECK(style_startup_library(&context,machine,path,3)==ERROR_FILENAME_EXCED_RANGE);}
    CHECK(style_startup_packet_parse(raw,sizeof(h)-1,machine,&context)==ERROR_INVALID_DATA);
    CHECK(style_startup_packet_parse(raw,65537,machine,&context)==ERROR_INVALID_DATA);
    for(unsigned i=0;i<8;++i){memcpy(copy,raw,h.bytes);((DWORD *)copy)[i]^=0x80000000u;CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);}
    memcpy(copy,raw,h.bytes);((DWORD *)copy)[3]=STYLE_STARTUP_POLICY_STYLE|STYLE_STARTUP_POLICY_GLOBAL_SESSION;
    CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    ((DWORD *)copy)[6]=123;CHECK(!style_startup_packet_parse(copy,h.bytes,machine,&context));CHECK(context.session_cookie==123);
    memcpy(copy,raw,h.bytes);
    ((DWORD *)copy)[3]=STYLE_STARTUP_POLICY_STYLE|STYLE_STARTUP_POLICY_PROPAGATE|STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE;
    CHECK(!style_startup_packet_parse(copy,h.bytes,machine,&context));
    CHECK(context.flags==((DWORD *)copy)[3] && !context.session_cookie);
    ((DWORD *)copy)[1]=3;
    ((DWORD *)copy)[3]|=9u<<STYLE_STARTUP_PROFILE_SHIFT;
    CHECK(!style_startup_packet_parse(copy,h.bytes,machine,&context));
    ((DWORD *)copy)[3]^=STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE;
    CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    memcpy(copy,raw,h.bytes);((WCHAR *)(copy+sizeof(h)+sizeof(directory)))[1]=L'\\';CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    memcpy(copy,raw,h.bytes);copy[h.bytes-2]=1;CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    style_startup_selection selection={14u,1u};
    h.version=STYLE_STARTUP_PACKET_VERSION;
    h.directory_offset=sizeof(h)+sizeof(selection);
    h.bytes=h.directory_offset+sizeof(directory)+sizeof(module);
    memcpy(copy,&h,sizeof(h));memcpy(copy+sizeof(h),&selection,sizeof(selection));
    memcpy(copy+h.directory_offset,raw+sizeof(h),sizeof(directory)+sizeof(module));
    CHECK(!style_startup_packet_parse(copy,h.bytes,machine,&context));
    CHECK(context.flags==(h.flags|(14u<<STYLE_STARTUP_PROFILE_SHIFT)|STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE));
    ((DWORD *)copy)[9]=0;
    CHECK(!style_startup_packet_parse(copy,h.bytes,machine,&context));
    CHECK(context.flags==(h.flags|(14u<<STYLE_STARTUP_PROFILE_SHIFT)));
    ((DWORD *)copy)[9]=2;
    CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    ((DWORD *)copy)[9]=0;((DWORD *)copy)[8]=UINT32_MAX;
    CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    ((DWORD *)copy)[8]=14;((DWORD *)copy)[3]|=STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE;
    CHECK(style_startup_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    CHECK(style_startup_packet_parse(copy,sizeof(h)+sizeof(selection)-1,machine,&context)==ERROR_INVALID_DATA);
    puts("PAYLOAD PASS generic module, machine, bounds, policy, path, termination");return 0;
}
