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
    WCHAR cls[80],real[80],text[256],module[MAX_PATH];LOGFONTW lf={0};RECT r;
    HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);BOOL own=attached && attached(w);
    (void)context;GetClassNameW(w,cls,80);GetWindowTextW(w,text,256);GetWindowRect(w,&r);
    module[0]=0;GetModuleFileNameW((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE),module,MAX_PATH);
    if(font)GetObjectW(font,sizeof(lf),&lf);
    RealGetWindowClassW(w,real,80);WNDCLASSEXW wc={.cbSize=sizeof(wc)};
    GetClassInfoExW((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE),cls,&wc);
    fwprintf(logfile,L"DETAIL real=%ls classproc=%p wndextra=%d classstyle=%08x unicode=%u dlg=%llx\n",real,(void *)wc.lpfnWndProc,wc.cbWndExtra,wc.style,IsWindowUnicode(w),(long long)SendMessageW(w,WM_GETDLGCODE,0,0));
    fwprintf(logfile,L"CONTROL class=%ls text=%ls attached=%u style=%08lx ex=%08lx size=%ld,%ld font=%ls height=%ld owner=%ls\n",
        cls,text,own,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),
        r.right-r.left,r.bottom-r.top,lf.lfFaceName,lf.lfHeight,module);
    WCHAR kind[64];RealGetWindowClassW(w,kind,64);
    if(!lstrcmpiW(kind,L"BUTTON")){++buttons;attached_count+=own;if(attached && !own)++failures;}
    if(!lstrcmpiW(kind,L"EDIT")){++edits;attached_count+=own;if(attached && !own)++failures;}
    if(!lstrcmpiW(kind,L"COMBOBOX")){++frames;attached_count+=own;if(attached && !own)++failures;}
    return TRUE;
}
static HWND proof_button;
static BOOL CALLBACK find_button(HWND w,LPARAM unused)
{WCHAR cls[64],text[64];(void)unused;GetClassNameW(w,cls,64);GetWindowTextW(w,text,64);
 if(!wcscmp(cls,L"TButton") && !wcscmp(text,L"Run")){proof_button=w;return FALSE;}return TRUE;}
static void draw_proof(HWND root)
{
    proof_button=NULL;EnumChildWindows(root,find_button,0);HWND button=proof_button;if(!button){++failures;fwprintf(logfile,L"DRAW_PROOF missing_button=1\n");return;}
    HDC dc=CreateCompatibleDC(NULL);BITMAPINFO bi={0};void *bits=NULL;bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth=75;bi.bmiHeader.biHeight=-25;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    HBITMAP bm=dc?CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&bits,NULL,0):NULL;
    if(!dc || !bm){if(bm)DeleteObject(bm);if(dc)DeleteDC(dc);++failures;return;}
    HGDIOBJ old=SelectObject(dc,bm);
    DWORD prior[75*25];ZeroMemory(bits,sizeof(prior));SendMessageW(button,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();CopyMemory(prior,bits,sizeof(prior));
    BOOL (__cdecl *remove)(HWND)=(void *)GetProcAddress(GetModuleHandleW(L"ntw16s32.dll"),"Ntw16sWindowRemove");
    BOOL removed=remove && remove(button);
    ZeroMemory(bits,sizeof(prior));SendMessageW(button,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();unsigned changed=0;
    for(unsigned i=0;i<75*25;++i)changed+=prior[i]!=((DWORD *)bits)[i];
    if(!removed || !changed || (prior[12*75+12]&0xffffff)!=0xc0c0c0 || attached(button))++failures;
    fwprintf(logfile,L"DK_RENDER_OK removed=%u changed=%u before_center=%08lx after_center=%08lx\n",removed,changed,prior[12*75+12],((DWORD *)bits)[12*75+12]);
    SelectObject(dc,old);DeleteObject(bm);DeleteDC(dc);
}
__declspec(dllexport) LRESULT CALLBACK DkInspect(int code,WPARAM p,LPARAM l)
{
    if(code>=0 && !busy){
        const CWPSTRUCT *m=(const CWPSTRUCT *)l;
        if(m->message==WM_APP+0x371){
            WCHAR path[MAX_PATH],event_name[128],dllpath[MAX_PATH]={0};HMODULE dll;
            busy=TRUE;buttons=edits=frames=attached_count=failures=0;
            GetEnvironmentVariableW(L"WIN16STYLE_DK_LOG",path,MAX_PATH);
            WCHAR suffix[MAX_PATH];swprintf_s(suffix,MAX_PATH,L"%ls-%u.txt",path,(unsigned)m->wParam);
            logfile=_wfopen(suffix,L"w");dll=GetModuleHandleW(L"ntw16s32.dll");
            attached=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
            if(dll)GetModuleFileNameW(dll,dllpath,MAX_PATH);
            if(logfile){
                fwprintf(logfile,L"PROCESS pid=%lu tid=%lu bits=%u dpi=%u dll=%ls root_attached=%u\n",GetCurrentProcessId(),GetCurrentThreadId(),(unsigned)(sizeof(void *)*8),GetDpiForWindow(m->hwnd),dllpath,attached && attached(m->hwnd));
                {HWND page=FindWindowExW(m->hwnd,NULL,L"TPageControl",NULL);
                    if(page)SendMessageW(page,TCM_SETCURFOCUS,m->wParam,0);}
                EnumChildWindows(m->hwnd,inspect,0);
                if(m->lParam && attached)draw_proof(m->hwnd);
                fwprintf(logfile,L"DK_CLASSIFIED page=%u\n",(unsigned)m->wParam);
                fwprintf(logfile,L"SUMMARY buttons=%u edits=%u combos=%u attached=%u failures=%u\n",buttons,edits,frames,attached_count,failures);
                fclose(logfile);
            }
            GetEnvironmentVariableW(L"WIN16STYLE_DK_READY",event_name,128);
            HANDLE event=OpenEventW(EVENT_MODIFY_STATE,FALSE,event_name);
            if(event){SetEvent(event);CloseHandle(event);}busy=FALSE;
        }
    }
    return CallNextHookEx(NULL,code,p,l);
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved)
{(void)module;(void)reason;(void)reserved;return TRUE;}
