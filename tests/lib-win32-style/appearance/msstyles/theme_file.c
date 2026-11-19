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
    return has_key(path,L"Theme",L"DisplayName",L"Windows 3.x") &&
        has_key(path,L"Control Panel\\Desktop",L"TileWallpaper",L"0") &&
        has_key(path,L"VisualStyles",L"Path",L"Win16.msstyles") &&
        has_key(path,L"VisualStyles",L"Size",L"NormalSize") &&
        has_key(path,L"VisualStyles",L"ColorStyle",L"NormalColor") &&
        has_key(path,L"MasterThemeSelector",L"MTSM",L"DABJDKT");
}

int main(void)
{
    WCHAR folder[MAX_PATH],path[MAX_PATH];
    DWORD length=GetTempPathW(ARRAYSIZE(folder),folder);
    if(!length || length>=ARRAYSIZE(folder) || !GetTempFileNameW(folder,L"wst",0,path))
        return 1;
    bool passed=check_file(path);
    if(!DeleteFileW(path) || !passed){
        fputs("Companion theme roundtrip failed\n",stderr);
        return 1;
    }
    return 0;
}
