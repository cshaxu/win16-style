#include <windows.h>
#include <uxtheme.h>
#include <stdio.h>
#include <wchar.h>

int wmain(int argc,WCHAR **argv)
{
    if(argc>2 || (argc==2 && wcscmp(argv[1],L"--disabled")))return 4;
    BOOL expect_disabled=argc==2;
    if(!IsThemeActive()){puts("NTW2KS CONTROL SKIP theme-inactive");return 77;}
    HWND parent=CreateWindowExW(0,L"STATIC",L"probe",WS_OVERLAPPED,0,0,160,80,NULL,NULL,NULL,NULL);
    if(!parent)return 1;
    HWND button=CreateWindowExW(0,L"BUTTON",L"probe",WS_CHILD,0,0,80,24,parent,NULL,NULL,NULL);
    if(!button){DestroyWindow(parent);return 2;}
    HTHEME theme=OpenThemeData(button,L"BUTTON");
    printf("NTW2KS CONTROL theme=%s\n",theme?"active":"disabled");
    if(theme)CloseThemeData(theme);
    DestroyWindow(parent);return (!!theme)==(!expect_disabled)?0:3;
}
