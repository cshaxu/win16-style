/* Project-authored native control examples. No styling implementation lives here. */
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <richedit.h>
#include <stdio.h>
#include <wchar.h>

#define PAGE_COUNT 9
#define NOTE 2099
#define ITEM_LIMIT 80
BOOL demo_dialog_execute(HWND,unsigned);
BOOL demo_dialog_notify(HWND,UINT,LPARAM);
BOOL demo_dialog_message(MSG *);
void demo_dialog_close(HWND);
static const WCHAR *titles[PAGE_COUNT]={L"Buttons and states",L"Editing and lists",L"Files and scrollbars",L"Containers and drawing",L"Lists and toolbars",L"Ranges and dates",L"Text and special objects",L"MDI windows",L"Common dialogs"};
static HWND pages[PAGE_COUNT];
static HMODULE rich_library;
typedef struct item {HWND window;int x,y,cx,cy;} item;
typedef struct page {unsigned index,count,events,ticks,record;BOOL failed,running;HFONT font;HIMAGELIST images;HWND mdi,properties;WCHAR path[32768];item items[ITEM_LIMIT];} page;
static int px(HWND w,int n){return MulDiv(n,(int)GetDpiForWindow(w),96);}
static void note(HWND w,const WCHAR *text){SetDlgItemTextW(w,NOTE,text);}
static HWND add(HWND w,page *s,const WCHAR *cls,const WCHAR *text,DWORD style,DWORD ex,int x,int y,int cx,int cy,int id)
{
    if(s->count==ITEM_LIMIT){s->failed=TRUE;return NULL;}
    HWND child=CreateWindowExW(ex,cls,text,WS_CHILD|WS_VISIBLE|style,px(w,x),px(w,y),px(w,cx),px(w,cy),w,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL);
    if(!child){s->failed=TRUE;return NULL;}
    s->items[s->count++]=(item){child,x,y,cx,cy};
    SendMessageW(child,WM_SETFONT,(WPARAM)s->font,TRUE);return child;
}
static void label(HWND w,page *s,const WCHAR *text,int x,int y,int cx,int id)
{add(w,s,L"STATIC",text,SS_LEFT,0,x,y,cx,24,id);}
static void dialog_heading(HWND w,page *s,const WCHAR *text,int y,int id)
{add(w,s,L"STATIC",text,SS_LEFT,0,16,y,668,20,id);}
static void button(HWND w,page *s,const WCHAR *text,DWORD style,int x,int y,int cx,int id)
{add(w,s,L"BUTTON",text,style|WS_TABSTOP,0,x,y,cx,28,id);}
static void populate_list(HWND child,BOOL combo)
{for(unsigned i=0;i<8;++i){WCHAR text[40];swprintf_s(text,40,L"Item %u",i+1);SendMessageW(child,combo?CB_ADDSTRING:LB_ADDSTRING,0,(LPARAM)text);}}
static void relayout(HWND w,page *s)
{
    NONCLIENTMETRICSW n={.cbSize=sizeof(n)};HFONT next=NULL;
    if(SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(n),&n,0,GetDpiForWindow(w)))next=CreateFontIndirectW(&n.lfMessageFont);
    if(next){HFONT old=s->font;s->font=next;for(unsigned i=0;i<s->count;++i)if(IsWindow(s->items[i].window))SendMessageW(s->items[i].window,WM_SETFONT,(WPARAM)next,TRUE);if(old)DeleteObject(old);}
    for(unsigned i=0;i<s->count;++i){item *a=&s->items[i];if(GetParent(a->window)==w)SetWindowPos(a->window,NULL,px(w,a->x),px(w,a->y),px(w,a->cx),px(w,a->cy),SWP_NOACTIVATE|SWP_NOZORDER);}
    if(s->mdi){RECT r;GetClientRect(w,&r);SetWindowPos(s->mdi,NULL,0,0,r.right,r.bottom,SWP_NOACTIVATE|SWP_NOZORDER);}
}
static void files(HWND w,page *s,const WCHAR *path)
{
    WCHAR full[32768],pattern[32768];DWORD size=GetFullPathNameW(path,32768,full,NULL);
    if(!size || size>=32768 || GetFileAttributesW(full)==INVALID_FILE_ATTRIBUTES || !(GetFileAttributesW(full)&FILE_ATTRIBUTE_DIRECTORY)){note(w,L"Directory is unavailable.");return;}
    if(swprintf_s(pattern,32768,L"%ls%ls*",full,full[size-1]==L'\\'?L"":L"\\")<0){note(w,L"Path is too long.");return;}
    wcscpy_s(s->path,32768,full);SetDlgItemTextW(w,2300,full);SendDlgItemMessageW(w,2302,LB_RESETCONTENT,0,0);SendDlgItemMessageW(w,2303,LB_RESETCONTENT,0,0);
    WIN32_FIND_DATAW data;HANDLE find=FindFirstFileW(pattern,&data);unsigned count=0;
    if(find!=INVALID_HANDLE_VALUE){do{if(!wcscmp(data.cFileName,L"."))continue;SendDlgItemMessageW(w,(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?2302:2303,LB_ADDSTRING,0,(LPARAM)data.cFileName);++count;}while(count<128 && FindNextFileW(find,&data));FindClose(find);}
    note(w,count==128?L"Showing the first 128 entries; no files are opened or changed.":L"Choose a drive or double-click a directory. Files are listed read-only.");
}
static void buttons(HWND w,page *s)
{
    label(w,s,L"Command buttons",16,16,330,2100);button(w,s,L"&Default",BS_DEFPUSHBUTTON,16,48,160,2101);button(w,s,L"Disabled",BS_PUSHBUTTON|WS_DISABLED,192,48,160,2102);
    label(w,s,L"Checkbox values and alignment",16,100,600,2103);
    button(w,s,L"Checkbox",BS_AUTOCHECKBOX,16,132,200,2110);button(w,s,L"Initially checked",BS_AUTOCHECKBOX,240,132,200,2111);SendDlgItemMessageW(w,2111,BM_SETCHECK,BST_CHECKED,0);
    button(w,s,L"Three-state",BS_AUTO3STATE,16,168,200,2112);SendDlgItemMessageW(w,2112,BM_SETCHECK,BST_INDETERMINATE,0);
    button(w,s,L"Disabled checked",BS_AUTOCHECKBOX|WS_DISABLED,240,168,250,2113);SendDlgItemMessageW(w,2113,BM_SETCHECK,BST_CHECKED,0);
    button(w,s,L"Box on right",BS_AUTOCHECKBOX|BS_LEFTTEXT,16,204,200,2114);button(w,s,L"Push-like check",BS_AUTOCHECKBOX|BS_PUSHLIKE,240,204,200,2115);
    label(w,s,L"Independent radio groups",16,260,500,2120);
    button(w,s,L"Group A: first",BS_AUTORADIOBUTTON|WS_GROUP,16,292,230,2121);button(w,s,L"Group A: second",BS_AUTORADIOBUTTON,16,328,230,2122);
    button(w,s,L"Group B: first",BS_AUTORADIOBUTTON|WS_GROUP,300,292,230,2123);button(w,s,L"Group B: second",BS_AUTORADIOBUTTON,300,328,230,2124);
    SendDlgItemMessageW(w,2121,BM_SETCHECK,BST_CHECKED,0);SendDlgItemMessageW(w,2123,BM_SETCHECK,BST_CHECKED,0);
    HWND icon=add(w,s,L"BUTTON",L"",BS_ICON|WS_TABSTOP,0,16,392,64,48,2130);SendMessageW(icon,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconW(NULL,IDI_INFORMATION));
    label(w,s,L"Application picture button (shared Windows icon)",96,402,580,2131);
}
static void edits(HWND w,page *s)
{
    label(w,s,L"Text / read-only / password",16,16,650,2200);
    add(w,s,L"EDIT",L"Editable text",WS_TABSTOP|ES_AUTOHSCROLL,WS_EX_CLIENTEDGE,16,48,212,28,2201);
    add(w,s,L"EDIT",L"Read-only text",ES_READONLY,WS_EX_CLIENTEDGE,244,48,212,28,2202);
    add(w,s,L"EDIT",L"secret",ES_PASSWORD|WS_TABSTOP,WS_EX_CLIENTEDGE,472,48,212,28,2203);
    add(w,s,L"EDIT",L"Multiline text\r\nCopy, paste, select and edit.",ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|WS_TABSTOP,WS_EX_CLIENTEDGE,16,96,668,92,2204);
    label(w,s,L"Editable / simple / list-only combo boxes",16,204,660,2210);
    for(int i=0;i<3;++i){HWND c=add(w,s,L"COMBOBOX",L"",WS_TABSTOP|WS_VSCROLL|(i==0?CBS_DROPDOWN:i==1?CBS_SIMPLE:CBS_DROPDOWNLIST),0,16+i*228,236,212,134,2211+i);populate_list(c,TRUE);SendMessageW(c,CB_SETCURSEL,1,0);}
    label(w,s,L"Single selection / extended multiple selection",16,384,660,2220);
    for(int i=0;i<2;++i){HWND c=add(w,s,L"LISTBOX",L"",WS_TABSTOP|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|(i?LBS_EXTENDEDSEL:0),WS_EX_CLIENTEDGE,16+i*342,416,326,72,2221+i);populate_list(c,FALSE);}
}
static void file_controls(HWND w,page *s)
{
    add(w,s,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,WS_EX_CLIENTEDGE,16,16,540,28,2300);button(w,s,L"&Open path",BS_PUSHBUTTON,572,16,112,2301);
    add(w,s,L"LISTBOX",L"",LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_TABSTOP,WS_EX_CLIENTEDGE,16,96,326,300,2302);
    add(w,s,L"LISTBOX",L"",LBS_NOTIFY|LBS_EXTENDEDSEL|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_TABSTOP,WS_EX_CLIENTEDGE,358,96,326,300,2303);
    label(w,s,L"Directories",16,64,300,2304);label(w,s,L"Files",358,64,300,2305);
    HWND drive=add(w,s,L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_TABSTOP,0,16,416,140,180,2310);WCHAR drives[512];DWORD n=GetLogicalDriveStringsW(512,drives);if(n && n<512)for(WCHAR *d=drives;*d;d+=wcslen(d)+1)SendMessageW(drive,CB_ADDSTRING,0,(LPARAM)d);
    SendMessageW(drive,CB_SETCURSEL,0,0);
    HWND h=add(w,s,L"SCROLLBAR",L"",SBS_HORZ|WS_TABSTOP,0,180,422,470,17,2311);HWND v=add(w,s,L"SCROLLBAR",L"",SBS_VERT|WS_TABSTOP,0,666,416,17,72,2312);
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=30};SetScrollInfo(h,SB_CTL,&info,TRUE);SetScrollInfo(v,SB_CTL,&info,TRUE);
    if(!GetCurrentDirectoryW(32768,s->path))wcscpy_s(s->path,32768,L"C:\\");
    files(w,s,s->path);
}
static void draw_canvas(HWND w,HDC dc)
{
        RECT r;GetClientRect(w,&r);FillRect(dc,&r,(HBRUSH)(COLOR_WINDOW+1));
        HGDIOBJ pen=SelectObject(dc,GetStockObject(DC_PEN)),brush=SelectObject(dc,GetStockObject(DC_BRUSH));SetDCPenColor(dc,RGB(30,80,140));SetDCBrushColor(dc,RGB(180,210,240));
        Rectangle(dc,px(w,20),px(w,20),px(w,160),px(w,92));Ellipse(dc,px(w,190),px(w,20),px(w,330),px(w,92));MoveToEx(dc,px(w,20),px(w,116),NULL);LineTo(dc,px(w,500),px(w,160));
        for(int i=0;i<5;++i)Rectangle(dc,px(w,35+i*85),px(w,290-i*20),px(w,85+i*85),px(w,320));
        DrawIconEx(dc,px(w,550),px(w,20),LoadIconW(NULL,IDI_INFORMATION),px(w,32),px(w,32),0,NULL,DI_NORMAL);
        SelectObject(dc,brush);SelectObject(dc,pen);
}
static LRESULT CALLBACK canvas_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_PAINT){PAINTSTRUCT paint;HDC dc=BeginPaint(w,&paint);draw_canvas(w,dc);EndPaint(w,&paint);return 0;}
    if(m==WM_PRINTCLIENT){draw_canvas(w,(HDC)a);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static void containers(HWND w,page *s)
{
    add(w,s,L"BUTTON",L"Frame / nested controls",BS_GROUPBOX,0,16,16,668,108,2400);
    add(w,s,L"STATIC",L"Label with a mnemonic: &Name",SS_LEFT,0,32,44,300,24,2401);
    add(w,s,L"EDIT",L"Container child",WS_TABSTOP|ES_AUTOHSCROLL,WS_EX_CLIENTEDGE,344,44,320,28,2402);
    add(w,s,L"STATIC",L"Disabled label",WS_DISABLED,0,32,84,280,24,2403);
    label(w,s,L"Application-owned Image / Shape / Line / PictureBox content",16,140,668,2404);
    add(w,s,L"Win16StyleDemoCanvas",L"",0,WS_EX_CLIENTEDGE,16,172,668,330,2405);
}
static void common_lists(HWND w,page *s)
{
    HWND rebar=add(w,s,REBARCLASSNAMEW,L"",WS_CLIPCHILDREN|RBS_VARHEIGHT|RBS_AUTOSIZE|CCS_NODIVIDER,0,0,0,700,36,2500);
    HWND toolbar=CreateWindowExW(0,TOOLBARCLASSNAMEW,NULL,WS_CHILD|WS_VISIBLE|TBSTYLE_FLAT|TBSTYLE_LIST|CCS_NORESIZE|CCS_NOPARENTALIGN,0,0,0,0,rebar,(HMENU)2501,GetModuleHandleW(NULL),NULL);
    if(!toolbar){s->failed=TRUE;return;}
    SendMessageW(toolbar,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);TBBUTTON buttons[2]={{.iBitmap=I_IMAGENONE,.idCommand=2510,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_BUTTON|BTNS_AUTOSIZE,.iString=(INT_PTR)L"New"},{.iBitmap=I_IMAGENONE,.idCommand=2511,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_CHECK|BTNS_AUTOSIZE,.iString=(INT_PTR)L"Toggle"}};
    SendMessageW(toolbar,TB_ADDBUTTONSW,2,(LPARAM)buttons);SendMessageW(toolbar,TB_AUTOSIZE,0,0);
    REBARBANDINFOW band={.cbSize=sizeof(band),.fMask=RBBIM_CHILD|RBBIM_CHILDSIZE|RBBIM_SIZE|RBBIM_STYLE,.fStyle=RBBS_GRIPPERALWAYS,.hwndChild=toolbar,.cxMinChild=(UINT)px(w,200),.cyMinChild=(UINT)px(w,30),.cx=(UINT)px(w,600)};SendMessageW(rebar,RB_INSERTBANDW,(WPARAM)-1,(LPARAM)&band);
    s->images=ImageList_Create(px(w,16),px(w,16),ILC_COLOR32|ILC_MASK,1,1);if(!s->images){s->failed=TRUE;return;}ImageList_AddIcon(s->images,LoadIconW(NULL,IDI_APPLICATION));
    HWND tree=add(w,s,WC_TREEVIEWW,L"",WS_TABSTOP|TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT,WS_EX_CLIENTEDGE,16,56,210,256,2502);TreeView_SetImageList(tree,s->images,TVSIL_NORMAL);
    TVINSERTSTRUCTW item={.hParent=TVI_ROOT,.hInsertAfter=TVI_LAST,.item={.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE,.pszText=L"Root",.iImage=0,.iSelectedImage=0}};HTREEITEM root=TreeView_InsertItem(tree,&item);item.hParent=root;item.item.pszText=L"Child";TreeView_InsertItem(tree,&item);TreeView_Expand(tree,root,TVE_EXPAND);
    HWND list=add(w,s,WC_LISTVIEWW,L"",WS_TABSTOP|LVS_REPORT|LVS_SHAREIMAGELISTS,WS_EX_CLIENTEDGE,244,56,440,256,2503);ListView_SetImageList(list,s->images,LVSIL_SMALL);
    LVCOLUMNW column={.mask=LVCF_TEXT|LVCF_WIDTH,.cx=px(w,210),.pszText=L"Name"};ListView_InsertColumn(list,0,&column);column.pszText=L"Value";ListView_InsertColumn(list,1,&column);
    LVITEMW row={.mask=LVIF_TEXT|LVIF_IMAGE,.iItem=0,.pszText=L"First row",.iImage=0};ListView_InsertItem(list,&row);ListView_SetItemText(list,0,1,L"Native report cells");
    HWND tab=add(w,s,WC_TABCONTROLW,L"",WS_TABSTOP,0,16,340,668,146,2504);TCITEMW ti={.mask=TCIF_TEXT,.pszText=L"First tab"};TabCtrl_InsertItem(tab,0,&ti);ti.pszText=L"Second tab";TabCtrl_InsertItem(tab,1,&ti);
    add(w,s,STATUSCLASSNAMEW,L"ImageList, header and notifications are owned by native controls.",SBARS_SIZEGRIP|CCS_NOPARENTALIGN|CCS_NORESIZE,0,0,520,700,24,2505);
}
static void ranges(HWND w,page *s)
{
    label(w,s,L"Progress / slider / numeric up-down",16,16,660,2600);
    HWND progress=add(w,s,PROGRESS_CLASSW,L"",PBS_SMOOTH,0,16,48,500,24,2601);SendMessageW(progress,PBM_SETPOS,45,0);SendMessageW(progress,PBM_SETSTEP,5,0);button(w,s,L"&Step",BS_PUSHBUTTON,544,46,140,2602);
    HWND slider=add(w,s,TRACKBAR_CLASSW,L"",WS_TABSTOP|TBS_AUTOTICKS,0,16,92,500,48,2603);SendMessageW(slider,TBM_SETRANGE,TRUE,MAKELPARAM(0,100));SendMessageW(slider,TBM_SETPOS,TRUE,40);
    HWND edit=add(w,s,L"EDIT",L"25",WS_TABSTOP|ES_NUMBER|ES_AUTOHSCROLL,WS_EX_CLIENTEDGE,544,100,112,28,2604);HWND up=add(w,s,UPDOWN_CLASSW,L"",UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ARROWKEYS,0,656,100,28,28,2605);SendMessageW(up,UDM_SETBUDDY,(WPARAM)edit,0);SendMessageW(up,UDM_SETRANGE32,0,100);SendMessageW(up,UDM_SETPOS32,0,25);
    label(w,s,L"Date picker / month calendar",16,164,660,2610);add(w,s,DATETIMEPICK_CLASSW,L"",WS_TABSTOP|DTS_SHORTDATEFORMAT,0,16,196,260,28,2611);add(w,s,MONTHCAL_CLASSW,L"",WS_TABSTOP,0,310,196,374,272,2612);
    HWND combo=add(w,s,WC_COMBOBOXEXW,L"",CBS_DROPDOWNLIST|WS_TABSTOP,0,16,260,260,180,2613);COMBOBOXEXITEMW ci={.mask=CBEIF_TEXT,.iItem=0,.pszText=L"Native ImageCombo counterpart"};SendMessageW(combo,CBEM_INSERTITEMW,0,(LPARAM)&ci);SendMessageW(combo,CB_SETCURSEL,0,0);
}
static INT_PTR CALLBACK property_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    (void)a;(void)b;if(m==WM_INITDIALOG)return TRUE;
    if(m==WM_NOTIFY && ((NMHDR *)b)->code==PSN_APPLY){SetWindowLongPtrW(w,DWLP_MSGRESULT,PSNRET_NOERROR);return TRUE;}return FALSE;
}
static void properties(HWND w,page *s)
{
    if(s->properties && IsWindow(s->properties) && GetWindow(s->properties,GW_OWNER)==w){if(PropSheet_GetCurrentPageHwnd(s->properties)){SetForegroundWindow(s->properties);return;}DestroyWindow(s->properties);s->properties=NULL;}
    PROPSHEETPAGEW pagespec[2]={0};for(int i=0;i<2;++i){pagespec[i].dwSize=sizeof(PROPSHEETPAGEW);pagespec[i].hInstance=GetModuleHandleW(NULL);pagespec[i].pszTemplate=MAKEINTRESOURCEW(2010+i);pagespec[i].pfnDlgProc=property_proc;}
    PROPSHEETHEADERW header={.dwSize=sizeof(header),.dwFlags=PSH_PROPSHEETPAGE|PSH_MODELESS,.hwndParent=w,.hInstance=GetModuleHandleW(NULL),.pszCaption=L"Native property pages",.nPages=2,.ppsp=pagespec};s->properties=(HWND)PropertySheetW(&header);if(s->properties==(HWND)-1)s->properties=NULL;
}
static void special(HWND w,page *s)
{
    if(!rich_library)rich_library=LoadLibraryExW(L"Msftedit.dll",NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);
    label(w,s,L"Rich text (native system RichEdit)",16,16,660,2700);
    if(rich_library){HWND rich=add(w,s,MSFTEDIT_CLASS,L"Rich text: select, edit, copy and paste.",WS_TABSTOP|ES_MULTILINE|WS_VSCROLL,WS_EX_CLIENTEDGE,16,48,668,100,2701);CHARFORMAT2W format={.cbSize=sizeof(format),.dwMask=CFM_BOLD,.dwEffects=CFE_BOLD};SendMessageW(rich,EM_SETSEL,0,9);SendMessageW(rich,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&format);SendMessageW(rich,EM_SETSEL,0,0);}
    else label(w,s,L"System RichEdit is unavailable.",16,48,660,2701);
    label(w,s,L"Numeric edit / timer / in-memory record navigation",16,172,660,2710);add(w,s,L"EDIT",L"12345",ES_NUMBER|WS_TABSTOP,WS_EX_CLIENTEDGE,16,204,130,28,2711);button(w,s,L"&Start timer",BS_PUSHBUTTON,170,204,170,2712);label(w,s,L"Ticks: 0",360,208,300,2713);
    button(w,s,L"&Previous",BS_PUSHBUTTON,16,256,130,2720);button(w,s,L"&Next",BS_PUSHBUTTON,170,256,130,2721);label(w,s,L"Record 1: Alpha",330,260,330,2722);
    button(w,s,L"Open &file...",BS_PUSHBUTTON,16,316,180,2730);button(w,s,L"Property &pages...",BS_PUSHBUTTON,220,316,220,2731);
    label(w,s,L"OLE / UserControl hosts retain the embedded application's contents.",16,384,668,2740);
    label(w,s,L"Charts and image controls retain application-owned pictures and drawing.",16,420,668,2741);
    label(w,s,L"Communication, binding and other nonvisual objects have no substitute UI.",16,456,668,2742);
}
static void dialog_examples(HWND w,page *s)
{
    dialog_heading(w,s,L"Open file",4,2890);
    button(w,s,L"Open: legacy",BS_PUSHBUTTON,16,26,212,2901);
    button(w,s,L"Open: Explorer API",BS_PUSHBUTTON,244,26,212,2924);
    button(w,s,L"Open: modern to classic",BS_PUSHBUTTON,472,26,212,2902);
    button(w,s,L"Open: modern retained",BS_PUSHBUTTON,16,56,212,2903);
    button(w,s,L"Multi-open: legacy",BS_PUSHBUTTON,244,56,212,2921);
    button(w,s,L"Multi-open: Explorer",BS_PUSHBUTTON,472,56,212,2922);
    button(w,s,L"Multi-open: COM",BS_PUSHBUTTON,16,86,212,2926);
    dialog_heading(w,s,L"Save file",116,2891);
    button(w,s,L"Save: legacy",BS_PUSHBUTTON,16,138,212,2904);
    button(w,s,L"Save: Explorer API",BS_PUSHBUTTON,244,138,212,2925);
    button(w,s,L"Save: modern to classic",BS_PUSHBUTTON,472,138,212,2905);
    button(w,s,L"Save: modern retained",BS_PUSHBUTTON,16,168,212,2906);
    dialog_heading(w,s,L"Choose folder",198,2892);
    button(w,s,L"Folder: legacy Shell",BS_PUSHBUTTON,16,220,212,2907);
    button(w,s,L"Folder: Shell bridge",BS_PUSHBUTTON,244,220,212,2920);
    button(w,s,L"Folder: modern to classic",BS_PUSHBUTTON,472,220,212,2908);
    button(w,s,L"Folder: modern retained",BS_PUSHBUTTON,16,250,212,2909);
    dialog_heading(w,s,L"Find, replace, font and color",280,2893);
    button(w,s,L"Find",BS_PUSHBUTTON,16,302,212,2910);
    button(w,s,L"Replace",BS_PUSHBUTTON,244,302,212,2911);
    button(w,s,L"Font",BS_PUSHBUTTON,472,302,212,2912);
    button(w,s,L"Color",BS_PUSHBUTTON,16,332,212,2913);
    dialog_heading(w,s,L"Print and page setup",362,2894);
    button(w,s,L"Print",BS_PUSHBUTTON,16,384,212,2914);
    button(w,s,L"Print setup: legacy",BS_PUSHBUTTON,244,384,212,2923);
    button(w,s,L"Print: extended",BS_PUSHBUTTON,472,384,212,2915);
    button(w,s,L"Page setup",BS_PUSHBUTTON,16,414,212,2916);
    dialog_heading(w,s,L"Messages and other dialogs",444,2895);
    button(w,s,L"Message",BS_PUSHBUTTON,16,466,160,2917);
    button(w,s,L"Task dialog",BS_PUSHBUTTON,184,466,160,2918);
    button(w,s,L"Property pages",BS_PUSHBUTTON,352,466,160,2919);
    button(w,s,L"Launch 16-bit target",BS_PUSHBUTTON,520,466,164,2927);
}
static LRESULT CALLBACK mdi_child(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefMDIChildProcW(w,m,a,b);}
static BOOL populate(HWND w,page *s)
{
    relayout(w,s);
    switch(s->index){case 0:buttons(w,s);break;case 1:edits(w,s);break;case 2:file_controls(w,s);break;case 3:containers(w,s);break;case 4:common_lists(w,s);break;case 5:ranges(w,s);break;case 6:special(w,s);break;case 7:{
        CLIENTCREATESTRUCT client={.idFirstChild=4000};s->mdi=CreateWindowExW(WS_EX_CLIENTEDGE,L"MDICLIENT",NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_VSCROLL|WS_HSCROLL,0,0,px(w,700),px(w,540),w,(HMENU)2800,GetModuleHandleW(NULL),&client);if(!s->mdi)return FALSE;
        for(int i=0;i<2;++i)if(!CreateMDIWindowW(L"Win16StyleDemoMDIChild",i?L"Second document":L"First document",WS_VISIBLE|WS_OVERLAPPEDWINDOW,px(w,20+i*140),px(w,20+i*110),px(w,360),px(w,220),s->mdi,GetModuleHandleW(NULL),0))return FALSE;
        break;}case 8:dialog_examples(w,s);break;}
    if(s->index!=7){label(w,s,L"Ready",16,s->index==4?494:510,668,NOTE);if(s->index==2)note(w,L"Directory listing only; double-click a directory to navigate.");}
    return !s->failed;
}
static LRESULT CALLBACK page_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    page *s=(page *)GetWindowLongPtrW(w,GWLP_USERDATA);
    if(m==WM_NCCREATE){CREATESTRUCTW *c=(CREATESTRUCTW *)b;s=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s));if(!s)return FALSE;s->index=(unsigned)(UINT_PTR)c->lpCreateParams;SetWindowLongPtrW(w,GWLP_USERDATA,(LONG_PTR)s);}
    if(!s)return DefWindowProcW(w,m,a,b);
    if(s->index==8 && demo_dialog_notify(w,m,b))return 0;
    if(m==WM_CREATE)return populate(w,s)?0:-1;
    if(m==WM_COMMAND){unsigned id=LOWORD(a);WCHAR text[400];
        if(s->index==8 && id==2919){properties(w,s);return 0;}
        if(s->index==8 && demo_dialog_execute(w,id))return 0;
        if(s->index==2 && id==2301){WCHAR path[32768];GetDlgItemTextW(w,2300,path,32768);files(w,s,path);}
        if(s->index==2 && id==2310 && HIWORD(a)==CBN_SELCHANGE){WCHAR drive[8];LRESULT n=SendDlgItemMessageW(w,2310,CB_GETCURSEL,0,0);if(n!=CB_ERR && SendDlgItemMessageW(w,2310,CB_GETLBTEXTLEN,n,0)<8 && SendDlgItemMessageW(w,2310,CB_GETLBTEXT,n,(LPARAM)drive)!=CB_ERR)files(w,s,drive);}
        if(s->index==2 && id==2302 && HIWORD(a)==LBN_DBLCLK){WCHAR name[260],path[32768];LRESULT n=SendDlgItemMessageW(w,2302,LB_GETCURSEL,0,0);if(n!=LB_ERR && SendDlgItemMessageW(w,2302,LB_GETTEXTLEN,n,0)<260 && SendDlgItemMessageW(w,2302,LB_GETTEXT,n,(LPARAM)name)!=LB_ERR && swprintf_s(path,32768,L"%ls\\%ls",s->path,name)>0)files(w,s,path);}
        if(id==2602)SendDlgItemMessageW(w,2601,PBM_STEPIT,0,0);
        if(id==2712){if(s->running){KillTimer(w,1);s->running=FALSE;}else s->running=SetTimer(w,1,500,NULL)!=0;SetDlgItemTextW(w,2712,s->running?L"&Stop timer":L"&Start timer");}
        if(id==2720 || id==2721){s->record=(s->record+(id==2721?1:2))%3;swprintf_s(text,400,L"Record %u: %ls",s->record+1,s->record==0?L"Alpha":s->record==1?L"Beta":L"Gamma");SetDlgItemTextW(w,2722,text);}
        if(id==2730){WCHAR file[32768]=L"";OPENFILENAMEW open={.lStructSize=sizeof(open),.hwndOwner=w,.lpstrFilter=L"All files\0*.*\0\0",.lpstrFile=file,.nMaxFile=32768,.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR};if(GetOpenFileNameW(&open)){note(w,file);return 0;}note(w,L"File selection cancelled; no file was opened.");return 0;}
        if(id==2731)properties(w,s);
        swprintf_s(text,400,L"Control %u: notification %u (event %u)",id,HIWORD(a),++s->events);note(w,text);return 0;
    }
    if(m==WM_NOTIFY){WCHAR text[160];NMHDR *n=(NMHDR *)b;swprintf_s(text,160,L"Native control %llu: notification %u (event %u)",(unsigned long long)n->idFrom,n->code,++s->events);note(w,text);return 0;}
    if(m==WM_TIMER && a==1){WCHAR text[80];swprintf_s(text,80,L"Ticks: %u",++s->ticks);SetDlgItemTextW(w,2713,text);return 0;}
    if((m==WM_HSCROLL || m==WM_VSCROLL) && s->index==2 && b){SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};if(GetScrollInfo((HWND)b,SB_CTL,&info)){switch(LOWORD(a)){case SB_LINELEFT:--info.nPos;break;case SB_LINERIGHT:++info.nPos;break;case SB_PAGELEFT:info.nPos-=(int)info.nPage;break;case SB_PAGERIGHT:info.nPos+=(int)info.nPage;break;case SB_THUMBTRACK:case SB_THUMBPOSITION:info.nPos=info.nTrackPos;break;case SB_LEFT:info.nPos=info.nMin;break;case SB_RIGHT:info.nPos=info.nMax;break;default:return 0;}info.fMask=SIF_POS;SetScrollInfo((HWND)b,SB_CTL,&info,TRUE);}return 0;}
    if(m==WM_DPICHANGED){RECT *rect=(RECT *)b;SetWindowPos(w,NULL,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,SWP_NOZORDER|SWP_NOACTIVATE);relayout(w,s);return 0;}
    if(m==WM_GETMINMAXINFO){RECT rect={0,0,px(w,700),px(w,548)};if(AdjustWindowRectExForDpi(&rect,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),FALSE,(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),GetDpiForWindow(w))){MINMAXINFO *limits=(MINMAXINFO *)b;limits->ptMinTrackSize.x=rect.right-rect.left;limits->ptMinTrackSize.y=rect.bottom-rect.top;}return 0;}
    if(m==WM_SIZE && s->mdi){RECT rect;GetClientRect(w,&rect);SetWindowPos(s->mdi,NULL,0,0,rect.right,rect.bottom,SWP_NOACTIVATE|SWP_NOZORDER);}
    if(m==WM_DESTROY){KillTimer(w,1);if(s->index==8)demo_dialog_close(w);}
    if(m==WM_NCDESTROY){LRESULT result=s->mdi?DefFrameProcW(w,s->mdi,m,a,b):DefWindowProcW(w,m,a,b);if(pages[s->index]==w)pages[s->index]=NULL;SetWindowLongPtrW(w,GWLP_USERDATA,0);if(s->images)ImageList_Destroy(s->images);if(s->font)DeleteObject(s->font);HeapFree(GetProcessHeap(),0,s);return result;}
    return s->mdi?DefFrameProcW(w,s->mdi,m,a,b):DefWindowProcW(w,m,a,b);
}
BOOL demo_catalog_register(HINSTANCE instance)
{
    INITCOMMONCONTROLSEX controls={.dwSize=sizeof(controls),.dwICC=ICC_WIN95_CLASSES|ICC_DATE_CLASSES|ICC_USEREX_CLASSES|ICC_COOL_CLASSES};if(!InitCommonControlsEx(&controls))return FALSE;
    WNDCLASSW cls={.lpfnWndProc=page_proc,.hInstance=instance,.hCursor=LoadCursorW(NULL,IDC_ARROW),.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1),.lpszClassName=L"Win16StyleDemoCatalog"};if(!RegisterClassW(&cls))return FALSE;
    cls.lpfnWndProc=canvas_proc;cls.lpszClassName=L"Win16StyleDemoCanvas";if(!RegisterClassW(&cls))return FALSE;cls.lpfnWndProc=mdi_child;cls.lpszClassName=L"Win16StyleDemoMDIChild";return RegisterClassW(&cls)!=0;
}
BOOL demo_catalog_menu(HMENU bar)
{
    HMENU menu=CreatePopupMenu();if(!menu)return FALSE;
    for(unsigned i=0;i<PAGE_COUNT;++i)if(!AppendMenuW(menu,MF_STRING,2000+i,titles[i])){DestroyMenu(menu);return FALSE;}
    if(!AppendMenuW(bar,MF_POPUP,(UINT_PTR)menu,L"&Controls")){DestroyMenu(menu);return FALSE;}return TRUE;
}
void demo_catalog_open(HWND owner,unsigned index)
{
    if(index>=PAGE_COUNT)return;
    if(pages[index]){ShowWindow(pages[index],SW_RESTORE);SetForegroundWindow(pages[index]);return;}
    HWND w=CreateWindowExW(0,L"Win16StyleDemoCatalog",titles[index],WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,700,540,owner,NULL,GetModuleHandleW(NULL),(void *)(UINT_PTR)index);
    if(!w){MessageBoxW(owner,L"The native controls page could not be created.",L"Controls",MB_OK|MB_ICONERROR);return;}
    pages[index]=w;RECT rect={0,0,px(w,700),px(w,548)};
    if(!AdjustWindowRectExForDpi(&rect,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,FALSE,0,GetDpiForWindow(w))){DestroyWindow(w);return;}
    SetWindowPos(w,NULL,0,0,rect.right-rect.left,rect.bottom-rect.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);ShowWindow(w,SW_SHOWNORMAL);UpdateWindow(w);
}
BOOL demo_catalog_message(MSG *message)
{
    if(demo_dialog_message(message))return TRUE;
    for(unsigned i=0;i<PAGE_COUNT;++i)if(pages[i]){page *s=(page *)GetWindowLongPtrW(pages[i],GWLP_USERDATA);if(s && s->mdi && TranslateMDISysAccel(s->mdi,message))return TRUE;if(s && s->properties && IsWindow(s->properties) && GetWindow(s->properties,GW_OWNER)==pages[i]){if(!PropSheet_GetCurrentPageHwnd(s->properties)){DestroyWindow(s->properties);s->properties=NULL;}else if(PropSheet_IsDialogMessage(s->properties,message)){if(!PropSheet_GetCurrentPageHwnd(s->properties)){DestroyWindow(s->properties);s->properties=NULL;}return TRUE;}}if(IsDialogMessageW(pages[i],message))return TRUE;}
    return FALSE;
}
void demo_catalog_dispatched(void)
{
    /* Modeless sheets signal completion during dispatch; retire before idle. */
    for(unsigned i=0;i<PAGE_COUNT;++i)if(pages[i]){page *s=(page *)GetWindowLongPtrW(pages[i],GWLP_USERDATA);if(s && s->properties && IsWindow(s->properties) && GetWindow(s->properties,GW_OWNER)==pages[i] && !PropSheet_GetCurrentPageHwnd(s->properties)){DestroyWindow(s->properties);s->properties=NULL;}}
}
void demo_catalog_close(void)
{
    for(unsigned i=0;i<PAGE_COUNT;++i)if(pages[i])DestroyWindow(pages[i]);
    if(rich_library){FreeLibrary(rich_library);rich_library=NULL;}
    HINSTANCE instance=GetModuleHandleW(NULL);UnregisterClassW(L"Win16StyleDemoMDIChild",instance);UnregisterClassW(L"Win16StyleDemoCanvas",instance);UnregisterClassW(L"Win16StyleDemoCatalog",instance);
}
