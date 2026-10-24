#include "run16s-exe/legacy_shell.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SHELL UNIT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static unsigned mode,calls;
static BOOL valid;
static BOOL WINAPI execute(SHELLEXECUTEINFOW *s)
{
    ++calls;valid=s->cbSize==sizeof(*s)&&s->fMask==(SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS)&&
        !s->lpVerb&&!s->lpDirectory&&!wcscmp(s->lpFile,L"C:\\space dir\\old.exe")&&
        !wcscmp(s->lpParameters,L"\"\" \"a b\" tail\\")&&s->nShow==SW_SHOWNORMAL;
    if(mode==0){s->hProcess=CreateEventW(NULL,TRUE,FALSE,NULL);return s->hProcess!=NULL;}
    SetLastError(mode==1?ERROR_CANCELLED:mode==2?ERROR_ACCESS_DENIED:0);return FALSE;
}
int wmain(void)
{
    WCHAR directory[MAX_PATH],file[MAX_PATH];CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,MAX_PATH));
    CHECK(swprintf_s(file,MAX_PATH,L"%ls\\image.exe",directory)>0);
    BYTE bytes[256]={0};IMAGE_DOS_HEADER *dos=(void *)bytes;dos->e_magic=IMAGE_DOS_SIGNATURE;
    for(unsigned kind=0;kind<7;++kind){
        dos->e_magic=kind==6?0:IMAGE_DOS_SIGNATURE;dos->e_lfanew=kind==4?4:kind?64:0;
        DWORD signature=kind==1?IMAGE_OS2_SIGNATURE:kind==2?IMAGE_OS2_SIGNATURE_LE:(kind==3||kind==4)?IMAGE_NT_SIGNATURE:0;
        memcpy(bytes+(kind==4?4:64),&signature,4);HANDLE out=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);DWORD written;
        CHECK(out!=INVALID_HANDLE_VALUE&&WriteFile(out,bytes,kind==5?12:sizeof(bytes),&written,NULL)&&CloseHandle(out));
        CHECK(run16s_legacy_image(file)==(kind<3));
    }
    CHECK(DeleteFileW(file)&&!run16s_legacy_image(file));
    CHECK(!run16s_legacy_shell(L"C:\\space dir\\old.exe",L"\"\" \"a b\" tail\\",execute));
    DWORD before,after;CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    for(mode=0;mode<4;++mode){DWORD error=run16s_legacy_shell(L"C:\\space dir\\old.exe",L"\"\" \"a b\" tail\\",execute);
        CHECK(valid&&error==(mode==0?0:mode==1?ERROR_CANCELLED:mode==2?ERROR_ACCESS_DENIED:ERROR_GEN_FAILURE));}
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&after)&&before==after&&calls==5);
    puts("SHELL UNIT PASS classification=8 delegation=5 raw_args=preserved no_ui_suppression handles=stable");return 0;
}
