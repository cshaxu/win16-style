#define _WIN32_WINNT 0x0A00
#include "print_general.h"
#include "print_directory.h"
#include "print_shell.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/dialog/print_ranges.h"
#include "lib-win32-style/application/dialog/print_resources.h"
#include <commctrl.h>
#include <shlobj.h>
#include <winspool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

enum { range_limit=32, fit_page_message=WM_APP+1, select_printer_message=WM_APP+2, printer_view_message=WM_APP+3 };
typedef struct general_page {
    BOOL ansi, full;
    DWORD flags,minimum,maximum,capacity,copies,action,range_count;
    PRINTPAGERANGE ranges[range_limit];
    WCHAR range_text[512];
    PRINTER_INFO_4W *devices;
    PCWSTR selected,default_device;
    PWSTR directory_device;
    DWORD device_count;
    HGLOBAL live_mode,live_names;
    style_print_snapshot saved;
    win2k_print_shell *shell;
    BOOL initialized,shell_visible;
} general_page;

typedef struct device_messages {
    UINT add, set_data, select, current, data, find, changed;
} device_messages;

static const device_messages device_controls[]={
    {CB_ADDSTRING,CB_SETITEMDATA,CB_SETCURSEL,CB_GETCURSEL,CB_GETITEMDATA,CB_FINDSTRINGEXACT,CBN_SELCHANGE},
    {LB_ADDSTRING,LB_SETITEMDATA,LB_SETCURSEL,LB_GETCURSEL,LB_GETITEMDATA,LB_FINDSTRINGEXACT,LBN_SELCHANGE}
};

static HGLOBAL device_names(const void *name,const void *port,BOOL ansi,BOOL is_default)
{
    if(!name || !port)return NULL;
    const void *driver=ansi?(const void *)"winspool":(const void *)L"winspool";
    size_t unit=ansi?1:sizeof(WCHAR),start=sizeof(DEVNAMES)/unit;
    size_t device=start+ARRAYSIZE("winspool");
    size_t output=device+(ansi?strlen(name):wcslen(name))+1;
    size_t length=output+(ansi?strlen(port):wcslen(port))+1;
    if(length>UINT16_MAX)return NULL;
    HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,length*unit);
    DEVNAMES *names=memory?GlobalLock(memory):NULL;
    if(!names){if(memory)GlobalFree(memory);return NULL;}
    *names=(DEVNAMES){(WORD)start,(WORD)device,(WORD)output,is_default?DN_DEFAULTPRN:0};
    CopyMemory((BYTE *)names+start*unit,driver,(device-start)*unit);
    CopyMemory((BYTE *)names+device*unit,name,(output-device)*unit);
    CopyMemory((BYTE *)names+output*unit,port,(length-output)*unit);
    GlobalUnlock(memory);
    return memory;
}

static BOOL load_device(general_page *page,PCWSTR device,HWND preferences,HGLOBAL previous)
{
    HANDLE printer=NULL;
    if(!device || !OpenPrinterW((PWSTR)device,&printer,NULL))return FALSE;
    DWORD size=0;
    if(page->ansi)GetPrinterA(printer,5,NULL,0,&size);
    else GetPrinterW(printer,5,NULL,0,&size);
    BYTE *info=size?malloc(size):NULL;
    BOOL loaded=info && (page->ansi?GetPrinterA(printer,5,info,size,&size):GetPrinterW(printer,5,info,size,&size));
    HGLOBAL mode=NULL,names=NULL;
    if(loaded){
        PRINTER_INFO_5A *a=(void *)info;
        PRINTER_INFO_5W *w=(void *)info;
        LONG bytes=page->ansi?DocumentPropertiesA(NULL,printer,a->pPrinterName,NULL,NULL,0):
            DocumentPropertiesW(NULL,printer,w->pPrinterName,NULL,NULL,0);
        mode=bytes>0?GlobalAlloc(GMEM_MOVEABLE,(SIZE_T)bytes):NULL;
        void *output=mode?GlobalLock(mode):NULL;
        void *input=previous?GlobalLock(previous):NULL;
        DWORD options=DM_OUT_BUFFER|(previous?DM_IN_BUFFER:0)|(preferences?DM_IN_PROMPT:0);
        BOOL settings=output && (!previous || input) &&
            (page->ansi?DocumentPropertiesA(preferences,printer,a->pPrinterName,output,input,options):
             DocumentPropertiesW(preferences,printer,w->pPrinterName,output,input,options))==IDOK;
        if(input)GlobalUnlock(previous);
        if(output)GlobalUnlock(mode);
        if(settings){
            BOOL is_default=page->default_device && !lstrcmpiW(device,page->default_device);
            names=page->ansi?device_names(a->pPrinterName,a->pPortName,TRUE,is_default):
                device_names(w->pPrinterName,w->pPortName,FALSE,is_default);
        }
    }
    free(info);
    ClosePrinter(printer);
    if(!names){if(mode)GlobalFree(mode);return FALSE;}
    if(page->live_mode)GlobalFree(page->live_mode);
    if(page->live_names)GlobalFree(page->live_names);
    page->live_mode=mode;
    page->live_names=names;
    page->selected=device;
    return TRUE;
}

