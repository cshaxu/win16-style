#include "ntw16s-dll/injection/injection.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PAYLOAD FAIL %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    BYTE raw[65536]={0},copy[65536];ntw16s_packet h={0};ntw16s_context context;
    static const WCHAR directory[]=L"C:\\Package\\";
    DWORD machine=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    h.bytes=sizeof(h)+sizeof(directory);h.version=1;h.machine=machine;h.flags=1;
    h.directory_offset=sizeof(h);h.directory_bytes=sizeof(directory);
    memcpy(raw,&h,sizeof(h));memcpy(raw+sizeof(h),directory,sizeof(directory));
    CHECK(!ntw16s_packet_parse(raw,h.bytes,machine,&context));CHECK(context.flags==1);
    CHECK(ntw16s_packet_parse(raw,sizeof(h)-1,machine,&context)==ERROR_INVALID_DATA);
    CHECK(ntw16s_packet_parse(raw,65537,machine,&context)==ERROR_INVALID_DATA);
    for(unsigned i=0;i<8;++i){
        memcpy(copy,raw,h.bytes);((DWORD *)copy)[i]^=0x80000000u;
        CHECK(ntw16s_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    }
    memcpy(copy,raw,h.bytes);copy[h.bytes-2]=1;
    CHECK(ntw16s_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    memcpy(copy,raw,h.bytes);copy[sizeof(h)]=0;
    CHECK(ntw16s_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    memcpy(copy,raw,h.bytes);((WCHAR *)(copy+sizeof(h)))[1]=L'_';
    CHECK(ntw16s_packet_parse(copy,h.bytes,machine,&context)==ERROR_INVALID_DATA);
    {static const WCHAR *const paths[]={L"C:\\Package\\",L"\\\\server\\share\\Package\\",
        L"\\\\?\\C:\\Package\\",L"\\\\?\\UNC\\server\\share\\",L"\\\\?\\unc\\server\\share\\"};
        for(unsigned i=0;i<sizeof(paths)/sizeof(paths[0]);++i){
            h.directory_bytes=(DWORD)((wcslen(paths[i])+1)*sizeof(WCHAR));h.bytes=sizeof(h)+h.directory_bytes;
            memcpy(copy,&h,sizeof(h));memcpy(copy+sizeof(h),paths[i],h.directory_bytes);
            CHECK(!ntw16s_packet_parse(copy,h.bytes,machine,&context));}}
    puts("PAYLOAD PASS fixed header, bounds, machine, flags, reserved fields, absolute path, termination");return 0;
}
