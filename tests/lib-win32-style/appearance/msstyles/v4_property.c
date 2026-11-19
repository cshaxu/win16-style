#include "lib-win32-style/appearance/msstyles/v4_property.h"
#include "lib-win32-style/appearance/msstyles/v4_tables.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define CHECK(condition) do{if(!(condition)){fprintf(stderr,"line %d: %s\n",__LINE__,#condition);return 1;}}while(0)

static int variant_fixture(void)
{
    const unsigned char record[]={'A',0,0,0};
    const unsigned char size[]={'B',0,'C',0,0,0};
    const msstyles_v4_variant value={{record,2},{size,3},{record,2}};
    msstyles_v4_variant got={0};
    unsigned char bytes[28];
    memset(bytes,0xff,sizeof(bytes));
    size_t used=0;
    CHECK(!msstyles_v4_variant_write(bytes,sizeof(bytes)-1,&value,&used));
    CHECK(used==0 && bytes[0]==0xff);
    CHECK(msstyles_v4_variant_write(bytes,sizeof(bytes),&value,&used) && used==sizeof(bytes));
    CHECK(!msstyles_v4_variant_read(bytes,sizeof(bytes)-1,&got,&used) && !got.record.bytes);
    CHECK(msstyles_v4_variant_read(bytes,sizeof(bytes),&got,&used));
    CHECK(used==sizeof(bytes) && got.record.units==2 && got.size.units==3 && got.color.units==2);
    CHECK(!memcmp(got.size.bytes,size,sizeof(size)) && bytes[18]==0 && bytes[19]==0);
    bytes[12]=0;
    CHECK(!msstyles_v4_variant_read(bytes,sizeof(bytes),&got,&used));
    memset(bytes,0xff,4);
    CHECK(!msstyles_v4_variant_read(bytes,sizeof(bytes),&got,&used));
    msstyles_v4_name name;
    CHECK(msstyles_v4_class_write(bytes,sizeof(bytes),value.size,&used) && used==8);
    CHECK(!msstyles_v4_class_read(bytes,7,&name,&used));
    CHECK(msstyles_v4_class_read(bytes,8,&name,&used));
    CHECK(used==8 && name.units==3 && !memcmp(name.bytes,size,sizeof(size)));
    bytes[7]=1;
    CHECK(!msstyles_v4_class_read(bytes,8,&name,&used));
    return 0;
}

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
    return variant_fixture();
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
    DWORD class_bytes=0,record_bytes=0,version_bytes=0,variant_bytes=0;
    const unsigned char *version=resource(module,L"PACKTHEM_VERSION",MAKEINTRESOURCEW(1),&version_bytes);
    const unsigned char *classes=resource(module,L"CMAP",L"CMAP",&class_bytes);
    const unsigned char *records=resource(module,L"VARIANT",L"NORMAL",&record_bytes);
    const unsigned char *variants=resource(module,L"VMAP",L"VMAP",&variant_bytes);
    CHECK(version && version_bytes==2 && version[0]==4 && version[1]==0);
    CHECK(classes && class_bytes && !(class_bytes%2) && records && record_bytes);
    CHECK(variants);
    msstyles_v4_variant variant;
    unsigned char variant_copy[128];
    size_t read=0,written=0;
    CHECK(msstyles_v4_variant_read(variants,variant_bytes,&variant,&read) && read==variant_bytes);
    CHECK(variant.record.units==ARRAYSIZE(L"Normal") && variant.size.units==ARRAYSIZE(L"NormalSize") &&
        variant.color.units==ARRAYSIZE(L"NormalColor"));
    CHECK(!memcmp(variant.record.bytes,L"Normal",sizeof(L"Normal")) &&
        !memcmp(variant.size.bytes,L"NormalSize",sizeof(L"NormalSize")) &&
        !memcmp(variant.color.bytes,L"NormalColor",sizeof(L"NormalColor")));
    CHECK(msstyles_v4_variant_write(variant_copy,sizeof(variant_copy),&variant,&written));
    CHECK(written==variant_bytes && !memcmp(variant_copy,variants,written));
    size_t class_count=0,class_pos=0;
    while(class_pos<class_bytes){
        msstyles_v4_name name;
        unsigned char storage[256];
        CHECK(msstyles_v4_class_read(classes+class_pos,class_bytes-class_pos,&name,&read));
        CHECK(msstyles_v4_class_write(storage,sizeof(storage),name,&written));
        CHECK(read==written && !memcmp(storage,name.bytes,read));
        class_pos+=read;
        ++class_count;
    }
    CHECK(class_count>3 && class_pos==class_bytes);
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
