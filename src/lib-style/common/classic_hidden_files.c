#include "lib-style/common/classic_hidden_files.h"
#include <dlgs.h>
#include <wchar.h>
#include <string.h>

/* Experimental: refresh only the file list, never the process directory.
 * Folder navigation and full Shell filtering are not yet implemented. */
void style_classic_hidden_files(HWND w,WCHAR **filter,BOOL (*folder_path)(HWND,PWSTR,size_t))
{
    HWND files=GetDlgItem(w,lst1),filters=GetDlgItem(w,cmb1);WCHAR folder[MAX_PATH];
    if(!files || !filters || !folder_path(w,folder,ARRAYSIZE(folder)))return;
    LRESULT chosen=SendMessageW(filters,CB_GETCURSEL,0,0);if(chosen<0)return;
    PCWSTR spec=L"*.*";
    if(*filter){PCWSTR at=*filter;
        for(LRESULT i=0;i<chosen && *at;++i){at+=wcslen(at)+1;if(!*at)return;at+=wcslen(at)+1;}
        if(!*at)return;
        at+=wcslen(at)+1;
        if(!*at)return;
        spec=at;}
    while(*spec){PCWSTR end=wcschr(spec,L';');size_t n=end?(size_t)(end-spec):wcslen(spec);
        WCHAR pattern[32768];size_t dir=wcslen(folder);
        if(n && dir+1+n+1<=ARRAYSIZE(pattern)){memcpy(pattern,folder,dir*sizeof(WCHAR));
            if(dir && pattern[dir-1]!=L'\\')pattern[dir++]=L'\\';
            memcpy(pattern+dir,spec,n*sizeof(WCHAR));pattern[dir+n]=0;
            WIN32_FIND_DATAW data;HANDLE search=FindFirstFileW(pattern,&data);
            if(search!=INVALID_HANDLE_VALUE){do{
                if((data.dwFileAttributes&(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)) &&
                   !(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) &&
                   SendMessageW(files,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)data.cFileName)==LB_ERR)
                    (void)SendMessageW(files,LB_ADDSTRING,0,(LPARAM)data.cFileName);
            }while(FindNextFileW(search,&data));FindClose(search);}}
        if(!end)break;
        spec=end+1;
    }
}
