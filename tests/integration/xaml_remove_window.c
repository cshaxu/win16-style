#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "ntw16s-dll/window_thread.h"

typedef struct probe { DWORD pid; HMODULE module,property_module; HOOKPROC callback,property_callback; uintptr_t adapter; BOOL removed,property_ok,checkbox,radio,textbox,combo,scroll,list,progress,existing,preexisting,remove_bridge,keep_fixture; } probe;
static uintptr_t send_property(HWND window,UINT message,WPARAM operation,uintptr_t adapter)
{
    DWORD_PTR ignored=0; RemovePropW(window,L"Win16Xaml.Test.Property.Result");
    if (!SendMessageTimeoutW(window,message,operation,(LPARAM)adapter,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored)) return 0;
    uintptr_t result=(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Result");
    RemovePropW(window,L"Win16Xaml.Test.Property.Result"); return result;
}
static uintptr_t send_window(HWND window,UINT message,WPARAM operation)
{
    DWORD_PTR ignored=0; RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
    if (!SendMessageTimeoutW(window,message,operation,(LPARAM)window,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored)) return 0;
    uintptr_t result=(uintptr_t)GetPropW(window,NTW16S_WINDOW_THREAD_RESULT);
    RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT); return result;
}
static void preexisting_probe(HWND window,probe *p,UINT property_message)
{
    UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
    uintptr_t result=send_window(window,message,NTW16S_WINDOW_THREAD_REMOVE);
    if (result!=1) { wprintf(L"initial_remove=%llu\n",(unsigned long long)result); return; }
    for (unsigned int i=0;i<50;i++) {
        if (send_property(window,property_message,8,p->adapter)==1) break;
        Sleep(100);
    }
    if (send_property(window,property_message,8,p->adapter)!=1) { wprintf(L"disconnect_timeout\n"); return; }
    result=send_property(window,property_message,7,p->adapter);
    wprintf(L"preexisting_create=%llu\n",(unsigned long long)result);
    if (result!=1) return;
    if (send_property(window,property_message,8,p->adapter)!=1) {
        wprintf(L"session_restarted_before_reattach\n");
        (void)send_property(window,property_message,2,p->adapter); return;
    }
    result=send_window(window,message,NTW16S_WINDOW_THREAD_ATTACH);
    wprintf(L"reattach=%llu\n",(unsigned long long)result);
    if (result==1) {
        uintptr_t source=0;
        for (unsigned int i=0;i<60;i++) {
            if (send_property(window,property_message,8,p->adapter)==2)
                source=send_property(window,property_message,4,p->adapter);
            if (source==5) break;
            Sleep(100);
        }
        wprintf(L"preexisting_source=%llu\n",(unsigned long long)source);
        if (p->keep_fixture) {
            uintptr_t fixture_attached=send_property(window,property_message,13,p->adapter);
            wprintf(L"fixture_attached=%llu fixture_hwnd=0x%llX fixture_host=%llu\n",(unsigned long long)fixture_attached,
                    (unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.FixtureHwnd"),
                    (unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.FixtureHost"));
            uintptr_t owner_ok=send_property(window,property_message,15,p->adapter);
            wprintf(L"owner_probe=%llu owner_hwnd=0x%llX\n",(unsigned long long)owner_ok,
                    (unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.OwnerHwnd"));
            RemovePropW(window,L"Win16Xaml.Test.OwnerHwnd");
            RemovePropW(window,L"Win16Xaml.Test.FixtureHwnd");
            RemovePropW(window,L"Win16Xaml.Test.FixtureHost");
        }
        if (!p->keep_fixture) wprintf(L"fixture_close=%llu\n",(unsigned long long)send_property(window,property_message,10,p->adapter));
        wprintf(L"host_recorded=%llu\n",(unsigned long long)send_property(window,property_message,12,p->adapter));
        uintptr_t main_before=p->keep_fixture ? send_property(window,property_message,14,p->adapter) : 0;
        if(p->keep_fixture)wprintf(L"main_button_before=%llu\n",(unsigned long long)main_before);
        if(p->keep_fixture){
            uintptr_t owner_ok=send_property(window,property_message,16,p->adapter);
            wprintf(L"main_owner_probe=%llu main_owner_hwnd=0x%llX\n",(unsigned long long)owner_ok,
                    (unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.OwnerHwnd"));
            RemovePropW(window,L"Win16Xaml.Test.OwnerHwnd");
        }
        if (p->remove_bridge) wprintf(L"bridge_destroy=%llu\n",(unsigned long long)send_property(window,property_message,11,p->adapter));
        wprintf(L"attached_before_remove=%llu\n",(unsigned long long)send_property(window,property_message,9,p->adapter));
        p->removed=send_window(window,message,NTW16S_WINDOW_THREAD_REMOVE)==1;
        wprintf(L"attached_after_remove=%llu\n",(unsigned long long)send_property(window,property_message,9,p->adapter));
        uintptr_t remaining=p->keep_fixture ? send_property(window,property_message,4,p->adapter) : 0;
        if (p->keep_fixture) wprintf(L"remaining_window_source=%llu\n",(unsigned long long)remaining);
        uintptr_t main_after=p->keep_fixture ? send_property(window,property_message,14,p->adapter) : 0;
        if(p->keep_fixture)wprintf(L"main_button_after=%llu\n",(unsigned long long)main_after);
        uintptr_t host_after=send_property(window,property_message,12,p->adapter);
        wprintf(L"host_after_remove=%llu\n",(unsigned long long)host_after);
        uintptr_t after=send_property(window,property_message,2,p->adapter);
        wprintf(L"preexisting_restore=%llu\n",(unsigned long long)after);
        p->property_ok=p->removed && source==5 && host_after==1 &&
            (p->keep_fixture ? remaining==5 && main_before==5 && main_after==2 : after==2);
    } else (void)send_property(window,property_message,2,p->adapter);
}
static BOOL CALLBACK visit(HWND window,LPARAM parameter)
{
    probe *p=(probe *)parameter; DWORD pid=0;
    DWORD thread=GetWindowThreadProcessId(window,&pid);
    if (pid!=p->pid || !thread || GetAncestor(window,GA_ROOT)!=window) return TRUE;
    WCHAR name[120]={0}; GetClassNameW(window,name,ARRAYSIZE(name));
    wprintf(L"window_class=%ls visible=%d thread=%lu hwnd=0x%llX\n",name,IsWindowVisible(window),(unsigned long)thread,(unsigned long long)(uintptr_t)window);
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,p->callback,p->module,thread);
    if (!hook) return TRUE;
    HHOOK property_hook=p->property_callback ? SetWindowsHookExW(WH_CALLWNDPROC,p->property_callback,p->property_module,thread) : NULL;
    UINT property_message=RegisterWindowMessageW(L"Win16Xaml.Test.Property");
    if (p->preexisting && property_hook && property_message) {
        preexisting_probe(window,p,property_message);
        UnhookWindowsHookEx(property_hook); UnhookWindowsHookEx(hook); return FALSE;
    }
    DWORD_PTR ignored=0;
    uintptr_t before=0,after=0,fixture_attached=0;
    if (property_hook && property_message) {
        RemovePropW(window,L"Win16Xaml.Test.Property.Result");
        if (SendMessageTimeoutW(window,property_message,p->existing?5:p->progress?29:p->list?27:p->scroll?24:p->combo?21:p->textbox?18:p->radio?17:p->checkbox?3:1,(LPARAM)p->adapter,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored))
            before=(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Result");
        RemovePropW(window,L"Win16Xaml.Test.Property.Result");
        if (p->checkbox) wprintf(L"checkbox_create_result=%llu\n",(unsigned long long)before);
        if (p->existing) {
            wprintf(L"checkbox_create_phase=%llu\n",(unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Phase"));
            RemovePropW(window,L"Win16Xaml.Test.Property.Phase");
            wprintf(L"ancestor_entries=%llu nodes=%llu max_depth=%llu\n",
                    (unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Entries"),
                    (unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Nodes"),
                    (unsigned long long)(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Depth"));
            RemovePropW(window,L"Win16Xaml.Test.Property.Entries");
            RemovePropW(window,L"Win16Xaml.Test.Property.Nodes");
            RemovePropW(window,L"Win16Xaml.Test.Property.Depth");
        }
        if (p->checkbox && before==1) {
            for (unsigned int i=0;i<40 && before!=5;i++) {
                Sleep(100);
                if (SendMessageTimeoutW(window,property_message,4,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored))
                    before=(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Result");
                RemovePropW(window,L"Win16Xaml.Test.Property.Result");
            }
        }
        if (p->checkbox && !p->existing) {
            fixture_attached=send_property(window,property_message,13,p->adapter);
            wprintf(L"fixture_attached=%llu\n",(unsigned long long)fixture_attached);
            if (p->combo) {
                uintptr_t opened=send_property(window,property_message,22,0),item_source=0;
                if (opened==1) for (unsigned int i=0;i<30 && item_source!=5;i++) {
                    Sleep(100); item_source=send_property(window,property_message,23,0);
                }
                wprintf(L"combo_open_result=%llu item_source=%llu\n",(unsigned long long)opened,(unsigned long long)item_source);
                if (opened!=1 || item_source!=5) before=0;
            }
            if (p->list) {
                uintptr_t created=send_property(window,property_message,28,0),item_source=0;
                if (created==1) for (unsigned int i=0;i<30 && item_source!=5;i++) {
                    Sleep(100); item_source=send_property(window,property_message,23,0);
                }
                wprintf(L"list_item_result=%llu item_source=%llu\n",(unsigned long long)created,(unsigned long long)item_source);
                if (created!=1 || item_source!=5) before=0;
            }
            if (p->textbox) {
                HWND fixture=(HWND)GetPropW(window,L"Win16Xaml.Test.FixtureHwnd");
                if (fixture && SetForegroundWindow(fixture) && GetForegroundWindow()==fixture &&
                    send_property(window,property_message,19,0)==2 && GetForegroundWindow()==fixture) {
                    INPUT input[2]={{0},{0}};
                    input[0].type=INPUT_KEYBOARD; input[0].ki.wScan=L'z'; input[0].ki.dwFlags=KEYEVENTF_UNICODE;
                    input[1]=input[0]; input[1].ki.dwFlags|=KEYEVENTF_KEYUP;
                    UINT sent=SendInput(2,input,sizeof(INPUT)); Sleep(300);
                    uintptr_t result=send_property(window,property_message,20,0);
                    wprintf(L"textbox_keyboard_sent=%u result=%llu\n",sent,(unsigned long long)result);
                    if (sent!=2 || result!=2) before=0;
                } else wprintf(L"SKIP textbox_keyboard_foreground\n");
            }
            RemovePropW(window,L"Win16Xaml.Test.FixtureHwnd");
            RemovePropW(window,L"Win16Xaml.Test.FixtureHost");
        }
    }
    UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
    RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
    if (message && SendMessageTimeoutW(window,message,NTW16S_WINDOW_THREAD_REMOVE,(LPARAM)window,
                                       SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored)) {
        uintptr_t result=(uintptr_t)GetPropW(window,NTW16S_WINDOW_THREAD_RESULT);
        wprintf(L"remove_result=%llu\n",(unsigned long long)result);
        if (result==1) p->removed=TRUE;
    }
    if (p->removed && property_hook && property_message) {
        if (SendMessageTimeoutW(window,property_message,2,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored))
            after=(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Result");
        wprintf(L"property_source_before=%llu after=%llu (encoded source+1)\n",(unsigned long long)before,(unsigned long long)after);
        p->property_ok=before==5 && (p->checkbox && !p->existing ? fixture_attached==2 && after==5 : after==2);
    }
    RemovePropW(window,L"Win16Xaml.Test.Property.Result");
    RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
    UnhookWindowsHookEx(hook);
    if (property_hook) UnhookWindowsHookEx(property_hook);
    return !p->removed;
}
int wmain(int argc,WCHAR **argv)
{
    if (argc!=3 && argc!=5 && argc!=6) return 2;
    probe p={0}; p.pid=wcstoul(argv[1],NULL,10); p.module=LoadLibraryW(argv[2]);
    if (!p.pid || !p.module) return 3;
    p.callback=(HOOKPROC)(uintptr_t)GetProcAddress(p.module,"Ntw16sWindowThreadHook");
    if (!p.callback) { FreeLibrary(p.module); return 4; }
    if (argc>=5) {
        p.adapter=(uintptr_t)wcstoull(argv[4],NULL,10);
        p.property_module=LoadLibraryW(argv[3]);
        if (p.property_module) p.property_callback=(HOOKPROC)(uintptr_t)GetProcAddress(p.property_module,"XamlPropertyProbe");
        if (!p.adapter || !p.property_callback) { if (p.property_module) FreeLibrary(p.property_module); FreeLibrary(p.module); return 5; }
        p.existing=argc==6 && !wcscmp(argv[5],L"--existing");
        p.preexisting=argc==6 && (!wcscmp(argv[5],L"--preexisting") || !wcscmp(argv[5],L"--no-bridge") || !wcscmp(argv[5],L"--keep-fixture"));
        p.remove_bridge=argc==6 && !wcscmp(argv[5],L"--no-bridge");
        p.keep_fixture=argc==6 && !wcscmp(argv[5],L"--keep-fixture");
        p.radio=argc==6 && !wcscmp(argv[5],L"--radio");
        p.textbox=argc==6 && !wcscmp(argv[5],L"--textbox");
        p.combo=argc==6 && !wcscmp(argv[5],L"--combo");
        p.scroll=argc==6 && !wcscmp(argv[5],L"--scroll");
        p.list=argc==6 && !wcscmp(argv[5],L"--list");
        p.progress=argc==6 && !wcscmp(argv[5],L"--progress");
        p.checkbox=p.existing || p.preexisting || p.radio || p.textbox || p.combo || p.scroll || p.list || p.progress || (argc==6 && !wcscmp(argv[5],L"--checkbox"));
    }
    EnumWindows(visit,(LPARAM)&p);
    if (p.property_module) FreeLibrary(p.property_module);
    FreeLibrary(p.module);
    return !p.removed ? 77 : argc>=5 && !p.property_ok ? 6 : 0;
}
