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
    unsigned char bytes[CLASSIC_STYLE_RESOURCE_CAPACITY],reference[CLASSIC_STYLE_RESOURCE_CAPACITY],
        second[CLASSIC_STYLE_RESOURCE_CAPACITY];
    size_t length=classic_style_write(reference,sizeof(reference),&original,NULL,NULL,NULL,NULL);
    CHECK(length==40 && !memcmp(reference,"CSTY\x01\0\0\0\x28\0\0\0\x01\0\0\0",16));
    CHECK(!classic_style_write(bytes,length-1,&original,NULL,NULL,NULL,NULL));
    for(size_t end=0;end<length;++end)
        CHECK(classic_style_read(reference,end,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID && !group.count);
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK);
    CHECK(classic_style_write(second,sizeof(second),&group,NULL,NULL,NULL,NULL)==length && !memcmp(reference,second,length));

    memcpy(bytes,reference,length);
    bytes[4]=2;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_VERSION);
    bytes[4]=1;
    bytes[36]=bytes[32];
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    bytes[36]=reference[36];
    bytes[20]=0xff;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    bytes[20]=reference[20];

    memcpy(bytes+length,"\x0a\0\0\0\x01\0\0\0\xab",9);
    bytes[6]=1;
    bytes[8]=(unsigned char)(length+9);
    bytes[12]=2;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK && group.count==2);
    bytes[length+2]=1;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_REQUIRED_RECORD && group.count==2);
    bytes[16]=10;
    bytes[18]=0;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    bytes[6]=0;
    bytes[8]=(unsigned char)length;
    bytes[12]=1;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_NO_BUTTON_GROUP);

    memcpy(bytes,reference,length);
    memcpy(bytes+length,bytes+16,length-16);
    bytes[12]=2;
    bytes[8]=(unsigned char)(length*2-16);
    CHECK(classic_style_read(bytes,length*2-16,&group,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    const classic_scroll_metrics fixed={CLASSIC_SCROLL_EXTENT_FIXED,11};
    classic_scroll_metrics metrics={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed,NULL,NULL,NULL)==length+12);
    CHECK(classic_style_read(bytes,length+12,&group,&metrics,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        metrics.source==fixed.source && metrics.logical_extent==11);
    bytes[length+10]=0;
    CHECK(classic_style_read(bytes,length+12,&group,&metrics,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        metrics.logical_extent==11);
    const classic_menu_metrics menu={22,10,15,5};
    classic_menu_metrics columns={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed,&menu,NULL,NULL)==length+28);
    CHECK(classic_style_read(bytes,length+28,&group,&metrics,&columns,NULL,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&columns,&menu,sizeof(menu)));
    bytes[length+24]=5;
    CHECK(classic_style_read(bytes,length+28,&group,&metrics,&columns,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        !memcmp(&columns,&menu,sizeof(menu)));
    const classic_dialog_metrics dialog={{430,275,575,350,375,199,8,21},510,460,500,300,185,240,80,65,290,198};
    classic_dialog_metrics dimensions={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed,&menu,&dialog,NULL)==length+72);
    CHECK(classic_style_read(bytes,length+72,&group,&metrics,&columns,&dimensions,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&dimensions,&dialog,sizeof(dialog)));
    bytes[length+40]=0;bytes[length+41]=0; /* Places width below compact width. */
    CHECK(classic_style_read(bytes,length+72,&group,&metrics,&columns,&dimensions,NULL)==CLASSIC_STYLE_INVALID &&
        !memcmp(&dimensions,&dialog,sizeof(dialog)));
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,&dimensions,NULL)==CLASSIC_STYLE_OK &&
        !dimensions.file.compact_width && !dimensions.print_width);
    classic_dialog_policy mixed={CLASSIC_DIALOG_WIN16,CLASSIC_DIALOG_WIN2K,
        CLASSIC_DIALOG_WIN16,CLASSIC_DIALOG_NATIVE},policy={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,&mixed)==length+12);
    CHECK(classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,&policy)==CLASSIC_STYLE_OK &&
        !memcmp(&mixed,&policy,sizeof(policy)) && group.slots[1].role==CLASSIC_BUTTON_CLOSE);
    bytes[length+11]=3;
    CHECK(classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,&policy)==CLASSIC_STYLE_INVALID &&
        !memcmp(&mixed,&policy,sizeof(policy)));
    bytes[length+11]=0;
    memcpy(bytes+length+12,bytes+length,12);
    bytes[8]=(unsigned char)(length+24);bytes[12]=3;
    CHECK(classic_style_read(bytes,length+24,&group,NULL,NULL,NULL,&policy)==CLASSIC_STYLE_INVALID);
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,NULL,&policy)==CLASSIC_STYLE_OK &&
        !policy.appearance);
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
    classic_menu_metrics columns;
    classic_dialog_metrics dialog;
    classic_dialog_policy policy;
    enum classic_style_result result=classic_style_read(handle?LockResource(handle):NULL,size,&group,&metrics,&columns,&dialog,&policy);
    const WORD language=MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);
    if(measure==CLASSIC_SCROLL_EXTENT_SYSTEM){
        for(WORD id=201;id<=203;++id)
            CHECK(FindResourceExW(module,MAKEINTRESOURCEW(5),MAKEINTRESOURCEW(id),language));
    }else CHECK(!FindResourceExW(module,MAKEINTRESOURCEW(5),MAKEINTRESOURCEW(201),language));
    FreeLibrary(module);
    CHECK(result==CLASSIC_STYLE_OK && group.count==expected_count);
    CHECK(group.slots[group.count-1].role==expected_last && group.slots[group.count-1].gap_before==expected_gap);
    CHECK(metrics.source==measure && !metrics.logical_extent);
    unsigned route=measure==CLASSIC_SCROLL_EXTENT_SYSTEM?CLASSIC_DIALOG_WIN2K:CLASSIC_DIALOG_WIN16;
    CHECK(policy.appearance==route && policy.file==route && policy.folder==route && policy.print==route);
    if(measure==CLASSIC_SCROLL_EXTENT_CROSS_AXIS){
        CHECK(columns.check_column==18 && columns.shortcut_gap==8 && columns.arrow_column==12 && columns.plain_column==4);
        CHECK(!dialog.file.compact_width && !dialog.print_width);
    }else{
        CHECK(!columns.check_column);
        CHECK(dialog.file.compact_width==427 && dialog.file.compact_height==272 &&
            dialog.file.places_width==565 && dialog.file.places_height==345 &&
            dialog.file.client_dlu_width==370 && dialog.file.client_dlu_height==197 &&
            dialog.file.gap==7 && dialog.file.bottom_reserve==20);
        CHECK(dialog.print_width==500 && dialog.print_full_height==450 && dialog.print_large_height==490);
        CHECK(dialog.color_width==298 && dialog.color_height==184 && dialog.find_width==236 &&
            dialog.find_native_height==74 && dialog.find_classic_height==62 && dialog.font_width==287 && dialog.font_height==196);
    }
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==1)return fixtures();
    if(argc!=2)return 2;
    return packaged(argv[1],L"Win16",3,CLASSIC_BUTTON_MAXIMIZE,0,CLASSIC_SCROLL_EXTENT_CROSS_AXIS) ||
        packaged(argv[1],L"Win2K",4,CLASSIC_BUTTON_CLOSE,2,CLASSIC_SCROLL_EXTENT_SYSTEM);
}
