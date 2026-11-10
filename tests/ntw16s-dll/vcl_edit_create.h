#ifndef TEST_VCL_EDIT_CREATE_H
#define TEST_VCL_EDIT_CREATE_H

__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL multiline)
{
    DWORD style=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL;
#ifdef T18_MEMO
    multiline=TRUE;
#endif
    if(multiline)style|=ES_MULTILINE|WS_VSCROLL|WS_HSCROLL;
    HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,edit_class,L"probe",style,20,20,220,multiline?70:28,parent,NULL,owner,NULL);
    if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=GetWindowLongPtrW(w,GWL_STYLE);
        cs.dwExStyle=GetWindowLongPtrW(w,GWL_EXSTYLE);cs.lpszClass=edit_class;cs.lpszName=L"probe";cs.cx=220;cs.cy=28;
        enabled=TRUE;CallWindowProcW(native_edit,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_edit,w,WM_CREATE,0,(LPARAM)&cs);}
#ifdef T18_MEMO
    if(w)SendMessageW(w,EM_SETLIMITTEXT,0,0);
#endif
    return w;
}
#endif
