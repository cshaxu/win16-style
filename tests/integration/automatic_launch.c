#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 500000
#include "../ntw16s-dll/isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LAUNCH MATRIX FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static int matrix(int argc,WCHAR **argv)
{
    WCHAR scratch[MAX_PATH],package[MAX_PATH],launcher[MAX_PATH],dll32[MAX_PATH],dll64[MAX_PATH],command[32768];
    WCHAR targets[2][MAX_PATH];unsigned suite;
    DWORD code;STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    CHECK(argc==6 || argc==7);
    suite=argc==6?0:!wcscmp(argv[6],L"dialogs")?1:2;
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    CHECK(swprintf_s(package,MAX_PATH,L"%ls\\package with spaces",scratch)>0);
    CHECK(CreateDirectoryW(package,NULL) || GetLastError()==ERROR_ALREADY_EXISTS);
    CHECK(swprintf_s(launcher,MAX_PATH,L"%ls\\run16s.exe",package)>0);
    CHECK(swprintf_s(dll32,MAX_PATH,L"%ls\\ntw16s32.dll",package)>0);
    CHECK(swprintf_s(dll64,MAX_PATH,L"%ls\\ntw16s64.dll",package)>0);
    CHECK(CopyFileW(argv[1],launcher,FALSE) && CopyFileW(argv[2],dll32,FALSE) && CopyFileW(argv[3],dll64,FALSE));
    for(unsigned i=0;i<2;++i){CHECK(swprintf_s(targets[i],MAX_PATH,L"%ls\\目标 程序%u.exe",scratch,i?64:32)>0);
        CHECK(CopyFileW(argv[4+i],targets[i],FALSE));}
    for(unsigned width=0;width<2;++width)for(unsigned styled=0;styled<2;++styled){
        /* All argument encodings are intentional CRT vectors. */
        if(styled)CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\" styled \"two words\" \"quote\\\"inside\" \"trailing\\\\\" \"\" \"中文 αβ\"",launcher,targets[width])>0);
        else CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" native \"two words\" \"quote\\\"inside\" \"trailing\\\\\" \"\" \"中文 αβ\"",targets[width])>0);
        if(argc==7)CHECK(!wcscat_s(command,ARRAYSIZE(command),L" ") && !wcscat_s(command,ARRAYSIZE(command),argv[6]));
        CHECK(CreateProcessW(styled?launcher:targets[width],command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&startup,&child));
        DWORD wait=WaitForSingleObject(child.hProcess,120000);
        if(wait!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
        CHECK(wait==WAIT_OBJECT_0 && GetExitCodeProcess(child.hProcess,&code));
        CloseHandle(child.hThread);CloseHandle(child.hProcess);
        if(code)fprintf(stderr,"Target width=%u styled=%u suite=%u exit=%lu\n",width?64:32,styled,suite,code);
        CHECK(!code);
        {WCHAR path[MAX_PATH];FILE *file=NULL;char line[160];unsigned threads=0,complete=0,entries=0,apis=0,observations=0;
            CHECK(swprintf_s(path,MAX_PATH,L"%ls\\target-%u-%u-%u.txt",scratch,width?64:32,styled,suite)>0);
            CHECK(!_wfopen_s(&file,path,L"rt"));
            while(fgets(line,sizeof(line),file)){threads+=strstr(line,"GUI-thread-complete")!=NULL;
                complete+=strstr(line,"target-complete")!=NULL;entries+=strstr(line,"target-entry")!=NULL;
                apis+=strstr(line,"APIs-complete")!=NULL;observations+=strstr(line,"dialog-observed")!=NULL;}
            fclose(file);CHECK(entries==1 && complete==1 && threads==2);
            CHECK(apis==(suite?2u:0u) && observations==(suite==1?8u:suite==2?12u:0u));}
    }
    puts("LAUNCH MATRIX PASS actual x64 CLI -> x86/x64 plus native controls on private desktop");return 0;
}
int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,matrix);}
