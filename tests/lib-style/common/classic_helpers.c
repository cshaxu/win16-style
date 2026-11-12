#include "lib-style/common/classic_file_helpers.h"
#include <dlgs.h>
int main(void)
{
    WCHAR root[MAX_PATH],out[MAX_PATH];
    if(style_classic_file_folder(NULL,out,MAX_PATH)) return 1;
    if(!GetTempPathW(MAX_PATH,root) || root[1]!=L':' || root[2]!=L'\\') return 2;
    root[3]=0;
    HWND parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent) return 3;
    HWND list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|LBS_HASSTRINGS,0,0,40,40,parent,(HMENU)(INT_PTR)lst2,GetModuleHandleW(NULL),NULL);
    if(!list){DestroyWindow(parent);return 4;}
    SendMessageW(list,LB_ADDSTRING,0,(LPARAM)root);SendMessageW(list,LB_SETCURSEL,0,0);
    BOOL ok=style_classic_file_folder(parent,out,MAX_PATH) && !lstrcmpiW(out,root) &&
        !style_classic_file_folder(parent,out,3);
    DestroyWindow(parent);return ok?0:5;
}