static BOOL read_snapshot(HWND window,general_page *page,BOOL confirm)
{
    BOOL numeric=FALSE;
    DWORD copies=GetDlgItemInt(window,WIN2K_PRINT_COPIES,&numeric,FALSE);
    if(!numeric || !copies || copies>32767)return FALSE;
    DWORD flags=page->flags&~(DWORD)(PD_CURRENTPAGE|PD_SELECTION|PD_PAGENUMS|PD_COLLATE|PD_PRINTTOFILE);
    PRINTPAGERANGE ranges[range_limit];
    DWORD count=0;
    if(IsDlgButtonChecked(window,WIN2K_PRINT_PAGES)==BST_CHECKED){
        WCHAR input[ARRAYSIZE(page->range_text)];
        if(!GetDlgItemTextW(window,WIN2K_PRINT_RANGE,input,ARRAYSIZE(input)) ||
           !style_print_ranges_parse(input,page->minimum,page->maximum,ranges,page->capacity,&count))return FALSE;
        flags|=PD_PAGENUMS;
    }else if(IsDlgButtonChecked(window,WIN2K_PRINT_SELECTION)==BST_CHECKED)flags|=PD_SELECTION;
    else if(IsDlgButtonChecked(window,WIN2K_PRINT_CURRENT)==BST_CHECKED)flags|=PD_CURRENTPAGE;
    if(IsDlgButtonChecked(window,WIN2K_PRINT_COLLATE)==BST_CHECKED)flags|=PD_COLLATE;
    if(IsDlgButtonChecked(window,WIN2K_PRINT_FILE)==BST_CHECKED)flags|=PD_PRINTTOFILE;
    style_print_snapshot snapshot;
    if(!style_print_snapshot_create(page->live_mode,page->live_names,page->ansi,copies,flags,&snapshot))return FALSE;
    style_print_snapshot_dispose(&page->saved);
    page->saved=snapshot;
    page->range_count=count;
    if(count)CopyMemory(page->ranges,ranges,count*sizeof(*ranges));
    page->action=confirm?PD_RESULT_PRINT:PD_RESULT_APPLY;
    return TRUE;
}

static void present_device(HWND window,PCWSTR device)
{
    HANDLE printer=NULL;
    DWORD bytes=0;
    PRINTER_INFO_2W *info=NULL;
    PCWSTR status=L"Unavailable",location=L"",comment=L"";
    if(OpenPrinterW((PWSTR)device,&printer,NULL)){
        GetPrinterW(printer,2,NULL,0,&bytes);
        info=bytes?malloc(bytes):NULL;
        if(info && GetPrinterW(printer,2,(BYTE *)info,bytes,&bytes)){
            status=info->Status&(PRINTER_STATUS_ERROR|PRINTER_STATUS_OFFLINE)?L"Offline or unavailable":
                info->Status&PRINTER_STATUS_PAUSED?L"Paused":
                info->Status&PRINTER_STATUS_PRINTING?L"Printing":
                info->Status?L"Busy":L"Ready";
            if(info->pLocation)location=info->pLocation;
            if(info->pComment)comment=info->pComment;
        }
        ClosePrinter(printer);
    }
    SetDlgItemTextW(window,WIN2K_PRINT_DEVICE_STATUS,status);
    SetDlgItemTextW(window,WIN2K_PRINT_DEVICE_LOCATION,location);
    SetDlgItemTextW(window,WIN2K_PRINT_DEVICE_COMMENT,comment);
    free(info);
}

