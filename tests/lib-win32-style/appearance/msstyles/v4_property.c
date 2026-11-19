#include "lib-win32-style/appearance/msstyles/v4_property.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define CHECK(condition) do{if(!(condition)){fprintf(stderr,"line %d: %s\n",__LINE__,#condition);return 1;}}while(0)

static int fixtures(void)
{
    const unsigned char payload[]={0x02,0x10,0x20,0x30};
    unsigned char bytes[72]={0};
    msstyles_v4_property value={4001,200,2,1,1,0,sizeof(payload),payload};
    msstyles_v4_property got={0};
    size_t used=0;
    CHECK(msstyles_v4_property_write(bytes,sizeof(bytes),3,&value,&used) && used==40);
    CHECK(!msstyles_v4_property_read(bytes,31,3,&got,&used));
    CHECK(!msstyles_v4_property_read(bytes,39,3,&got,&used) && got.class_id==0 && used==40);
    CHECK(!msstyles_v4_property_read(bytes,40,2,&got,&used));
    CHECK(msstyles_v4_property_read(bytes,40,3,&got,&used));
    CHECK(used==40 && got.name_id==4001 && got.type_id==200 && got.class_id==2);
    CHECK(got.state_id==1 && got.data==bytes+32 && !memcmp(got.data,payload,sizeof(payload)));
    CHECK(bytes[36]==0 && bytes[39]==0);
    bytes[24]=1;
    CHECK(!msstyles_v4_property_read(bytes,40,3,&got,&used));
    bytes[24]=0;
    memset(bytes+28,0xff,4);
    CHECK(!msstyles_v4_property_read(bytes,40,3,&got,&used));
    CHECK(!msstyles_v4_property_write(bytes,39,3,&value,&used));
    value.short_value=1946;
    value.data=NULL;
    value.declared_size=16;
    CHECK(msstyles_v4_property_write(bytes,32,3,&value,&used) && used==32);
    CHECK(msstyles_v4_property_read(bytes,32,3,&got,&used) && got.short_value==1946);
    CHECK(used==32 && got.declared_size==16 && !got.data);
    return 0;
}

static const unsigned char *resource(HMODULE module,PCWSTR type,PCWSTR name,DWORD *size)
{
    HRSRC entry=FindResourceW(module,name,type);
    if(!entry)return NULL;
    *size=SizeofResource(module,entry);
    HGLOBAL loaded=LoadResource(module,entry);
    return loaded?LockResource(loaded):NULL;
}

static int walk_sample(HMODULE module)
{
    DWORD class_bytes=0,record_bytes=0,version_bytes=0;
    const unsigned char *version=resource(module,L"PACKTHEM_VERSION",MAKEINTRESOURCEW(1),&version_bytes);
    const unsigned char *classes=resource(module,L"CMAP",L"CMAP",&class_bytes);
    const unsigned char *records=resource(module,L"VARIANT",L"NORMAL",&record_bytes);
    CHECK(version && version_bytes==2 && version[0]==4 && version[1]==0);
    CHECK(classes && class_bytes && !(class_bytes%2) && records && record_bytes);
    size_t class_count=0;
    bool word=false;
    for(size_t i=0;i<class_bytes;i+=2){
        if(classes[i] || classes[i+1])word=true;
        else if(word){++class_count;word=false;}
    }
    CHECK(class_count>3 && !word);
    size_t pos=0,count=0;
    while(pos<record_bytes){
        msstyles_v4_property value;
        size_t used=0,written=0;
        unsigned char storage[128];
        CHECK(msstyles_v4_property_read(records+pos,record_bytes-pos,class_count,&value,&used));
        unsigned char *output=used<=sizeof(storage)?storage:HeapAlloc(GetProcessHeap(),0,used);
        CHECK(output);
        bool equal=msstyles_v4_property_write(output,used,class_count,&value,&written) &&
            written==used && !memcmp(output,records+pos,used);
        if(output!=storage)HeapFree(GetProcessHeap(),0,output);
        CHECK(equal);
        pos+=used;
        ++count;
    }
    CHECK(pos==record_bytes && count>100);
    printf("MSSTYLES V4 PASS classes=%zu records=%zu bytes=%lu\n",class_count,count,(unsigned long)record_bytes);
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==1)return fixtures();
    CHECK(argc==2);
    HMODULE module=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(module);
    int result=walk_sample(module);
    FreeLibrary(module);
    return result;
}
