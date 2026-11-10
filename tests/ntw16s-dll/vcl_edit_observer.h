#ifndef TEST_VCL_EDIT_OBSERVER_H
#define TEST_VCL_EDIT_OBSERVER_H

typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef BOOL (__cdecl *window_fn)(HWND);
static window_fn remove_style;
static HWND observed;
static BOOL remove_on_color,removed;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_CTLCOLOREDIT && (HWND)l==observed && remove_on_color){remove_on_color=FALSE;removed=remove_style(observed);}
    return DefWindowProcW(w,m,p,l);
}

static int check_edit_input(HWND edit)
{
    CHECK(SendMessageW(edit,WM_SETTEXT,0,(LPARAM)L"probe"));
    SendMessageW(edit,EM_SETSEL,1,3);SendMessageW(edit,WM_CHAR,L'X',1);
    WCHAR text[32];CHECK(GetWindowTextW(edit,text,32)>0 && !wcscmp(text,L"pXbe"));
    CHECK(SendMessageW(edit,EM_SETREADONLY,TRUE,0));SendMessageW(edit,WM_CHAR,L'Y',1);
    CHECK(GetWindowTextW(edit,text,32)>0 && !wcscmp(text,L"pXbe"));CHECK(SendMessageW(edit,EM_SETREADONLY,FALSE,0));
    return 0;
}

#endif