static void initialize_page(HWND window,const general_page *page)
{
    const device_messages *messages=&device_controls[page->full];
    for(DWORD i=0;i<page->device_count;++i){
        PCWSTR name=page->devices[i].pPrinterName;
        LRESULT index=SendDlgItemMessageW(window,WIN2K_PRINT_DEVICE,messages->add,0,(LPARAM)name);
        if(index<0)continue;
        SendDlgItemMessageW(window,WIN2K_PRINT_DEVICE,messages->set_data,(WPARAM)index,(LPARAM)name);
        if(name==page->selected)SendDlgItemMessageW(window,WIN2K_PRINT_DEVICE,messages->select,(WPARAM)index,0);
    }
    if(page->full){
        present_device(window,page->selected);
        EnableWindow(GetDlgItem(window,WIN2K_PRINT_FIND_DEVICE),!SHRestricted(REST_NOPRINTERADD));
    }
    int checked=WIN2K_PRINT_ALL;
    if(page->flags&PD_SELECTION)checked=WIN2K_PRINT_SELECTION;
    if(page->flags&PD_CURRENTPAGE)checked=WIN2K_PRINT_CURRENT;
    if(page->flags&PD_PAGENUMS)checked=WIN2K_PRINT_PAGES;
    CheckRadioButton(window,WIN2K_PRINT_ALL,WIN2K_PRINT_PAGES,checked);
    EnableWindow(GetDlgItem(window,WIN2K_PRINT_SELECTION),!(page->flags&PD_NOSELECTION));
    EnableWindow(GetDlgItem(window,WIN2K_PRINT_CURRENT),!(page->flags&PD_NOCURRENTPAGE));
    EnableWindow(GetDlgItem(window,WIN2K_PRINT_PAGES),!(page->flags&PD_NOPAGENUMS));
    EnableWindow(GetDlgItem(window,WIN2K_PRINT_RANGE),!(page->flags&PD_NOPAGENUMS));
    SendDlgItemMessageW(window,WIN2K_PRINT_RANGE,EM_SETLIMITTEXT,ARRAYSIZE(page->range_text)-1,0);
    SendDlgItemMessageW(window,WIN2K_PRINT_COPIES,EM_SETLIMITTEXT,5,0);
    SetDlgItemTextW(window,WIN2K_PRINT_RANGE,page->range_text);
    SetDlgItemInt(window,WIN2K_PRINT_COPIES,page->copies,FALSE);
    CheckDlgButton(window,WIN2K_PRINT_COLLATE,page->flags&PD_COLLATE?BST_CHECKED:BST_UNCHECKED);
    CheckDlgButton(window,WIN2K_PRINT_FILE,page->flags&PD_PRINTTOFILE?BST_CHECKED:BST_UNCHECKED);
    HWND file=GetDlgItem(window,WIN2K_PRINT_FILE);
    EnableWindow(file,!(page->flags&PD_DISABLEPRINTTOFILE));
    if(page->flags&PD_HIDEPRINTTOFILE)ShowWindow(file,SW_HIDE);
}

static void fit_sheet(HWND window)
{
    RECT outer;
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    if(!GetWindowRect(window,&outer) ||
       !GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor) ||
       outer.bottom<=monitor.rcWork.bottom)return;
    /* Keep the native action buttons and trim only unused bottom margin. */
    RECT button;
    int border=GetSystemMetrics(SM_CYEDGE);
    if(GetWindowRect(GetDlgItem(window,IDOK),&button) && button.bottom+border<=monitor.rcWork.bottom)
        SetWindowPos(window,NULL,0,0,outer.right-outer.left,monitor.rcWork.bottom-outer.top,
            SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSENDCHANGING);
}

static BOOL select_shell_printer(HWND window,general_page *page)
{
    PWSTR name=win2k_print_shell_selected(page->shell);
    PCWSTR device=NULL;
    for(DWORD i=0;name && i<page->device_count;++i){
        if(!lstrcmpiW(name,page->devices[i].pPrinterName)){device=page->devices[i].pPrinterName;break;}
    }
    CoTaskMemFree(name);
    if(device==page->selected)return TRUE;
    if(device && load_device(page,device,NULL,NULL)){
        present_device(window,device);
        SetDlgItemTextW(window,WIN2K_PRINT_STATUS,L"");
        PropSheet_Changed(GetParent(window),window);
        return TRUE;
    }
    win2k_print_shell_select(page->shell,page->selected);
    SetDlgItemTextW(window,WIN2K_PRINT_STATUS,L"Could not read settings for the selected printer.");
    return FALSE;
}

