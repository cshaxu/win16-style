/* Adapted from the fixed gallery's native controls/menu/message examples.
 * Windows owns appearance, control data, notifications and input behavior. */
#include <windows.h>
#include <stdio.h>
BOOL demo_catalog_register(HINSTANCE);
BOOL demo_catalog_menu(HMENU);
void demo_catalog_open(HWND,unsigned);
BOOL demo_catalog_message(MSG *);
void demo_catalog_close(void);
void demo_catalog_dispatched(void);
enum {APPLY=100,EDIT_TEXT,CHECK_OPTION,RADIO_A,RADIO_B,LIST,COMBO,SCROLL,STATUS,
      LABEL,GROUP,RESET=1001,EXIT_APP,ABOUT};
static const struct {
    PCWSTR name,text;DWORD style;int x,y,width,height,id;
} controls[]={
    {L"STATIC",L"Text to apply:",SS_LEFT,20,20,180,28,LABEL},
    {L"EDIT",L"Native text",WS_BORDER|WS_TABSTOP|ES_AUTOHSCROLL,20,52,350,34,EDIT_TEXT},
    {L"BUTTON",L"&Apply",BS_DEFPUSHBUTTON|WS_TABSTOP,390,52,150,36,APPLY},
    {L"BUTTON",L"&Enabled",BS_AUTOCHECKBOX|WS_TABSTOP|WS_GROUP,20,104,230,32,CHECK_OPTION},
    {L"BUTTON",L"Selection",BS_GROUPBOX,280,100,260,100,GROUP},
    {L"BUTTON",L"Option &A",BS_AUTORADIOBUTTON|WS_TABSTOP|WS_GROUP,296,128,220,28,RADIO_A},
    {L"BUTTON",L"Option &B",BS_AUTORADIOBUTTON,296,164,220,28,RADIO_B},
    {L"LISTBOX",L"",WS_BORDER|WS_TABSTOP|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT,20,152,230,130,LIST},
    {L"COMBOBOX",L"",WS_TABSTOP|CBS_DROPDOWNLIST,280,220,260,160,COMBO},
    {L"SCROLLBAR",L"",WS_TABSTOP|SBS_HORZ,20,314,520,17,SCROLL},
    {L"STATIC",L"Ready",SS_LEFT,20,364,520,40,STATUS}
};
static void status(HWND window,PCWSTR text)
{SetDlgItemTextW(window,STATUS,text);}
static int scaled(HWND window,int value)
{return MulDiv(value,(int)GetDpiForWindow(window),96);}
static void layout(HWND window)
{
    for(unsigned i=0;i<ARRAYSIZE(controls);++i){
        HWND child=GetDlgItem(window,controls[i].id);
        if(child)SetWindowPos(child,NULL,scaled(window,controls[i].x),scaled(window,controls[i].y),
            scaled(window,controls[i].width),scaled(window,controls[i].height),SWP_NOZORDER|SWP_NOACTIVATE);
    }
}
static void reset(HWND window)
{
    SetDlgItemTextW(window,EDIT_TEXT,L"Native text");
    CheckDlgButton(window,CHECK_OPTION,BST_CHECKED);
    CheckRadioButton(window,RADIO_A,RADIO_B,RADIO_A);
    SendDlgItemMessageW(window,LIST,LB_SETCURSEL,1,0);
    SendDlgItemMessageW(window,COMBO,CB_SETCURSEL,1,0);
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=20,.nPos=30,.nTrackPos=0};
    SetScrollInfo(GetDlgItem(window,SCROLL),SB_CTL,&info,TRUE);status(window,L"Ready");
}
static BOOL populate(HWND window,HINSTANCE instance)
{
    for(unsigned i=0;i<ARRAYSIZE(controls);++i)
        if(!CreateWindowW(controls[i].name,controls[i].text,WS_CHILD|WS_VISIBLE|controls[i].style,
            0,0,1,1,window,(HMENU)(INT_PTR)controls[i].id,instance,NULL))return FALSE;
    for(int i=0;i<6;++i){WCHAR text[40];swprintf_s(text,ARRAYSIZE(text),L"Item %02d",i+1);
        if(SendDlgItemMessageW(window,LIST,LB_ADDSTRING,0,(LPARAM)text)==LB_ERR ||
            SendDlgItemMessageW(window,COMBO,CB_ADDSTRING,0,(LPARAM)text)==CB_ERR)return FALSE;
    }
    layout(window);reset(window);return TRUE;
}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_CREATE){CREATESTRUCTW *creation=(CREATESTRUCTW *)b;
        return populate(window,creation->hInstance)?0:-1;}
    if(message==DM_GETDEFID)return MAKELRESULT(APPLY,DC_HASDEFID);
    if(message==WM_COMMAND){int id=LOWORD(a),notification=HIWORD(a);WCHAR text[256],line[300];
        if(id>=2000 && id<2008){demo_catalog_open(window,(unsigned)(id-2000));return 0;}
        if(id==EXIT_APP){DestroyWindow(window);return 0;}
        if(id==RESET){reset(window);return 0;}
        if(id==ABOUT){MessageBoxW(window,L"A native Windows controls demonstration.",L"About Win16 Style Demo",MB_OK|MB_ICONINFORMATION);return 0;}
        if(id==APPLY && notification==BN_CLICKED){
            GetDlgItemTextW(window,EDIT_TEXT,text,ARRAYSIZE(text));
            swprintf_s(line,ARRAYSIZE(line),L"Applied: %ls",text);status(window,line);return 0;
        }
        if(id==CHECK_OPTION && notification==BN_CLICKED){status(window,
            IsDlgButtonChecked(window,CHECK_OPTION)==BST_CHECKED?L"Enabled":L"Disabled");return 0;}
        if((id==RADIO_A || id==RADIO_B) && notification==BN_CLICKED){status(window,id==RADIO_A?L"Option A":L"Option B");return 0;}
        if((id==LIST && notification==LBN_SELCHANGE) || (id==COMBO && notification==CBN_SELCHANGE)){
            LRESULT selected=SendDlgItemMessageW(window,id,id==LIST?LB_GETCURSEL:CB_GETCURSEL,0,0);
            swprintf_s(line,ARRAYSIZE(line),L"%ls selection: %lld",id==LIST?L"List":L"Combo",(long long)selected+1);status(window,line);return 0;
        }
    }
    if(message==WM_HSCROLL && (HWND)b==GetDlgItem(window,SCROLL)){
        SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};WCHAR text[80];GetScrollInfo((HWND)b,SB_CTL,&info);
        switch(LOWORD(a)){
        case SB_LINELEFT:--info.nPos;break;case SB_LINERIGHT:++info.nPos;break;
        case SB_PAGELEFT:info.nPos-=(int)info.nPage;break;case SB_PAGERIGHT:info.nPos+=(int)info.nPage;break;
        case SB_LEFT:info.nPos=info.nMin;break;case SB_RIGHT:info.nPos=info.nMax;break;
        case SB_THUMBTRACK:case SB_THUMBPOSITION:info.nPos=info.nTrackPos;break;
        default:return 0;}
        info.fMask=SIF_POS;SetScrollInfo((HWND)b,SB_CTL,&info,TRUE);
        swprintf_s(text,ARRAYSIZE(text),L"Scroll position: %d",GetScrollPos((HWND)b,SB_CTL));status(window,text);return 0;
    }
    if(message==WM_DPICHANGED){RECT *r=(RECT *)b;
        SetWindowPos(window,NULL,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);layout(window);return 0;}
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,a,b);
}
static HMENU menu(void)
{
    HMENU bar=CreateMenu(),file=CreatePopupMenu(),help=CreatePopupMenu();
    if(!bar || !file || !help)goto failure;
    if(!AppendMenuW(file,MF_STRING,RESET,L"&Reset") || !AppendMenuW(file,MF_SEPARATOR,0,NULL) ||
       !AppendMenuW(file,MF_STRING,EXIT_APP,L"E&xit") || !AppendMenuW(help,MF_STRING,ABOUT,L"&About"))goto failure;
    if(!AppendMenuW(bar,MF_POPUP,(UINT_PTR)file,L"&File"))goto failure;
    file=NULL;
    if(!demo_catalog_menu(bar))goto failure;
    if(!AppendMenuW(bar,MF_POPUP,(UINT_PTR)help,L"&Help"))goto failure;
    help=NULL;
    return bar;
failure:
    if(file)DestroyMenu(file);
    if(help)DestroyMenu(help);
    if(bar)DestroyMenu(bar);
    return NULL;
}
int demo_run(HINSTANCE instance,int show)
{
    WNDCLASSW cls={0};MSG message;HMENU bar=menu();
    cls.lpfnWndProc=window_proc;cls.hInstance=instance;cls.hCursor=LoadCursorW(NULL,IDC_ARROW);
    cls.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);cls.lpszClassName=L"Win16StyleDemo";
    if(!bar || !demo_catalog_register(instance) || !RegisterClassW(&cls)){if(bar)DestroyMenu(bar);return 1;}
    HWND window=CreateWindowW(cls.lpszClassName,L"Win16 Style Demo",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT,CW_USEDEFAULT,600,500,NULL,bar,instance,NULL);
    if(!window){DestroyMenu(bar);return 1;}
    RECT r={0,0,scaled(window,560),scaled(window,425)};
    if(!AdjustWindowRectExForDpi(&r,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,TRUE,0,GetDpiForWindow(window))){DestroyWindow(window);return 1;}
    SetWindowPos(window,NULL,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    ShowWindow(window,show);UpdateWindow(window);SetFocus(GetDlgItem(window,EDIT_TEXT));
    int result;
    while((result=GetMessageW(&message,NULL,0,0))>0)
        if(!demo_catalog_message(&message) && !IsDialogMessageW(window,&message)){TranslateMessage(&message);DispatchMessageW(&message);demo_catalog_dispatched();}
    demo_catalog_close();return result<0?1:(int)message.wParam;
}
