#include "print_resources.h"
#include <stddef.h>
#include <stdint.h>

BOOL style_print_global_available(HGLOBAL memory)
{if(!memory)return TRUE;UINT flags=GlobalFlags(memory);return flags!=GMEM_INVALID_HANDLE && !(flags & GMEM_LOCKCOUNT);}
HGLOBAL style_print_clone_global(HGLOBAL original)
{
    SIZE_T size=original?GlobalSize(original):0;void *src=size?GlobalLock(original):NULL;
    HGLOBAL copy=src?GlobalAlloc(GMEM_MOVEABLE,size):NULL;void *dst=copy?GlobalLock(copy):NULL;
    if(!dst){if(src)GlobalUnlock(original);if(copy)GlobalFree(copy);return NULL;}
    CopyMemory(dst,src,size);GlobalUnlock(copy);GlobalUnlock(original);return copy;
}
/* Apply is captured before PrintDlg commits the print-to-file selection. */
HGLOBAL style_print_clone_file_devnames(HGLOBAL original,BOOL ansi)
{
    SIZE_T bytes=original?GlobalSize(original):0,unit=ansi?1:sizeof(WCHAR);
    SIZE_T offset=(bytes+unit-1)/unit,extra=6*unit;
    if(bytes<sizeof(DEVNAMES) || offset>65535 || offset*unit>SIZE_MAX-extra)return NULL;
    const void *src=GlobalLock(original);if(!src)return NULL;
    HGLOBAL result=GlobalAlloc(GMEM_MOVEABLE,offset*unit+extra);void *dst=result?GlobalLock(result):NULL;
    if(dst){CopyMemory(dst,src,bytes);((DEVNAMES *)dst)->wOutputOffset=(WORD)offset;
        if(ansi)CopyMemory((BYTE *)dst+offset,"FILE:",6);
        else CopyMemory((WCHAR *)dst+offset,L"FILE:",6*sizeof(WCHAR));
        GlobalUnlock(result);}
    else if(result){GlobalFree(result);result=NULL;}
    GlobalUnlock(original);return result;
}
HDC style_print_create_dc(void *mode,DEVNAMES *names,BOOL ansi,DWORD flags)
{
    if(ansi){
        if(flags & PD_RETURNDC)return CreateDCA((char*)names+names->wDriverOffset,(char*)names+names->wDeviceOffset,(char*)names+names->wOutputOffset,mode);
        if(flags & PD_RETURNIC)return CreateICA((char*)names+names->wDriverOffset,(char*)names+names->wDeviceOffset,(char*)names+names->wOutputOffset,mode);
    }else{
        if(flags & PD_RETURNDC)return CreateDCW((WCHAR*)names+names->wDriverOffset,(WCHAR*)names+names->wDeviceOffset,(WCHAR*)names+names->wOutputOffset,mode);
        if(flags & PD_RETURNIC)return CreateICW((WCHAR*)names+names->wDriverOffset,(WCHAR*)names+names->wDeviceOffset,(WCHAR*)names+names->wOutputOffset,mode);
    }
    return NULL;
}
void style_print_set_copies(void *data,BOOL ansi,short copies,BOOL collate)
{
    short value=collate?DMCOLLATE_TRUE:DMCOLLATE_FALSE;
    if(ansi){DEVMODEA *mode=data;mode->dmCopies=copies;mode->dmCollate=value;}
    else{DEVMODEW *mode=data;mode->dmCopies=copies;mode->dmCollate=value;}
}
/* PrintDlgEx embeds the requested copy count in the returned printer DC.
 * The legacy PrintDlg DC can omit it even when nCopies is returned correctly. */
HDC style_print_create_print_dc(HGLOBAL mode,HGLOBAL names,BOOL ansi,DWORD copies,DWORD flags)
{
    if(!copies || copies>32767)return NULL;
    HGLOBAL dc_mode=style_print_clone_global(mode);if(!dc_mode)return NULL;
    void *data=GlobalLock(dc_mode);DEVNAMES *dn=names?GlobalLock(names):NULL;HDC dc=NULL;
    if(data && dn){
        style_print_set_copies(data,ansi,(short)copies,(flags&PD_COLLATE)!=0);
        dc=style_print_create_dc(data,dn,ansi,flags);
    }
    if(data)GlobalUnlock(dc_mode);
    if(dn)GlobalUnlock(names);
    GlobalFree(dc_mode);return dc;
}
/* Old PrintDlg may return application-managed copies even when the driver
 * flag was requested, especially for collated copies. Convert that result
 * before passing the device mode back through the PrintDlgEx interface. */
BOOL style_print_normalize_driver_copies(HGLOBAL mode,BOOL ansi,WORD *copies,DWORD *flags)
{
    if(!mode || !copies || !*copies || *copies>32767 || !flags)return FALSE;
    if(*copies>1 || (*flags&PD_COLLATE)){
        SIZE_T needed=ansi?offsetof(DEVMODEA,dmCollate)+sizeof(((DEVMODEA *)0)->dmCollate):
            offsetof(DEVMODEW,dmCollate)+sizeof(((DEVMODEW *)0)->dmCollate);
        if(GlobalSize(mode)<needed)return FALSE;
        void *data=GlobalLock(mode);if(!data)return FALSE;
        style_print_set_copies(data,ansi,(short)*copies,(*flags&PD_COLLATE)!=0);
        GlobalUnlock(mode);
    }
    *copies=1;*flags &= ~(DWORD)PD_COLLATE;return TRUE;
}