static BOOL add_directory_printer(HWND window,general_page *page,PCWSTR device)
{
    HWND list=GetDlgItem(window,WIN2K_PRINT_DEVICE);
    LRESULT index=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)device);
    if(index>=0){
        LRESULT existing=SendMessageW(list,LB_GETITEMDATA,(WPARAM)index,0);
        if(!existing || existing==LB_ERR || !load_device(page,(PCWSTR)existing,NULL,NULL))return FALSE;
        if(page->shell_visible && win2k_print_shell_select(page->shell,page->selected))return TRUE;
    }else{
        index=SendMessageW(list,LB_ADDSTRING,0,(LPARAM)device);
        if(index<0)return FALSE;
        if(SendMessageW(list,LB_SETITEMDATA,(WPARAM)index,(LPARAM)device)==LB_ERR ||
           !load_device(page,device,NULL,NULL)){
            SendMessageW(list,LB_DELETESTRING,(WPARAM)index,0);
            return FALSE;
        }
        if(page->directory_device){
            LRESULT old=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)page->directory_device);
            if(old>=0)SendMessageW(list,LB_DELETESTRING,(WPARAM)old,0);
            CoTaskMemFree(page->directory_device);
        }
        page->directory_device=(PWSTR)device;
        index=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)device);
    }
    win2k_print_shell_close(page->shell);
    page->shell=NULL;
    page->shell_visible=FALSE;
    ShowWindow(list,SW_SHOW);
    SendMessageW(list,LB_SETCURSEL,(WPARAM)index,0);
    return TRUE;
}

static BOOL find_directory_printer(HWND window,general_page *page)
{
    if(!page->full || SHRestricted(REST_NOPRINTERADD))return FALSE;
    PWSTR device=NULL;
    HRESULT result=win2k_print_directory_select(window,&device);
    BOOL selected=result==S_OK && add_directory_printer(window,page,device);
    if(device!=page->directory_device)CoTaskMemFree(device);
    if(selected)present_device(window,page->selected);
    else if(result!=S_FALSE)SetDlgItemTextW(window,WIN2K_PRINT_STATUS,L"Could not select a directory printer.");
    return selected;
}

