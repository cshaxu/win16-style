#define COBJMACROS
#include <windows.h>
#include <commctrl.h>
#include <oleacc.h>
#include <stdio.h>
#include <wchar.h>
/* Test-only observer, loaded only by a hook on a newly owned GUI thread. */
static BOOL busy;
static unsigned buttons,edits,frames,attached_count,failures;
static BOOL (__cdecl *attached)(HWND);
static FILE *logfile;
static BOOL CALLBACK inspect(HWND w,LPARAM context)
{
    WCHAR cls[80],text[256],module[MAX_PATH];LOGFONTW lf={0};RECT r;
    HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);BOOL own=attached && attached(w);
    (void)context;GetClassNameW(w,cls,80);GetWindowTextW(w,text,256);GetWindowRect(w,&r);
    module[0]=0;GetModuleFileNameW((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE),module,MAX_PATH);
    if(font)GetObjectW(font,sizeof(lf),&lf);
    fwprintf(logfile,L"CONTROL class=%ls text=%ls attached=%u style=%08lx ex=%08lx size=%ld,%ld font=%ls height=%ld owner=%ls\n",
        cls,text,own,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),
        r.right-r.left,r.bottom-r.top,lf.lfFaceName,lf.lfHeight,module);
    if(!wcscmp(cls,L"ThunderRT6CommandButton")){
        LRESULT before=SendMessageW(w,BM_GETSTATE,0,0),during;
        SendMessageW(w,BM_SETSTATE,TRUE,0);during=SendMessageW(w,BM_GETSTATE,0,0);
        SendMessageW(w,BM_SETSTATE,FALSE,0);
        ++buttons;attached_count+=own;
        if(!(during&BST_PUSHED) || (SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED))++failures;
        fwprintf(logfile,L"BUTTON_STATE before=%lld during=%lld restored=%lld\n",(long long)before,(long long)during,(long long)SendMessageW(w,BM_GETSTATE,0,0));
        /* Grid and Clear are safe, bounded real application commands. */
        if(wcsstr(text,L"Grid") || wcsstr(text,L"Clear"))SendMessageW(w,BM_CLICK,0,0);
    }else if(!wcscmp(cls,L"ThunderRT6TextBox")){
        WCHAR after[256];DWORD first=0,last=0;
        ++edits;attached_count+=own;
        SendMessageW(w,WM_SETTEXT,0,(LPARAM)L"12345");SendMessageW(w,EM_SETSEL,1,4);
        SendMessageW(w,EM_GETSEL,(WPARAM)&first,(LPARAM)&last);
        SendMessageW(w,EM_REPLACESEL,TRUE,(LPARAM)L"8");GetWindowTextW(w,after,256);
        if(first!=1 || last!=4 || wcscmp(after,L"185"))++failures;
        fwprintf(logfile,L"EDIT selection=%lu,%lu replacement=%ls undo=%lld\n",first,last,after,(long long)SendMessageW(w,EM_CANUNDO,0,0));
        SendMessageW(w,WM_SETTEXT,0,(LPARAM)text);
    }else if(!wcscmp(cls,L"ThunderRT6Frame")){
        IDispatch *object=NULL;HRESULT hr;
        ++frames;attached_count+=own;
        hr=AccessibleObjectFromWindow(w,OBJID_NATIVEOM,&IID_IDispatch,(void **)&object);
        fwprintf(logfile,L"NATIVEOM class=%ls hr=%08lx object=%u\n",cls,(DWORD)hr,object!=NULL);
        if(object)IDispatch_Release(object);
    }
    return TRUE;
}
__declspec(dllexport) LRESULT CALLBACK VbInspect(int code,WPARAM p,LPARAM l)
{
    if(code>=0 && !busy){
        const CWPSTRUCT *m=(const CWPSTRUCT *)l;
        if(m->message==WM_APP+0x371){
            WCHAR path[MAX_PATH],event_name[128],dllpath[MAX_PATH]={0};HMODULE dll;
            busy=TRUE;buttons=edits=frames=attached_count=failures=0;
            GetEnvironmentVariableW(L"WIN16STYLE_VB_LOG",path,MAX_PATH);
            logfile=_wfopen(path,L"w");dll=GetModuleHandleW(L"ntw16s32.dll");
            attached=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
            if(dll)GetModuleFileNameW(dll,dllpath,MAX_PATH);
            if(logfile){
                fwprintf(logfile,L"PROCESS pid=%lu tid=%lu bits=%u dpi=%u dll=%ls root_attached=%u\n",GetCurrentProcessId(),GetCurrentThreadId(),(unsigned)(sizeof(void *)*8),GetDpiForWindow(m->hwnd),dllpath,attached && attached(m->hwnd));
                EnumChildWindows(m->hwnd,inspect,0);
                fwprintf(logfile,L"SUMMARY buttons=%u edits=%u frames=%u attached=%u failures=%u\n",buttons,edits,frames,attached_count,failures);
                fclose(logfile);
            }
            GetEnvironmentVariableW(L"WIN16STYLE_VB_READY",event_name,128);
            HANDLE event=OpenEventW(EVENT_MODIFY_STATE,FALSE,event_name);
            if(event){SetEvent(event);CloseHandle(event);}busy=FALSE;
        }
    }
    return CallNextHookEx(NULL,code,p,l);
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved)
{(void)module;(void)reason;(void)reserved;return TRUE;}
