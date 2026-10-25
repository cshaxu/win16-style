#include "ntw16s-dll/attachment.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTROL_VISIBILITY_FAIL line=%u type=%u error=%lu: %s\n",__LINE__,kind,GetLastError(),#x);return 1;}}while(0)
static unsigned kind;
static BOOL visible(HWND w){return (GetWindowLongPtrW(w,GWL_STYLE)&WS_VISIBLE)!=0;}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2 || argc==3);unsigned first=0,last=4;
    if(argc==3){WCHAR *tail=NULL;unsigned long selected=wcstoul(argv[2],&tail,10);CHECK(*argv[2] && tail && !*tail && selected<4);first=(unsigned)selected;last=first+1;}
    HMODULE dll=LoadLibraryW(argv[1]);CHECK(dll);
    ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void*)GetProcAddress(dll,"Ntw16sCreationBegin");
    BOOL (__cdecl *end)(ntw16s_creation_scope*,ntw16s_creation_result*)=(void*)GetProcAddress(dll,"Ntw16sCreationEnd");
    BOOL (__cdecl *attached)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");
    BOOL (__cdecl *remove_style)(HWND)=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");
    CHECK(begin && end && attached && remove_style);
    static const WCHAR *classes[]={L"BUTTON",L"COMBOBOX",L"LISTBOX",L"EDIT"};
    static const DWORD styles[]={BS_AUTOCHECKBOX,CBS_DROPDOWNLIST,LBS_NOTIFY,ES_MULTILINE|WS_VSCROLL|WS_HSCROLL};
    unsigned cases=0;HFONT font=GetStockObject(DEFAULT_GUI_FONT);
    for(unsigned pv=0;pv<2;++pv)for(unsigned cv=0;cv<2;++cv){
        HWND parent=CreateWindowW(L"STATIC",L"visibility owner",WS_POPUP,0,0,320,240,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
        ShowWindow(parent,SW_HIDE);if(pv)ShowWindow(parent,SW_SHOW);CHECK(visible(parent)==(BOOL)pv);
        for(kind=first;kind<last;++kind){
            ntw16s_creation_scope *scope=begin(TRUE);CHECK(scope);
            HWND w=CreateWindowW(classes[kind],L"initial",WS_CHILD|WS_BORDER|styles[kind]|(cv?WS_VISIBLE:0),10,10,180,70,parent,NULL,GetModuleHandleW(NULL),NULL);
            ntw16s_creation_result result={0};CHECK(end(scope,&result) && !result.failure && w && attached(w));
            CHECK(visible(w)==(BOOL)cv && IsWindowVisible(w)==(pv && cv));
            for(unsigned phase=0;phase<2;++phase){
                BOOL expected=phase?FALSE:(BOOL)cv;
                if(phase)ShowWindow(w,SW_HIDE);
                SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);CHECK(visible(w)==expected);
                EnableWindow(w,FALSE);CHECK(visible(w)==expected);EnableWindow(w,TRUE);CHECK(visible(w)==expected);
                SendMessageW(w,WM_SETTEXT,0,(LPARAM)L"application text");CHECK(visible(w)==expected);
                if(kind==0){SendMessageW(w,BM_SETCHECK,BST_CHECKED,0);CHECK(visible(w)==expected);SendMessageW(w,BM_SETSTATE,TRUE,0);CHECK(visible(w)==expected);SendMessageW(w,BM_SETSTATE,FALSE,0);}
                if(kind==1){SendMessageW(w,CB_ADDSTRING,0,(LPARAM)L"first");SendMessageW(w,CB_SETCURSEL,0,0);}
                if(kind==2){SendMessageW(w,LB_ADDSTRING,0,(LPARAM)L"first");CHECK(visible(w)==expected);SendMessageW(w,LB_SETCURSEL,0,0);}
                if(kind==3){SendMessageW(w,EM_SETSEL,0,0);SendMessageW(w,EM_REPLACESEL,TRUE,(LPARAM)L"edited");}
                CHECK(visible(w)==expected && IsWindowVisible(w)==(pv && expected));
            }
            SendMessageW(w,WM_SETREDRAW,FALSE,0);CHECK(!visible(w));
            SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);EnableWindow(w,FALSE);EnableWindow(w,TRUE);CHECK(!visible(w));
            SendMessageW(w,WM_SETREDRAW,TRUE,0);ShowWindow(w,SW_HIDE);CHECK(!visible(w));
            WCHAR before[128],after[128];GetWindowTextW(w,before,128);CHECK(remove_style(w) && !attached(w) && !visible(w));
            GetWindowTextW(w,after,128);CHECK(!wcscmp(before,after));LOGFONTW lf;CHECK(GetObjectW(font,sizeof(lf),&lf));
            CHECK(DestroyWindow(w));++cases;
        }
        CHECK(visible(parent)==(BOOL)pv && DestroyWindow(parent));
    }
    CHECK(FreeLibrary(dll));printf("CONTROL_VISIBILITY_OK bits=%u cases=%u hidden_parent=1 hidden_child=1 native_redraw=1 detach=1 borrowed_font=1\n",(unsigned)(sizeof(void*)*8),cases);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