static INT_PTR CALLBACK general_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_INITDIALOG){
        general_page *page=(void *)((const PROPSHEETPAGEW *)lp)->lParam;
        initialize_page(window,page);
        SetDlgItemTextW(GetParent(window),IDOK,L"Print");
        SetWindowLongPtrW(window,DWLP_USER,(LONG_PTR)page);
        if(page->full)page->shell=win2k_print_shell_open(window,GetDlgItem(window,WIN2K_PRINT_DEVICE),
            select_printer_message,printer_view_message);
        page->initialized=TRUE;
        PostMessageW(window,fit_page_message,0,0);
        return TRUE;
    }
    general_page *page=(void *)GetWindowLongPtrW(window,DWLP_USER);
    if(!page)return FALSE;
    if(message==WM_DESTROY){
        SetWindowLongPtrW(window,DWLP_USER,0);
        win2k_print_shell_close(page->shell);
        page->shell=NULL;
        return FALSE;
    }
    if(message==printer_view_message){
        if(page->shell && !page->shell_visible){
            page->shell_visible=wp && win2k_print_shell_show(page->shell,GetDlgItem(window,WIN2K_PRINT_DEVICE),page->selected);
            if(!page->shell_visible){win2k_print_shell_close(page->shell);page->shell=NULL;}
        }
        return TRUE;
    }
    if(message==select_printer_message){
        if(page->shell_visible)select_shell_printer(window,page);
        return TRUE;
    }
    if(message==fit_page_message){
        DPI_AWARENESS_CONTEXT previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        fit_sheet(GetParent(window));
        if(previous)SetThreadDpiAwarenessContext(previous);
        return TRUE;
    }
    if(message==WM_COMMAND){
        UINT control=LOWORD(wp),notice=HIWORD(wp);
        const device_messages *messages=&device_controls[page->full];
        if(!page->shell_visible && control==WIN2K_PRINT_DEVICE && notice==messages->changed){
            LRESULT index=SendDlgItemMessageW(window,control,messages->current,0,0);
            LRESULT device=index>=0?SendDlgItemMessageW(window,control,messages->data,(WPARAM)index,0):0;
            if(!device || device==-1 || !load_device(page,(PCWSTR)device,NULL,NULL)){
                LRESULT old=SendDlgItemMessageW(window,control,messages->find,(WPARAM)-1,(LPARAM)page->selected);
                SendDlgItemMessageW(window,control,messages->select,(WPARAM)old,0);
                SetDlgItemTextW(window,WIN2K_PRINT_STATUS,L"Could not read settings for the selected printer.");
                return TRUE;
            }
            if(page->full)present_device(window,page->selected);
        }else if(control==WIN2K_PRINT_FIND_DEVICE && notice==BN_CLICKED){
            if(!find_directory_printer(window,page))return TRUE;
        }else if(control==WIN2K_PRINT_PREFERENCES && notice==BN_CLICKED){
            if(page->shell_visible && !select_shell_printer(window,page))return TRUE;
            if(!load_device(page,page->selected,window,page->live_mode))return TRUE;
        }else if(notice==EN_CHANGE && control==WIN2K_PRINT_RANGE){
            CheckRadioButton(window,WIN2K_PRINT_ALL,WIN2K_PRINT_PAGES,WIN2K_PRINT_PAGES);
        }else if(!(notice==EN_CHANGE && control==WIN2K_PRINT_COPIES) &&
                 !(notice==BN_CLICKED && control>=WIN2K_PRINT_ALL && control<=WIN2K_PRINT_FILE))return FALSE;
        SetDlgItemTextW(window,WIN2K_PRINT_STATUS,L"");
        PropSheet_Changed(GetParent(window),window);
        return TRUE;
    }
    if(message==WM_NOTIFY && ((NMHDR *)lp)->code==PSN_APPLY){
        BOOL selected=!page->shell_visible || select_shell_printer(window,page);
        BOOL ok=selected && read_snapshot(window,page,((PSHNOTIFY *)lp)->lParam!=0);
        if(selected && !ok)SetDlgItemTextW(window,WIN2K_PRINT_STATUS,L"Check the page range, copy count, or printer settings.");
        SetWindowLongPtrW(window,DWLP_MSGRESULT,ok?PSNRET_NOERROR:PSNRET_INVALID);
        return TRUE;
    }
    return FALSE;
}

static UINT general_template(HWND owner,DWORD flags,const classic_dialog_metrics *metrics)
{
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    UINT dpi=GetDpiForWindow(owner);
    if(!dpi || !GetMonitorInfoW(MonitorFromWindow(owner,MONITOR_DEFAULTTONEAREST),&monitor))return WIN2K_PRINT_GENERAL;
    if(monitor.rcWork.right-monitor.rcWork.left<MulDiv(metrics->print_width,(int)dpi,96))return WIN2K_PRINT_GENERAL;
    /* Leave room for the native tab, action buttons and frame around the page. */
    LONG height=monitor.rcWork.bottom-monitor.rcWork.top;
    if((flags&PD_USELARGETEMPLATE) && height>=MulDiv(metrics->print_large_height,(int)dpi,96))return WIN2K_PRINT_GENERAL_LARGE;
    return height>=MulDiv(metrics->print_full_height,(int)dpi,96)?WIN2K_PRINT_GENERAL_FULL:WIN2K_PRINT_GENERAL;
}

static HRESULT show_sheet(HINSTANCE module,HWND owner,general_page *page)
{
    classic_theme_mapping source={0};
    HRESULT loaded=classic_theme_open_dialog(module,&source);
    if(FAILED(loaded))return loaded;
    const classic_dialog_metrics fallback=WIN2000_DIALOG_METRICS;
    const classic_dialog_metrics *metrics=source.model.dialog_metrics.print_width?&source.model.dialog_metrics:&fallback;
    UINT layout=general_template(owner,page->flags,metrics);
    if(!FindResourceW(source.resources,MAKEINTRESOURCEW(layout),MAKEINTRESOURCEW(5))){
        classic_theme_close(&source);
        return S_FALSE;
    }
    page->full=layout!=WIN2K_PRINT_GENERAL;
    PROPSHEETPAGEW details={.dwSize=sizeof(details),.dwFlags=PSP_USETITLE,.hInstance=source.resources,
        .pszTemplate=MAKEINTRESOURCEW(layout),.pszTitle=L"General",
        .pfnDlgProc=general_proc,.lParam=(LPARAM)page};
    PROPSHEETHEADERW sheet={.dwSize=sizeof(sheet),.dwFlags=PSH_PROPSHEETPAGE,.hwndParent=owner,
        .hInstance=module,.pszCaption=L"Print",.nPages=1,.ppsp=&details};
    INT_PTR result=PropertySheetW(&sheet);
    classic_theme_close(&source);
    return result<0 || !page->initialized?E_FAIL:S_OK;
}

