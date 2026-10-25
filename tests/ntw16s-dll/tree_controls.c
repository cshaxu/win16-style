#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <imm.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TREE_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
#include "system_glyph_mask.h"
static HWND tree;
static HFONT appfont;
static unsigned mode,pre,items,trace,selections,expansions;
static BOOL cancel_selection,cancel_expansion,remove_in_draw,destroy_in_draw;
static BOOL (__cdecl *attached)(HWND),(__cdecl *remove_style)(HWND);
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_APP+17){DestroyWindow((HWND)l);return 0;}
    if(m==WM_NOTIFY && l){NMHDR *header=(NMHDR*)l;
        if(header->hwndFrom==tree){
            if(header->code==TVN_SELCHANGINGW || header->code==TVN_SELCHANGINGA){++selections;if(cancel_selection)return TRUE;}
            if(header->code==TVN_ITEMEXPANDINGW || header->code==TVN_ITEMEXPANDINGA){++expansions;if(cancel_expansion)return TRUE;}
            if(header->code==NM_CUSTOMDRAW){NMTVCUSTOMDRAW *cd=(NMTVCUSTOMDRAW*)l;
                if(cd->nmcd.dwDrawStage==CDDS_PREPAINT){
                    if(remove_in_draw){remove_in_draw=FALSE;remove_style(tree);}
                    if(destroy_in_draw){destroy_in_draw=FALSE;PostMessageW(w,WM_APP+17,0,(LPARAM)tree);return CDRF_SKIPDEFAULT;}
                    ++pre;trace=(trace^CDDS_PREPAINT)*16777619u;return mode?CDRF_NOTIFYITEMDRAW:CDRF_DODEFAULT;}
                if(cd->nmcd.dwDrawStage==CDDS_ITEMPREPAINT){unsigned long long id=cd->nmcd.dwItemSpec;++items;trace=(trace^(unsigned)(id^(id>>32)))*16777619u;
                    SelectObject(cd->nmcd.hdc,appfont);cd->clrText=0;cd->clrTextBk=RGB(238,238,238);
                    if(mode==2){RECT sentinel={400,2,406,8};FillRect(cd->nmcd.hdc,&sentinel,GetStockObject(BLACK_BRUSH));TextOutA(cd->nmcd.hdc,410,2,"application",11);return CDRF_SKIPDEFAULT;}
                    return CDRF_NEWFONT;
                }
            }
        }
    }
    return DefWindowProcW(w,m,p,l);
}
static DWORD styled_pixels[600*180],native_pixels[600*180],styled_frame[600*180],native_frame[600*180];
static HDC surface;static HBITMAP bitmap;static HGDIOBJ original_bitmap;static DWORD *surface_bits;
static void capture(HWND w,DWORD *out,BOOL frame)
{
    if(!surface){BITMAPINFO info={0};surface=CreateCompatibleDC(NULL);info.bmiHeader.biSize=40;info.bmiHeader.biWidth=600;info.bmiHeader.biHeight=-180;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
        bitmap=CreateDIBSection(surface,&info,DIB_RGB_COLORS,(void**)&surface_bits,NULL,0);if(!surface||!bitmap)ExitProcess(2);original_bitmap=SelectObject(surface,bitmap);}
    int saved=SaveDC(surface);RECT clear={0,0,600,180};FillRect(surface,&clear,GetStockObject(WHITE_BRUSH));pre=items=0;trace=2166136261u;
    SendMessageW(w,frame?WM_PRINT:WM_PRINTCLIENT,(WPARAM)surface,PRF_CLIENT|PRF_ERASEBKGND|(frame?PRF_NONCLIENT:0));GdiFlush();CopyMemory(out,surface_bits,sizeof(styled_pixels));RestoreDC(surface,saved);
}
static RECT item_rect(HWND w,HTREEITEM item,BOOL text)
{RECT r={0};CopyMemory(&r,&item,sizeof(item));if(!SendMessageW(w,TVM_GETITEMRECT,text,(LPARAM)&r))ExitProcess(3);return r;}
static int child(BOOL styled,BOOL aware)
{
    setvbuf(stdout,NULL,_IONBF,0);
    /* Thread-local ASCII fixture, not an IME acceptance test. Native no-DLL
     * control proved MSCTF monitor waits during process teardown. */
    CHECK(ImmDisableIME(0));
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_TREEVIEW_CLASSES};CHECK(InitCommonControlsEx(&init));
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");attached=dll?(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;remove_style=dll?(void*)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    CHECK(styled?(dll&&attached&&remove_style):!dll);
    WNDCLASSW parent={.lpfnWndProc=parent_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"TreeProtocolParent"};CHECK(RegisterClassW(&parent));
    HWND root=CreateWindowW(parent.lpszClassName,L"tree protocol",WS_OVERLAPPEDWINDOW,0,0,650,240,NULL,NULL,parent.hInstance,NULL);CHECK(root);
    ShowWindow(root,SW_SHOWNORMAL);
    const WCHAR *names[]={WC_TREEVIEWW,L"TTreeView",L"TShellTreeView"};
    for(unsigned k=1;k<3;++k){WNDCLASSEXW alias={.cbSize=sizeof(alias)};CHECK(GetClassInfoExW(NULL,WC_TREEVIEWW,&alias));alias.hInstance=parent.hInstance;alias.lpszClassName=names[k];alias.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&alias));}
    for(unsigned kind=0;kind<3;++kind)for(unsigned small=0;small<2;++small)for(mode=0;mode<3;++mode){
        printf("TREE_BEGIN kind=%u small=%u draw=%u styled=%u\n",kind,small,mode,styled);
        tree=CreateWindowExW(WS_EX_CLIENTEDGE,names[kind],L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT,0,0,600,180,root,(HMENU)41,parent.hInstance,NULL);CHECK(tree);if(styled)CHECK(attached(tree));
        HFONT first=CreateFontW(small?-11:-16,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,NONANTIALIASED_QUALITY,0,small?L"MS Sans Serif":L"Arial");CHECK(first);appfont=first;SendMessageW(tree,WM_SETFONT,(WPARAM)appfont,FALSE);
        SendMessageW(tree,TVM_SETBKCOLOR,0,RGB(255,255,255));SendMessageW(tree,TVM_SETTEXTCOLOR,0,0);
        HIMAGELIST images=ImageList_Create(16,16,ILC_COLOR32,1,1);CHECK(images);HDC dc=CreateCompatibleDC(NULL);HBITMAP icon=CreateBitmap(16,16,1,32,NULL);CHECK(dc&&icon);HGDIOBJ ib=SelectObject(dc,icon);RECT ir={0,0,16,16};HBRUSH brush=CreateSolidBrush(RGB(31,127,219));CHECK(brush);FillRect(dc,&ir,brush);DeleteObject(brush);SelectObject(dc,ib);DeleteDC(dc);CHECK(ImageList_Add(images,icon,NULL)==0);DeleteObject(icon);SendMessageW(tree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)images);
        LPARAM cookie=(LPARAM)(UINT_PTR)(sizeof(void*)==8?0x123456789ULL:0x1234567u);
        TVINSERTSTRUCTW insert={.hParent=TVI_ROOT,.hInsertAfter=TVI_LAST,.item={.mask=TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE,.pszText=L"E8M0M0E8",.lParam=cookie,.iImage=0,.iSelectedImage=0}};
        HTREEITEM first_item=(HTREEITEM)SendMessageW(tree,TVM_INSERTITEMW,0,(LPARAM)&insert),children[31];CHECK(first_item);insert.hParent=first_item;
        for(unsigned i=0;i<31;++i){insert.item.lParam=cookie+(LPARAM)i+1;children[i]=(HTREEITEM)SendMessageW(tree,TVM_INSERTITEMW,0,(LPARAM)&insert);CHECK(children[i]);}
        /* TVIS_EXPANDEDONCE suppresses subsequent TVM_EXPAND notifications.
         * Test cancellation on the first expansion, as the native API defines. */
        unsigned exp=expansions;cancel_expansion=TRUE;
        LRESULT denied=SendMessageW(tree,TVM_EXPAND,TVE_EXPAND,(LPARAM)first_item);cancel_expansion=FALSE;
        CHECK(!denied && expansions==exp+1 && !(SendMessageW(tree,TVM_GETITEMSTATE,(WPARAM)first_item,TVIS_EXPANDED)&TVIS_EXPANDED));
        CHECK(SendMessageW(tree,TVM_EXPAND,TVE_EXPAND,(LPARAM)first_item));UpdateWindow(root);SetFocus(tree);
        SendMessageW(tree,TVM_SELECTITEM,TVGN_CARET,(LPARAM)first_item);SendMessageW(tree,WM_KEYDOWN,VK_DOWN,1);SendMessageW(tree,WM_KEYUP,VK_DOWN,1);CHECK((HTREEITEM)SendMessageW(tree,TVM_GETNEXTITEM,TVGN_CARET,0)==children[0]);
        unsigned sel=selections;cancel_selection=TRUE;CHECK(!SendMessageW(tree,TVM_SELECTITEM,TVGN_CARET,(LPARAM)children[1]));cancel_selection=FALSE;CHECK(selections==sel+1 && (HTREEITEM)SendMessageW(tree,TVM_GETNEXTITEM,TVGN_CARET,0)==children[0]);
        CHECK(SendMessageW(tree,TVM_EXPAND,TVE_COLLAPSE,(LPARAM)first_item));CHECK(SendMessageW(tree,TVM_EXPAND,TVE_EXPAND,(LPARAM)first_item));
        SendMessageW(tree,TVM_SELECTITEM,TVGN_FIRSTVISIBLE,(LPARAM)first_item);SendMessageW(tree,WM_VSCROLL,SB_LINEDOWN,0);CHECK((HTREEITEM)SendMessageW(tree,TVM_GETNEXTITEM,TVGN_FIRSTVISIBLE,0)!=first_item);
        SendMessageW(tree,TVM_SELECTITEM,TVGN_FIRSTVISIBLE,(LPARAM)first_item);SendMessageW(tree,TVM_SELECTITEM,TVGN_CARET,0);SetFocus(root);
        puts("TREE_STAGE input_ok");capture(tree,styled_pixels,FALSE);RECT label=item_rect(tree,first_item,TRUE);if(styled && mode!=2)CHECK(system_mask(styled_pixels,&label,appfont));
        HFONT updated=CreateFontW(small?-13:-14,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Tahoma");CHECK(updated);appfont=updated;SendMessageW(tree,WM_SETFONT,(WPARAM)appfont,TRUE);
        RECT before,after;GetClientRect(tree,&before);LRESULT height=SendMessageW(tree,TVM_GETITEMHEIGHT,0,0);capture(tree,styled_pixels,FALSE);unsigned before_pre=pre,before_items=items,before_trace=trace;label=item_rect(tree,first_item,TRUE);
        CHECK(before_pre && (mode?before_items>0:!before_items));if(styled && mode!=2)CHECK(system_mask(styled_pixels,&label,appfont));capture(tree,styled_frame,TRUE);
        if(styled){CHECK(remove_style(tree)&&!attached(tree));}
        capture(tree,native_pixels,FALSE);CHECK(pre==before_pre&&items==before_items&&trace==before_trace);capture(tree,native_frame,TRUE);GetClientRect(tree,&after);CHECK(EqualRect(&before,&after) && SendMessageW(tree,TVM_GETITEMHEIGHT,0,0)==height);
        unsigned changed=0;for(int y=label.top;y<label.bottom;++y)for(int x=label.left;x<label.right;++x)changed+=(styled_pixels[y*600+x]&0xffffff)!=(native_pixels[y*600+x]&0xffffff);
        CHECK(styled&&mode!=2?changed>10:changed==0);
        unsigned nc=0;for(int y=0;y<180;++y)for(int x=0;x<600;++x)if(x<2||y<2)nc+=(styled_frame[y*600+x]&0xffffff)!=(native_frame[y*600+x]&0xffffff);CHECK(styled?nc>0:nc==0);
        TVITEMW get={.mask=TVIF_PARAM,.hItem=first_item};CHECK(SendMessageW(tree,TVM_GETITEMW,0,(LPARAM)&get)&&get.lParam==cookie);CHECK(SendMessageW(tree,TVM_GETCOUNT,0,0)==32 && (HFONT)SendMessageW(tree,WM_GETFONT,0,0)==appfont && (HIMAGELIST)SendMessageW(tree,TVM_GETIMAGELIST,TVSIL_NORMAL,0)==images && ImageList_GetImageCount(images)==1);
        printf("TREE_VARIANT kind=%u small=%u draw=%u styled=%u glyph=%u nc=%u native_geometry_data_font_images=1\n",kind,small,mode,styled,changed,nc);
        CHECK(DestroyWindow(tree)&&ImageList_GetImageCount(images)==1&&ImageList_Destroy(images)&&DeleteObject(updated)&&DeleteObject(first));
    }
    if(styled){
        mode=0;tree=CreateWindowExW(WS_EX_CLIENTEDGE,WC_TREEVIEWW,L"",WS_CHILD|WS_VISIBLE,0,0,600,180,root,(HMENU)42,parent.hInstance,NULL);
        CHECK(tree && attached(tree));remove_in_draw=TRUE;capture(tree,styled_pixels,FALSE);CHECK(!remove_in_draw && !attached(tree));CHECK(DestroyWindow(tree));
        tree=CreateWindowExW(WS_EX_CLIENTEDGE,WC_TREEVIEWW,L"",WS_CHILD|WS_VISIBLE,0,0,600,180,root,(HMENU)43,parent.hInstance,NULL);
        CHECK(tree && attached(tree));destroy_in_draw=TRUE;capture(tree,styled_pixels,FALSE);MSG queued;while(PeekMessageW(&queued,NULL,0,0,PM_REMOVE)){TranslateMessage(&queued);DispatchMessageW(&queued);}
        CHECK(!destroy_in_draw && !IsWindow(tree));
        tree=CreateWindowExW(WS_EX_CLIENTEDGE|WS_EX_LAYOUTRTL,WC_TREEVIEWW,L"",WS_CHILD,0,0,600,180,root,(HMENU)44,parent.hInstance,NULL);
        CHECK(tree && !attached(tree));CHECK(DestroyWindow(tree));
        for(unsigned k=1;k<3;++k){CHECK(UnregisterClassW(names[k],parent.hInstance));WNDCLASSW fake={.lpfnWndProc=DefWindowProcW,.hInstance=parent.hInstance,.lpszClassName=names[k],.cbWndExtra=sizeof(void*)};CHECK(RegisterClassW(&fake));
            tree=CreateWindowExW(WS_EX_CLIENTEDGE,names[k],L"",WS_CHILD,0,0,600,180,root,(HMENU)45,parent.hInstance,NULL);CHECK(tree && !attached(tree));CHECK(DestroyWindow(tree));}
        puts("TREE_CALLBACK_RETIRE_OK remove=1 deferred_destroy=1 reject_rtl_fake=1");
    }
    printf("TREE_CONTROL_OK bits=%u styled=%u aware=%u dpi=%u variants=18 keyboard=1 cancel=1 image_font_leases=preserved\n",(unsigned)sizeof(void*)*8,styled,aware,GetDpiForWindow(root));CHECK(DestroyWindow(root));puts("TREE_ROOT_DESTROYED");
    CHECK(SelectObject(surface,original_bitmap)&&DeleteObject(bitmap)&&DeleteDC(surface));surface=NULL;puts("TREE_SURFACE_FREED");return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned styled=0;styled<2;++styled){
        if(styled)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",argv[1],self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"native-aware":L"native");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        CHECK(AssignProcessToJobObject(job,pi.hProcess));ResumeThread(pi.hThread);DWORD started=GetTickCount(),code=1,wait=WaitForSingleObject(pi.hProcess,10000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        printf("TREE_WORKER aware=%u styled=%u elapsed_ms=%lu wait=%lu exit=%lu\n",aware,styled,GetTickCount()-started,wait,code);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    puts("TREE_CLEANUP_OK");return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcsncmp(argv[2],L"styled",6),wcsstr(argv[2],L"aware")!=NULL);return isolated_process(argc,argv,worker);}
