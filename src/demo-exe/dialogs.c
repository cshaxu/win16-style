/* Native common-dialog examples. The application never links the style DLL. */
#define COBJMACROS
#define INITGUID
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define NOTE 2099
static HWND find_window,find_owner;
static FINDREPLACEW find_parameters;
static WCHAR find_text[128],replace_text[128];
static UINT find_message;
static void note(HWND owner,PCWSTR text){SetDlgItemTextW(owner,NOTE,text);}
static UINT_PTR CALLBACK old_file_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{(void)window;(void)message;(void)wparam;(void)lparam;return 0;}
static void old_file(HWND owner,BOOL save)
{
    WCHAR file[32768]=L"";OPENFILENAMEW options={0};options.lStructSize=sizeof(options);options.hwndOwner=owner;
    options.lpstrFile=file;options.nMaxFile=ARRAYSIZE(file);options.lpstrFilter=L"All files\0*.*\0Text files\0*.txt\0\0";
    options.Flags=OFN_ENABLEHOOK|OFN_LONGNAMES|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|(save?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);
    options.lpfnHook=old_file_hook;
    BOOL chosen=save?GetSaveFileNameW(&options):GetOpenFileNameW(&options);
    note(owner,chosen?file:L"Cancelled. No file was opened or saved.");
}
static void multi_file(HWND owner,BOOL explorer)
{
    WCHAR file[32768]=L"",text[512];OPENFILENAMEW options={0};
    options.lStructSize=sizeof(options);options.hwndOwner=owner;options.lpstrFile=file;
    options.nMaxFile=ARRAYSIZE(file);options.lpstrFilter=L"All files\0*.*\0Text files\0*.txt\0\0";
    options.Flags=OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_LONGNAMES;
    if(explorer)options.Flags|=OFN_EXPLORER;else{options.Flags|=OFN_ENABLEHOOK;options.lpfnHook=old_file_hook;}
    if(!GetOpenFileNameW(&options)){DWORD error=CommDlgExtendedError();
        if(error){swprintf(text,ARRAYSIZE(text),L"Multi-open failed: common-dialog error %lu",error);note(owner,text);}
        else note(owner,L"Multi-open cancelled. No files were opened.");
        return;}
    if(explorer && options.nFileOffset && options.nFileOffset<ARRAYSIZE(file) && file[options.nFileOffset-1]==0){
        unsigned count=0;const WCHAR *name=file+options.nFileOffset;
        while(*name && name<file+ARRAYSIZE(file)){++count;name+=wcslen(name)+1;}
        swprintf(text,ARRAYSIZE(text),L"Selected %u files in %.380ls",count,file);note(owner,text);
    }else{swprintf(text,ARRAYSIZE(text),L"Selected: %.480ls",file);note(owner,text);}
}
static void explorer_file(HWND owner,BOOL save)
{
    WCHAR file[32768]=L"",text[128];OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.hwndOwner=owner;
    o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);o.lpstrFilter=L"All files\0*.*\0Text files\0*.txt\0\0";
    o.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(save?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);
    if(save?GetSaveFileNameW(&o):GetOpenFileNameW(&o))note(owner,file);
    else {DWORD error=CommDlgExtendedError();swprintf(text,ARRAYSIZE(text),L"Explorer API cancelled or failed: error %lu",error);note(owner,text);}
}
static void legacy_launch(HWND owner)
{
    WCHAR file[32768]=L"",launcher[32768],arguments[32772],message[128];OPENFILENAMEW o={0};
    o.lStructSize=sizeof(o);o.hwndOwner=owner;o.lpstrFile=file;o.nMaxFile=ARRAYSIZE(file);
    o.lpstrTitle=L"Choose a 16-bit executable to launch";o.lpstrFilter=L"Executables\0*.exe\0All files\0*.*\0\0";
    o.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(!GetOpenFileNameW(&o)){note(owner,L"Launch cancelled.");return;}
    DWORD length=GetModuleFileNameW(NULL,launcher,ARRAYSIZE(launcher));
    WCHAR *slash=length && length<ARRAYSIZE(launcher)?wcsrchr(launcher,L'\\'):NULL;
    if(!slash || (size_t)(slash-launcher)+12>=ARRAYSIZE(launcher)){note(owner,L"Cannot find run16s next to demo.");return;}
    wcscpy(slash+1,L"run16s.exe");
    if(swprintf(arguments,ARRAYSIZE(arguments),L"\"%ls\"",file)<0){note(owner,L"Target path is too long.");return;}
    SHELLEXECUTEINFOW shell={.cbSize=sizeof(shell),.hwnd=owner,.lpFile=launcher,.lpParameters=arguments,.nShow=SW_SHOWNORMAL};
    if(ShellExecuteExW(&shell))note(owner,L"Launch requested; Windows owns any startup error dialog.");
    else {swprintf(message,ARRAYSIZE(message),L"Launcher failed: error %lu",GetLastError());note(owner,message);}
}
static void com_multi_file(HWND owner)
{
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);IFileOpenDialog *dialog=NULL;IShellItemArray *items=NULL;
    HRESULT hr=init;WCHAR text[256];DWORD count=0;FILEOPENDIALOGOPTIONS flags=0;
    if(FAILED(init)){note(owner,L"COM initialization failed.");return;}
    hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileOpenDialog,(void **)&dialog);
    if(SUCCEEDED(hr))hr=IFileOpenDialog_GetOptions(dialog,&flags);
    if(SUCCEEDED(hr))hr=IFileOpenDialog_SetOptions(dialog,flags|FOS_ALLOWMULTISELECT|FOS_FORCEFILESYSTEM);
    if(SUCCEEDED(hr))hr=IFileOpenDialog_Show(dialog,owner);
    if(SUCCEEDED(hr))hr=IFileOpenDialog_GetResults(dialog,&items);
    if(SUCCEEDED(hr))hr=IShellItemArray_GetCount(items,&count);
    if(SUCCEEDED(hr))swprintf(text,ARRAYSIZE(text),L"COM selected %lu items. No files were opened.",count);
    else swprintf(text,ARRAYSIZE(text),L"COM multi-open result: 0x%08lx",(DWORD)hr);
    note(owner,text);if(items)IShellItemArray_Release(items);if(dialog)IFileOpenDialog_Release(dialog);CoUninitialize();
}
static void modern_file(HWND owner,BOOL save,BOOL folder)
{
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(init)){note(owner,L"COM initialization failed.");return;}
    IFileDialog *dialog=NULL;
    HRESULT hr=CoCreateInstance(save?&CLSID_FileSaveDialog:&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog);
    if(SUCCEEDED(hr) && folder){FILEOPENDIALOGOPTIONS flags=0;hr=IFileDialog_GetOptions(dialog,&flags);
        if(SUCCEEDED(hr))hr=IFileDialog_SetOptions(dialog,flags|FOS_PICKFOLDERS|FOS_FORCEFILESYSTEM);}
    if(SUCCEEDED(hr))hr=IFileDialog_Show(dialog,owner);
    if(SUCCEEDED(hr)){
        IShellItem *item=NULL;PWSTR path=NULL;
        hr=IFileDialog_GetResult(dialog,&item);
        if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
        if(SUCCEEDED(hr))note(owner,path);else note(owner,L"Selected item has no filesystem path.");
        if(path)CoTaskMemFree(path);
        if(item)IShellItem_Release(item);
    }else note(owner,hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)?L"Cancelled. Nothing was opened or saved.":L"The Windows dialog could not be opened.");
    if(dialog)IFileDialog_Release(dialog);
    CoUninitialize();
}
static void forced_modern_file(HWND owner,UINT kind)
{
    typedef HRESULT (__cdecl *show_modern_fn)(HWND,UINT,PWSTR,DWORD);
    HMODULE module=GetModuleHandleW(L"ntw16s64.dll");
    FARPROC address=module?GetProcAddress(module,"Ntw16sShowModernFileDialog"):NULL;
    show_modern_fn show=NULL;memcpy(&show,&address,sizeof(show));
    if(!show){note(owner,L"This example requires run16s and its modern-dialog API.");return;}
    WCHAR path[32768];HRESULT hr=show(owner,kind,path,ARRAYSIZE(path));
    if(SUCCEEDED(hr))note(owner,path);
    else note(owner,hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)?L"Cancelled. Nothing was opened or saved.":L"The modern dialog API failed.");
}
static void shell_folder(HWND owner,BOOL bridged)
{
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(init)){note(owner,L"COM initialization failed.");return;}
    BROWSEINFOW info={0};info.hwndOwner=owner;info.lpszTitle=L"Select a folder";
    info.ulFlags=BIF_RETURNONLYFSDIRS|(bridged?(BIF_USENEWUI|BIF_VALIDATE):0);
    PIDLIST_ABSOLUTE selected=SHBrowseForFolderW(&info);
    WCHAR path[MAX_PATH];if(selected && SHGetPathFromIDListW(selected,path))note(owner,path);
    else note(owner,L"Cancelled or selected an item without a filesystem path.");
    if(selected)CoTaskMemFree(selected);
    CoUninitialize();
}
static void find_dialog(HWND owner,BOOL replace)
{
    if(find_window && IsWindow(find_window)){SetForegroundWindow(find_window);return;}
    if(!find_message)find_message=RegisterWindowMessageW(FINDMSGSTRINGW);
    ZeroMemory(&find_parameters,sizeof(find_parameters));find_parameters.lStructSize=sizeof(find_parameters);
    find_parameters.hwndOwner=owner;find_parameters.lpstrFindWhat=find_text;find_parameters.wFindWhatLen=ARRAYSIZE(find_text);
    find_parameters.lpstrReplaceWith=replace_text;find_parameters.wReplaceWithLen=ARRAYSIZE(replace_text);find_parameters.Flags=FR_DOWN;
    find_owner=owner;find_window=replace?ReplaceTextW(&find_parameters):FindTextW(&find_parameters);
    if(!find_window){find_owner=NULL;note(owner,L"The Windows find dialog could not be opened.");}
}
static void font_dialog(HWND owner)
{
    LOGFONTW font={0};CHOOSEFONTW options={0};options.lStructSize=sizeof(options);options.hwndOwner=owner;options.lpLogFont=&font;options.Flags=CF_SCREENFONTS;
    if(ChooseFontW(&options))note(owner,font.lfFaceName);else note(owner,L"Font selection cancelled.");
}
static void color_dialog(HWND owner)
{
    static COLORREF custom[16];CHOOSECOLORW options={0};options.lStructSize=sizeof(options);options.hwndOwner=owner;
    options.lpCustColors=custom;options.rgbResult=RGB(0,0,0);options.Flags=CC_RGBINIT;
    if(ChooseColorW(&options)){WCHAR text[100];swprintf(text,ARRAYSIZE(text),L"Selected RGB: %u, %u, %u",GetRValue(options.rgbResult),GetGValue(options.rgbResult),GetBValue(options.rgbResult));note(owner,text);}
    else note(owner,L"Color selection cancelled.");
}
static void print_dialog(HWND owner,BOOL extended)
{
    HGLOBAL mode=NULL,names=NULL;HDC dc=NULL;
    if(extended){PRINTPAGERANGE ranges[10]={0};PRINTDLGEXW options={0};options.lStructSize=sizeof(options);options.hwndOwner=owner;
        options.Flags=PD_RETURNDC;options.nMaxPageRanges=ARRAYSIZE(ranges);options.lpPageRanges=ranges;options.nMinPage=1;options.nMaxPage=1000;options.nCopies=1;options.nStartPage=START_PAGE_GENERAL;
        HRESULT hr=PrintDlgExW(&options);mode=options.hDevMode;names=options.hDevNames;dc=options.hDC;
        note(owner,SUCCEEDED(hr)?L"Print dialog closed. No print job was sent.":L"The extended print dialog failed.");
    }else{PRINTDLGW options={0};options.lStructSize=sizeof(options);options.hwndOwner=owner;options.Flags=PD_RETURNDC;options.nCopies=1;options.nMinPage=1;options.nMaxPage=1000;
        (void)PrintDlgW(&options);mode=options.hDevMode;names=options.hDevNames;dc=options.hDC;note(owner,L"Print dialog closed. No print job was sent.");}
    if(dc)DeleteDC(dc);
    if(mode)GlobalFree(mode);
    if(names)GlobalFree(names);
}
static void page_dialog(HWND owner)
{
    PAGESETUPDLGW options={0};options.lStructSize=sizeof(options);options.hwndOwner=owner;
    (void)PageSetupDlgW(&options);if(options.hDevMode)GlobalFree(options.hDevMode);if(options.hDevNames)GlobalFree(options.hDevNames);
    note(owner,L"Page setup closed. No print job was sent.");
}
BOOL demo_dialog_execute(HWND owner,unsigned id)
{
    switch(id){
    case 2901:old_file(owner,FALSE);break;case 2902:modern_file(owner,FALSE,FALSE);break;case 2903:forced_modern_file(owner,1);break;
    case 2904:old_file(owner,TRUE);break;case 2905:modern_file(owner,TRUE,FALSE);break;case 2906:forced_modern_file(owner,2);break;
    case 2907:shell_folder(owner,FALSE);break;case 2908:modern_file(owner,FALSE,TRUE);break;case 2909:forced_modern_file(owner,3);break;
    case 2910:find_dialog(owner,FALSE);break;case 2911:find_dialog(owner,TRUE);break;case 2912:font_dialog(owner);break;case 2913:color_dialog(owner);break;
    case 2914:print_dialog(owner,FALSE);break;case 2915:print_dialog(owner,TRUE);break;case 2916:page_dialog(owner);break;
    case 2917:MessageBoxW(owner,L"A native Windows message box.",L"Message",MB_OKCANCEL|MB_ICONINFORMATION);break;
    case 2918:{int button=0;HRESULT hr=TaskDialog(owner,GetModuleHandleW(NULL),L"Task dialog",L"Native task dialog",L"Choose OK or Cancel.",TDCBF_OK_BUTTON|TDCBF_CANCEL_BUTTON,NULL,&button);note(owner,SUCCEEDED(hr)?L"Task dialog closed.":L"Task dialog unavailable.");break;}
    case 2920:shell_folder(owner,TRUE);break;
    case 2921:multi_file(owner,FALSE);break;case 2922:multi_file(owner,TRUE);break;
    case 2923:{PRINTDLGW options={0};options.lStructSize=sizeof(options);options.hwndOwner=owner;options.Flags=PD_PRINTSETUP;
        (void)PrintDlgW(&options);if(options.hDevMode)GlobalFree(options.hDevMode);if(options.hDevNames)GlobalFree(options.hDevNames);
        if(options.hDC)DeleteDC(options.hDC);
        note(owner,L"Print setup closed. No print job was sent.");break;}
    case 2924:explorer_file(owner,FALSE);break;case 2925:explorer_file(owner,TRUE);break;
    case 2926:com_multi_file(owner);break;
    case 2927:legacy_launch(owner);break;
    default:return FALSE;
    }
    return TRUE;
}
BOOL demo_dialog_notify(HWND owner,UINT message,LPARAM parameter)
{
    if(message!=find_message || !find_message || owner!=find_owner || !parameter)return FALSE;
    FINDREPLACEW *data=(FINDREPLACEW *)parameter;
    if(data->Flags&FR_DIALOGTERM){find_window=NULL;find_owner=NULL;note(owner,L"Find / replace closed.");}
    else if(data->Flags&FR_REPLACEALL)note(owner,L"Replace All requested (demo does not edit a document).");
    else if(data->Flags&FR_REPLACE)note(owner,L"Replace requested (demo does not edit a document).");
    else if(data->Flags&FR_FINDNEXT)note(owner,L"Find Next requested (demo does not search a document).");
    return TRUE;
}
BOOL demo_dialog_message(MSG *message)
{return find_window && IsWindow(find_window) && IsDialogMessageW(find_window,message);}
void demo_dialog_close(HWND owner)
{if(find_window && IsWindow(find_window) && find_owner==owner)DestroyWindow(find_window);find_window=NULL;find_owner=NULL;}