static PCWSTR match_device(const general_page *page,const DEVNAMES *names,SIZE_T bytes)
{
    SIZE_T unit=page->ansi?1:sizeof(WCHAR);
    if(bytes<sizeof(*names) || names->wDeviceOffset<sizeof(*names)/unit)return NULL;
    SIZE_T offset=names->wDeviceOffset,extent=bytes/unit;
    if(offset>=extent)return NULL;
    const WCHAR *name;
    WCHAR converted[512];
    if(page->ansi){
        const char *a=(const char *)names+offset;
        if(!memchr(a,0,extent-offset) || !MultiByteToWideChar(CP_ACP,0,a,-1,converted,ARRAYSIZE(converted)))return NULL;
        name=converted;
    }else{
        name=(const WCHAR *)names+offset;
        if(!wmemchr(name,0,extent-offset))return NULL;
    }
    for(DWORD i=0;i<page->device_count;++i){
        PCWSTR candidate=page->devices[i].pPrinterName;
        if(!lstrcmpiW(name,candidate))return candidate;
    }
    return NULL;
}

static short mode_copies(HGLOBAL input,BOOL ansi)
{
    SIZE_T available=GlobalSize(input);
    void *mode=GlobalLock(input);
    if(!mode)return -1;
    short copies=-1;
    if(ansi && available>=FIELD_OFFSET(DEVMODEA,dmCollate)+sizeof(short)){
        const DEVMODEA *a=mode;
        if(a->dmSize>=FIELD_OFFSET(DEVMODEA,dmCollate)+sizeof(short) &&
           (SIZE_T)a->dmSize+a->dmDriverExtra<=available)copies=(a->dmFields&DM_COPIES)?a->dmCopies:0;
    }else if(!ansi && available>=FIELD_OFFSET(DEVMODEW,dmCollate)+sizeof(short)){
        const DEVMODEW *w=mode;
        if(w->dmSize>=FIELD_OFFSET(DEVMODEW,dmCollate)+sizeof(short) &&
           (SIZE_T)w->dmSize+w->dmDriverExtra<=available)copies=(w->dmFields&DM_COPIES)?w->dmCopies:0;
    }
    GlobalUnlock(input);
    return copies;
}

static BOOL prepare_devices(general_page *page,HGLOBAL initial_mode,HGLOBAL initial_names)
{
    DWORD size=0,count=0;
    EnumPrintersW(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,NULL,4,NULL,0,&size,&count);
    page->devices=size?malloc(size):NULL;
    if(!page->devices || !EnumPrintersW(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,NULL,4,
       (BYTE *)page->devices,size,&size,&page->device_count) || !page->device_count)return FALSE;
    DWORD chars=0;
    GetDefaultPrinterW(NULL,&chars);
    WCHAR *name=chars?calloc(chars,sizeof(*name)):NULL;
    page->default_device=name;
    if(name && !GetDefaultPrinterW(name,&chars)){free(name);page->default_device=NULL;name=NULL;}
    if(initial_names){
        const DEVNAMES *names=GlobalLock(initial_names);
        if(!names)return FALSE;
        PCWSTR device=match_device(page,names,GlobalSize(initial_names));
        GlobalUnlock(initial_names);
        short copies=mode_copies(initial_mode,page->ansi);
        if(!device || copies<0 || !load_device(page,device,NULL,initial_mode))return FALSE;
        if(copies)page->copies=(DWORD)copies;
        return TRUE;
    }
    if(!name)return FALSE;
    for(DWORD i=0;i<page->device_count;++i){
        PCWSTR device=page->devices[i].pPrinterName;
        if(!lstrcmpiW(name,device))return load_device(page,device,NULL,NULL);
    }
    return FALSE;
}

static void dispose_page(general_page *page)
{
    CoTaskMemFree(page->directory_device);
    free((void *)page->default_device);
    free(page->devices);
    if(page->live_mode)GlobalFree(page->live_mode);
    if(page->live_names)GlobalFree(page->live_names);
    style_print_snapshot_dispose(&page->saved);
}

