/* Translate only contracts supported by the system old-style file dialog. */
#include "file_contract.h"
#include <string.h>
#include <wchar.h>
#define COMPATIBLE_FLAGS (OFN_READONLY | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | \
    OFN_NOCHANGEDIR | OFN_SHOWHELP | OFN_NOVALIDATE | OFN_PATHMUSTEXIST | \
    OFN_FILEMUSTEXIST | OFN_CREATEPROMPT | OFN_NOREADONLYRETURN | OFN_NOTESTFILECREATE | \
    OFN_NONETWORKBUTTON | OFN_EXPLORER | OFN_LONGNAMES | OFN_EXTENSIONDIFFERENT)
#define CONTRACT(suffix,type,legacy_size) \
BOOL win16_dialog_file_prepare_##suffix(const type *value,type *local,DWORD extra,LPOFNHOOKPROC hook) \
{ \
    BOOL full=value && value->lStructSize==sizeof(*value); \
    BOOL legacy=value && value->lStructSize==(legacy_size); \
    if(!(full || legacy) || (value->Flags & ~(COMPATIBLE_FLAGS | extra)) || \
       (full && value->FlagsEx))return FALSE; \
    memset(local,0,sizeof(*local));memcpy(local,value,value->lStructSize); \
    local->lStructSize=sizeof(*local); \
    local->Flags=(local->Flags & ~OFN_EXPLORER) | OFN_LONGNAMES | OFN_ENABLEHOOK; \
    local->lpfnHook=hook;return TRUE; \
} \
void win16_dialog_file_result_##suffix(type *value,const type *local) \
{ \
    value->Flags=(value->Flags & ~(OFN_READONLY | OFN_EXTENSIONDIFFERENT)) | \
        (local->Flags & (OFN_READONLY | OFN_EXTENSIONDIFFERENT)); \
    value->nFilterIndex=local->nFilterIndex;value->nFileOffset=local->nFileOffset; \
    value->nFileExtension=local->nFileExtension; \
}
CONTRACT(a,OPENFILENAMEA,OPENFILENAME_SIZE_VERSION_400A)
CONTRACT(w,OPENFILENAMEW,OPENFILENAME_SIZE_VERSION_400W)

DWORD win16_dialog_com_file_flags(FILEOPENDIALOGOPTIONS flags)
{
    DWORD result=OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_ENABLEHOOK;
    if(flags&FOS_ALLOWMULTISELECT)result|=OFN_ALLOWMULTISELECT;
    if(flags&FOS_OVERWRITEPROMPT)result|=OFN_OVERWRITEPROMPT;
    if(flags&FOS_NOCHANGEDIR)result|=OFN_NOCHANGEDIR;
    if(flags&FOS_PATHMUSTEXIST)result|=OFN_PATHMUSTEXIST;
    if(flags&FOS_FILEMUSTEXIST)result|=OFN_FILEMUSTEXIST;
    if(flags&FOS_CREATEPROMPT)result|=OFN_CREATEPROMPT;
    if(flags&FOS_NOREADONLYRETURN)result|=OFN_NOREADONLYRETURN;
    if(flags&FOS_NOTESTFILECREATE)result|=OFN_NOTESTFILECREATE;
    if(flags&FOS_NODEREFERENCELINKS)result|=OFN_NODEREFERENCELINKS;
    return result;
}

BOOL win16_dialog_com_can_bridge(HRESULT options_result,FILEOPENDIALOGOPTIONS flags,
    BOOL unsupported,BOOL has_events,BOOL save,BOOL experimental_events,BOOL experimental_hidden)
{
    DWORD supported=FOS_OVERWRITEPROMPT|FOS_NOCHANGEDIR|FOS_FORCEFILESYSTEM|
        FOS_PATHMUSTEXIST|FOS_FILEMUSTEXIST|FOS_CREATEPROMPT|FOS_NOREADONLYRETURN|
        FOS_NOTESTFILECREATE|FOS_PICKFOLDERS|FOS_ALLOWMULTISELECT|FOS_FORCESHOWHIDDEN|FOS_NODEREFERENCELINKS;
    if(experimental_events)supported|=FOS_SHAREAWARE;
    if(FAILED(options_result) || unsupported || (flags & ~supported) ||
       (flags&FOS_ALLOWMULTISELECT))return FALSE;
    if(experimental_events){if((flags&FOS_SHAREAWARE) && !has_events)return FALSE;}
    else if(has_events)return FALSE;
    if((flags&FOS_NODEREFERENCELINKS) && (save || (flags&FOS_PICKFOLDERS)))return FALSE;
    if((flags&FOS_FORCESHOWHIDDEN) && (!experimental_hidden || (flags&FOS_PICKFOLDERS)))return FALSE;
    return TRUE;
}

BOOL win16_dialog_format_filters(UINT count,const COMDLG_FILTERSPEC *items,WCHAR **out)
{
    size_t chars=1;
    for(UINT i=0;i<count;++i){
        if(!items[i].pszName || !items[i].pszSpec)return FALSE;
        size_t name=wcslen(items[i].pszName),spec=wcslen(items[i].pszSpec);
        if(name>32767 || spec>32767 || name+spec+2>65535-chars)return FALSE;
        chars+=name+spec+2;
    }
    WCHAR *filter=CoTaskMemAlloc(chars*sizeof(WCHAR));
    if(!filter)return FALSE;
    WCHAR *at=filter;
    for(UINT i=0;i<count;++i){size_t n=wcslen(items[i].pszName)+1;memcpy(at,items[i].pszName,n*sizeof(WCHAR));at+=n;
        n=wcslen(items[i].pszSpec)+1;memcpy(at,items[i].pszSpec,n*sizeof(WCHAR));at+=n;}
    *at=0;*out=filter;return TRUE;
}

BOOL win16_dialog_copy_text(PCWSTR text,WCHAR **out)
{
    WCHAR *copy=NULL;
    if(text){size_t bytes=(wcslen(text)+1)*sizeof(WCHAR);copy=CoTaskMemAlloc(bytes);
        if(!copy)return FALSE;
        memcpy(copy,text,bytes);}
    *out=copy;return TRUE;
}
