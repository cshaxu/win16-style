#include "theme_kind.h"

int win16_control_theme_kind(LPCWSTR name)
{
    WCHAR safe[9];SIZE_T got=0;
    if(!name || !ReadProcessMemory(GetCurrentProcess(),name,safe,7*sizeof(WCHAR),&got) || got!=7*sizeof(WCHAR))
        return WIN16_CONTROL_THEME_OTHER;
    if(safe[6]==0 && !lstrcmpiW(safe,L"BUTTON"))return WIN16_CONTROL_THEME_BUTTON;
    if(!ReadProcessMemory(GetCurrentProcess(),name,safe,sizeof(safe),&got) || got!=sizeof(safe))return WIN16_CONTROL_THEME_OTHER;
    if(safe[8]==0 && !lstrcmpiW(safe,L"COMBOBOX"))return WIN16_CONTROL_THEME_COMBOBOX;
    return safe[8]==0 && !lstrcmpiW(safe,L"TEXTGLOW")?WIN16_CONTROL_THEME_TEXTGLOW:WIN16_CONTROL_THEME_OTHER;
}

