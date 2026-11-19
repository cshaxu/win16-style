#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(expr) do{if(!(expr)){fprintf(stderr,"line %d: %s\n",__LINE__,#expr);return false;}}while(0)

static const unsigned char *resource(HMODULE module,PCWSTR type,PCWSTR name,DWORD *size)
{
    HRSRC item=FindResourceW(module,name,type);
    if(!item)return NULL;
    *size=SizeofResource(module,item);
    HGLOBAL handle=LoadResource(module,item);
    return handle?LockResource(handle):NULL;
}

static bool field(PCWSTR file,PCWSTR section,PCWSTR key,PCWSTR expected)
{
    WCHAR value[100];
    return GetPrivateProfileStringW(section,key,L"",value,ARRAYSIZE(value),file) &&
        !wcscmp(value,expected);
}

static bool variant_fields(HMODULE module,PCWSTR theme)
{
    DWORD size=0;
    const unsigned char *data=resource(module,L"VMAP",L"VMAP",&size);
    const PCWSTR keys[]={NULL,L"Size",L"ColorStyle"};
    CHECK(data);
    for(size_t i=0;i<ARRAYSIZE(keys);++i){
        uint32_t units;
        WCHAR value[64];
        CHECK(size>=sizeof(units));
        memcpy(&units,data,sizeof(units));
        size-=sizeof(units);
        data+=sizeof(units);
        CHECK(units>1 && units<=ARRAYSIZE(value));
        size_t length=units*sizeof(WCHAR),span=(length+3u)&~3u;
        CHECK(span<=size);
        memcpy(value,data,length);
        CHECK(value[units-1]==0 && wcslen(value)==units-1);
        if(keys[i])CHECK(field(theme,L"VisualStyles",keys[i],value));
        else CHECK(!_wcsicmp(value,L"Normal"));
        size-=span;
        data+=span;
    }
    return size==0;
}

static bool style_data(HMODULE module,uint32_t caption,uint32_t face)
{
    DWORD size=0;
    const unsigned char *data=resource(module,L"PACKTHEM_VERSION",MAKEINTRESOURCEW(1),&size);
    CHECK(data && size==2 && data[0]==4 && data[1]==0);
    data=resource(module,L"CMAP",L"CMAP",&size);
    CHECK(data && size>=2 && !(size%2) && !(data[size-1]|data[size-2]));
    unsigned indices[3]={0},classes=0;
    WCHAR value[48];
    size_t units=0;
    for(size_t i=0;i<size;i+=2){
        WCHAR unit;
        memcpy(&unit,data+i,2);
        if(unit){CHECK(units+1<ARRAYSIZE(value));value[units++]=unit;}
        else if(units){
            value[units]=0;
            ++classes;
            if(!wcscmp(value,L"Button"))indices[0]=classes;
            if(!wcscmp(value,L"ScrollBar"))indices[1]=classes;
            if(!wcscmp(value,L"Window"))indices[2]=classes;
            units=0;
        }
    }
    CHECK(indices[0] && indices[1] && indices[2]);
    const uint32_t values[]={face,face,caption};
    unsigned observed=0;
    data=resource(module,L"VARIANT",L"NORMAL",&size);
    CHECK(data);
    while(size){
        uint32_t item[8],color;
        CHECK(size>=sizeof(item));
        memcpy(item,data,sizeof(item));
        size-=sizeof(item);
        data+=sizeof(item);
        CHECK(item[2]<classes && !item[5] && !item[6] && item[7]<=size);
        size_t length=item[7],pad=(8-length%8)%8;
        CHECK(pad<=size-length);
        for(unsigned i=0;i<ARRAYSIZE(indices);++i){
            if(item[0]!=3802 || item[2]+1!=indices[i] || item[3]!=1 || item[4]!=(i==2))continue;
            CHECK(item[1]==204 && length==sizeof(color) && !(observed&(1u<<i)));
            memcpy(&color,data,sizeof(color));
            CHECK(color==values[i]);
            observed|=1u<<i;
        }
        size-=length+pad;
        data+=length+pad;
    }
    return observed==7;
}

static bool check_pair(PCWSTR root,PCWSTR stem,PCWSTR label,uint32_t caption,uint32_t face)
{
    WCHAR descriptor[MAX_PATH],carrier[MAX_PATH],filename[48];
    CHECK(swprintf(descriptor,ARRAYSIZE(descriptor),L"%ls/%ls/%ls.theme",root,stem,stem)>0);
    CHECK(swprintf(filename,ARRAYSIZE(filename),L"%ls.msstyles",stem)>0);
    CHECK(swprintf(carrier,ARRAYSIZE(carrier),L"%ls/%ls/%ls",root,stem,filename)>0);
    CHECK(field(descriptor,L"Theme",L"DisplayName",label));
    CHECK(field(descriptor,L"Control Panel\\Desktop",L"TileWallpaper",L"0"));
    CHECK(field(descriptor,L"VisualStyles",L"Path",filename));
    CHECK(field(descriptor,L"MasterThemeSelector",L"MTSM",L"DABJDKT"));
    HMODULE module=LoadLibraryExW(carrier,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(module);
    bool valid=variant_fields(module,descriptor) && style_data(module,caption,face);
    FreeLibrary(module);
    return valid;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    if(!check_pair(argv[1],L"Win16",L"Windows 3.x",RGB(164,200,240),RGB(192,192,192)) ||
       !check_pair(argv[1],L"Win2000",L"Windows 2000",RGB(0,0,128),RGB(212,208,200)))return 1;
    puts("Classic themes: both companion descriptions and v4 resources passed");
    return 0;
}
