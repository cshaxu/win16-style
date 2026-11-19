#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(expr) do{if(!(expr)){fprintf(stderr,"line %d: %s\n",__LINE__,#expr);return 1;}}while(0)

static int fixtures(void)
{
    const classic_button_group original={2,1,3,{{CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0},
        {CLASSIC_BUTTON_CLOSE,CLASSIC_BUTTON_RIGHT,2}}};
    classic_button_group group={0};
    unsigned char bytes[96],reference[96],second[96];
    size_t length=classic_style_write(reference,sizeof(reference),&original,NULL);
    CHECK(length==40 && !memcmp(reference,"CSTY\x01\0\0\0\x28\0\0\0\x01\0\0\0",16));
    CHECK(!classic_style_write(bytes,length-1,&original,NULL));
    for(size_t end=0;end<length;++end)
        CHECK(classic_style_read(reference,end,&group,NULL)==CLASSIC_STYLE_INVALID && !group.count);
    CHECK(classic_style_read(reference,length,&group,NULL)==CLASSIC_STYLE_OK);
    CHECK(classic_style_write(second,sizeof(second),&group,NULL)==length && !memcmp(reference,second,length));

    memcpy(bytes,reference,length);
    bytes[4]=2;
    CHECK(classic_style_read(bytes,length,&group,NULL)==CLASSIC_STYLE_VERSION);
    bytes[4]=1;
    bytes[36]=bytes[32];
    CHECK(classic_style_read(bytes,length,&group,NULL)==CLASSIC_STYLE_INVALID);
    bytes[36]=reference[36];
    bytes[20]=0xff;
    CHECK(classic_style_read(bytes,length,&group,NULL)==CLASSIC_STYLE_INVALID);
    bytes[20]=reference[20];

    memcpy(bytes+length,"\x09\0\0\0\x01\0\0\0\xab",9);
    bytes[6]=1;
    bytes[8]=(unsigned char)(length+9);
    bytes[12]=2;
    CHECK(classic_style_read(bytes,length+9,&group,NULL)==CLASSIC_STYLE_OK && group.count==2);
    bytes[length+2]=1;
    CHECK(classic_style_read(bytes,length+9,&group,NULL)==CLASSIC_STYLE_REQUIRED_RECORD && group.count==2);
    bytes[16]=9;
    bytes[18]=0;
    CHECK(classic_style_read(bytes,length,&group,NULL)==CLASSIC_STYLE_INVALID);
    bytes[6]=0;
    bytes[8]=(unsigned char)length;
    bytes[12]=1;
    CHECK(classic_style_read(bytes,length,&group,NULL)==CLASSIC_STYLE_NO_BUTTON_GROUP);

    memcpy(bytes,reference,length);
    memcpy(bytes+length,bytes+16,length-16);
    bytes[12]=2;
    bytes[8]=(unsigned char)(length*2-16);
    CHECK(classic_style_read(bytes,length*2-16,&group,NULL)==CLASSIC_STYLE_INVALID);
    const classic_scroll_metrics fixed={CLASSIC_SCROLL_EXTENT_FIXED,11};
    classic_scroll_metrics metrics={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed)==length+12);
    CHECK(classic_style_read(bytes,length+12,&group,&metrics)==CLASSIC_STYLE_OK &&
        metrics.source==fixed.source && metrics.logical_extent==11);
    bytes[length+10]=0;
    CHECK(classic_style_read(bytes,length+12,&group,&metrics)==CLASSIC_STYLE_INVALID &&
        metrics.logical_extent==11);
    return 0;
}

static int packaged(PCWSTR root,PCWSTR name,unsigned expected_count,unsigned expected_last,unsigned expected_gap,
    unsigned measure)
{
    WCHAR path[MAX_PATH];
    CHECK(swprintf(path,ARRAYSIZE(path),L"%ls/%ls/%ls.msstyles",root,name,name)>0);
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(module);
    HRSRC resource=FindResourceW(module,L"CLASSIC_STYLE",MAKEINTRESOURCEW(10));
    DWORD size=resource?SizeofResource(module,resource):0;
    HGLOBAL handle=resource?LoadResource(module,resource):NULL;
    classic_button_group group;
    classic_scroll_metrics metrics;
    enum classic_style_result result=classic_style_read(handle?LockResource(handle):NULL,size,&group,&metrics);
    FreeLibrary(module);
    CHECK(result==CLASSIC_STYLE_OK && group.count==expected_count);
    CHECK(group.slots[group.count-1].role==expected_last && group.slots[group.count-1].gap_before==expected_gap);
    CHECK(metrics.source==measure && !metrics.logical_extent);
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==1)return fixtures();
    if(argc!=2)return 2;
    return packaged(argv[1],L"Win16",3,CLASSIC_BUTTON_MAXIMIZE,0,CLASSIC_SCROLL_EXTENT_CROSS_AXIS) ||
        packaged(argv[1],L"Win2K",4,CLASSIC_BUTTON_CLOSE,2,CLASSIC_SCROLL_EXTENT_SYSTEM);
}
