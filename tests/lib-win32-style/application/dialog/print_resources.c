#include "lib-win32-style/application/dialog/print_resources.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("PRINT_RESOURCES_FAIL line=%d\n",__LINE__);return 1;}}while(0)
static int check_mode(BOOL ansi)
{
    SIZE_T size=ansi?sizeof(DEVMODEA):sizeof(DEVMODEW);
    HGLOBAL mode=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,size);CHECK(mode);
    void *data=GlobalLock(mode);CHECK(data);
    CHECK(!style_print_global_available(mode));GlobalUnlock(mode);CHECK(style_print_global_available(mode));
    HGLOBAL copy=style_print_clone_global(mode);CHECK(copy && copy!=mode && GlobalSize(copy)==GlobalSize(mode));
    WORD copies=3;DWORD flags=PD_COLLATE;CHECK(style_print_normalize_driver_copies(copy,ansi,&copies,&flags));
    CHECK(copies==1 && !(flags&PD_COLLATE));data=GlobalLock(copy);CHECK(data);
    if(ansi){DEVMODEA *m=data;CHECK(m->dmCopies==3 && m->dmCollate==DMCOLLATE_TRUE);}
    else{DEVMODEW *m=data;CHECK(m->dmCopies==3 && m->dmCollate==DMCOLLATE_TRUE);}
    GlobalUnlock(copy);data=GlobalLock(mode);CHECK(data);
    if(ansi)CHECK(((DEVMODEA *)data)->dmCopies==0);else CHECK(((DEVMODEW *)data)->dmCopies==0);
    GlobalUnlock(mode);GlobalFree(copy);GlobalFree(mode);return 0;
}
static int check_names(BOOL ansi)
{
    HGLOBAL source=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,64);CHECK(source);
    HGLOBAL copy=style_print_clone_file_devnames(source,ansi);CHECK(copy && copy!=source);
    DEVNAMES *names=GlobalLock(copy);CHECK(names);
    if(ansi)CHECK(!strcmp((char *)names+names->wOutputOffset,"FILE:"));
    else CHECK(!lstrcmpW((WCHAR *)names+names->wOutputOffset,L"FILE:"));
    GlobalUnlock(copy);GlobalFree(copy);GlobalFree(source);return 0;
}
static int check_snapshot(BOOL ansi)
{
    HGLOBAL mode=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,ansi?sizeof(DEVMODEA):sizeof(DEVMODEW));
    HGLOBAL names=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,64);
    CHECK(mode && names);
    style_print_snapshot saved={0},driver={0};
    CHECK(style_print_snapshot_create(mode,names,ansi,3,PD_COLLATE,&saved));
    CHECK(saved.mode!=mode && saved.names!=names && !saved.dc && saved.copies==3 && saved.flags==PD_COLLATE);
    CHECK(!style_print_snapshot_create(mode,NULL,ansi,4,PD_COLLATE,&saved));
    CHECK(saved.copies==3 && saved.mode && saved.names);
    CHECK(style_print_snapshot_create(mode,names,ansi,5,PD_COLLATE|PD_USEDEVMODECOPIESANDCOLLATE|PD_PRINTTOFILE,&driver));
    CHECK(driver.copies==1 && !driver.dc && driver.flags==(PD_USEDEVMODECOPIESANDCOLLATE|PD_PRINTTOFILE));
    void *data=GlobalLock(driver.mode);
    CHECK(data);
    if(ansi){DEVMODEA *value=data;CHECK(value->dmCopies==5 && value->dmCollate==DMCOLLATE_TRUE);}
    else{DEVMODEW *value=data;CHECK(value->dmCopies==5 && value->dmCollate==DMCOLLATE_TRUE);}
    GlobalUnlock(driver.mode);
    DEVNAMES *file=GlobalLock(driver.names);
    CHECK(file && (ansi?!strcmp((char *)file+file->wOutputOffset,"FILE:"):
        !lstrcmpW((WCHAR *)file+file->wOutputOffset,L"FILE:")));
    GlobalUnlock(driver.names);
    style_print_snapshot_dispose(&saved);
    style_print_snapshot_dispose(&driver);
    CHECK(!saved.mode && !driver.mode && style_print_global_available(mode) && style_print_global_available(names));
    GlobalFree(names);
    GlobalFree(mode);
    return 0;
}
int main(void)
{
    CHECK(style_print_global_available(NULL));CHECK(!style_print_clone_global(NULL));
    CHECK(!style_print_clone_file_devnames(NULL,FALSE));
    CHECK(!style_print_create_print_dc(NULL,NULL,FALSE,0,0));
    WORD count=0;DWORD flags=0;CHECK(!style_print_normalize_driver_copies(NULL,FALSE,&count,&flags));
    CHECK(!check_mode(FALSE) && !check_mode(TRUE) && !check_names(FALSE) && !check_names(TRUE));
    CHECK(!check_snapshot(FALSE) && !check_snapshot(TRUE));
    puts("PRINT_RESOURCES_PASS clone ownership locked ansi unicode copies file");return 0;
}
