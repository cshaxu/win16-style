#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/appearance/msstyles/theme_file.h"
#include <windows.h>
#include <string.h>
#include <wchar.h>

static bool has_key(PCWSTR file,PCWSTR section,PCWSTR key,PCWSTR expected)
{
    WCHAR value[64];
    return GetPrivateProfileStringW(section,key,L"",value,ARRAYSIZE(value),file) &&
        !wcscmp(value,expected);
}

static bool check_file(PCWSTR path)
{
    const msstyles_v4_variant variant={
        .size={(const unsigned char *)L"NormalSize",ARRAYSIZE(L"NormalSize")},
        .color={(const unsigned char *)L"NormalColor",ARRAYSIZE(L"NormalColor")}
    };
    FILE *file=_wfopen(path,L"wb");
    if(!file)return false;
    bool valid=!msstyles_theme_file_write(file,"Win16", "../Win16",&variant) &&
        !msstyles_theme_file_write(file,"Win16\n[Injected]","Win16",&variant) &&
        ftell(file)==0 && msstyles_theme_file_write(file,"Windows 3.x","Win16",&variant);
    if(fclose(file)!=0 || !valid)return false;
    file=_wfopen(path,L"rb");
    if(!file)return false;
    msstyles_theme_descriptor parsed={0};
    valid=msstyles_theme_file_read(file,&parsed);
    if(fclose(file)!=0 || !valid)return false;
    return !strcmp(parsed.display_name,"Windows 3.x") && !strcmp(parsed.file,"Win16.msstyles") && !strcmp(parsed.size,"NormalSize") &&
        !strcmp(parsed.color,"NormalColor") &&
        has_key(path,L"Theme",L"DisplayName",L"Windows 3.x") &&
        has_key(path,L"Control Panel\\Desktop",L"TileWallpaper",L"0") &&
        has_key(path,L"VisualStyles",L"Path",L"Win16.msstyles") &&
        has_key(path,L"VisualStyles",L"Size",L"NormalSize") &&
        has_key(path,L"VisualStyles",L"ColorStyle",L"NormalColor") &&
        has_key(path,L"MasterThemeSelector",L"MTSM",L"DABJDKT");
}

static bool optional_name(PCWSTR path,const char *input,const char *name)
{
    FILE *file=_wfopen(path,L"w+b");
    if(!file)return false;
    msstyles_theme_descriptor parsed={0};
    bool valid=fputs(input,file)!=EOF &&
        fputs("[VisualStyles]\nPath=Sample.msstyles\nSize=S\nColorStyle=C\n",file)!=EOF &&
        fseek(file,0,SEEK_SET)==0 && msstyles_theme_file_read(file,&parsed);
    return fclose(file)==0 && valid && !strcmp(parsed.display_name,name);
}

static bool reject(PCWSTR path,const char *text)
{
    FILE *file=_wfopen(path,L"w+b");
    if(!file)return false;
    msstyles_theme_descriptor original={"before.msstyles","BeforeSize","BeforeColor","BeforeName"};
    msstyles_theme_descriptor parsed=original;
    bool passed=fputs(text,file)!=EOF && fseek(file,0,SEEK_SET)==0 &&
        !msstyles_theme_file_read(file,&parsed) && !memcmp(&original,&parsed,sizeof(original));
    if(fclose(file)!=0 || !passed)return false;
    classic_theme_mapping source={0};
    return classic_theme_open(path,&source)==HRESULT_FROM_WIN32(ERROR_INVALID_DATA) &&
        !source.resources;
}

int main(void)
{
    WCHAR folder[MAX_PATH],path[MAX_PATH];
    DWORD length=GetTempPathW(ARRAYSIZE(folder),folder);
    if(!length || length>=ARRAYSIZE(folder) || !GetTempFileNameW(folder,L"wst",0,path))
        return 1;
    bool passed=check_file(path) &&
        optional_name(path,"[Theme]\nDisplayName=Clair & Nuit\n","Clair & Nuit") &&
        optional_name(path,"[Theme]\nDisplayName=@themeui.dll,-1\n","") &&
        optional_name(path,"[Theme]\nDisplayName=One\nDisplayName=Two\n","") &&
        optional_name(path,"[Theme]\nDisplayName=Invalid\tTab\n","") &&
        optional_name(path,"","") &&
        reject(path,"[VisualStyles]\nPath=../Win16.msstyles\nSize=S\nColorStyle=C\n") &&
        reject(path,"[VisualStyles]\nPath=C:\\Win16.msstyles\nSize=S\nColorStyle=C\n") &&
        reject(path,"[VisualStyles]\nPath=Win16.msstyles\nSize=S\nSize=S2\nColorStyle=C\n") &&
        reject(path,"[VisualStyles]\nPath=Win16.msstyles\nSize=S\n[Theme]\nColorStyle=C\n");
    if(!DeleteFileW(path) || !passed){
        fputs("Companion theme roundtrip failed\n",stderr);
        return 1;
    }
    return 0;
}