static BOOL supports_flags(DWORD flags,DWORD count,DWORD capacity,DWORD minimum,DWORD maximum,
    const PRINTPAGERANGE *ranges)
{
    const DWORD supported=PD_NOPAGENUMS|PD_NOSELECTION|PD_NOCURRENTPAGE|PD_NOWARNING|
        PD_CURRENTPAGE|PD_HIDEPRINTTOFILE|PD_DISABLEPRINTTOFILE|PD_PRINTTOFILE|PD_COLLATE|
        PD_PAGENUMS|PD_SELECTION|PD_RETURNDC|PD_RETURNIC|PD_USEDEVMODECOPIESANDCOLLATE|PD_USELARGETEMPLATE;
    if(flags&~supported)return FALSE;
    if((flags&PD_SELECTION) && (flags&(PD_NOSELECTION|PD_CURRENTPAGE|PD_PAGENUMS)))return FALSE;
    if((flags&PD_CURRENTPAGE) && (flags&(PD_NOCURRENTPAGE|PD_PAGENUMS)))return FALSE;
    if((flags&PD_PRINTTOFILE) && (flags&(PD_DISABLEPRINTTOFILE|PD_HIDEPRINTTOFILE)))return FALSE;
    if(flags&PD_NOPAGENUMS)return !count && !(flags&PD_PAGENUMS);
    return ranges && capacity && capacity<=range_limit && minimum && minimum<=maximum &&
        ((flags&PD_PAGENUMS)?count && count<=capacity:!count);
}

#define PRINT_GENERAL(NAME,TYPE,ANSI) \
BOOL NAME(HINSTANCE resources,TYPE *p,HRESULT *result) \
{ \
    if(!resources || !p || p->lStructSize!=sizeof(*p) || !p->hwndOwner || !IsWindow(p->hwndOwner) || \
       p->Flags2 || p->ExclusionFlags || p->hDC || (!!p->hDevMode!=!!p->hDevNames) || \
       (p->hDevMode && p->hDevMode==p->hDevNames) || \
       !style_print_global_available(p->hDevMode) || !style_print_global_available(p->hDevNames) || \
       !p->nCopies || p->nCopies>32767 || p->lpCallback || p->nPropertyPages || p->lphPropertyPages || \
       p->nStartPage!=START_PAGE_GENERAL || !supports_flags(p->Flags,p->nPageRanges,p->nMaxPageRanges, \
        p->nMinPage,p->nMaxPage,p->lpPageRanges))return FALSE; \
    general_page page={.ansi=ANSI,.flags=p->Flags,.minimum=p->nMinPage,.maximum=p->nMaxPage, \
        .capacity=p->nMaxPageRanges,.copies=p->nCopies}; \
    if(p->nPageRanges && !style_print_ranges_format(p->lpPageRanges,p->nPageRanges,p->nMinPage,p->nMaxPage, \
       page.range_text,ARRAYSIZE(page.range_text)))return FALSE; \
    if(!prepare_devices(&page,p->hDevMode,p->hDevNames)){dispose_page(&page);return FALSE;} \
    *result=show_sheet(resources,p->hwndOwner,&page); \
    if(*result==S_FALSE){dispose_page(&page);return FALSE;} \
    if(SUCCEEDED(*result) && page.saved.mode && (!style_print_global_available(p->hDevMode) || \
       !style_print_global_available(p->hDevNames)))*result=E_FAIL; \
    if(SUCCEEDED(*result)){ \
        p->dwResultAction=page.action; \
        if(page.saved.mode){ \
            if(p->hDevMode)GlobalFree(p->hDevMode); \
            if(p->hDevNames)GlobalFree(p->hDevNames); \
            p->hDevMode=page.saved.mode;p->hDevNames=page.saved.names;p->hDC=page.saved.dc; \
            p->nCopies=page.saved.copies;p->Flags=page.saved.flags;p->nPageRanges=page.range_count; \
            if(page.range_count)CopyMemory(p->lpPageRanges,page.ranges,page.range_count*sizeof(*page.ranges)); \
            page.saved=(style_print_snapshot){0}; \
        } \
    } \
    dispose_page(&page); \
    return TRUE; \
}
PRINT_GENERAL(win2k_print_general_a,PRINTDLGEXA,TRUE)
PRINT_GENERAL(win2k_print_general_w,PRINTDLGEXW,FALSE)
#undef PRINT_GENERAL
