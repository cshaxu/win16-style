#define COBJMACROS
#include "native.h"
#include <uiautomation.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

static BOOL class_is(HWND window,PCWSTR name)
{WCHAR value[80];return GetClassNameW(window,value,ARRAYSIZE(value))&&!wcscmp(value,name);}
HWND style_uwp_title(HWND frame,HWND *auxiliary)
{
    HWND best=NULL;LONG width=0;*auxiliary=NULL;
    for(HWND child=GetWindow(frame,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)){
        RECT r;
        if(IsWindowVisible(child)&&class_is(child,L"ApplicationFrameTitleBarWindow")&&GetWindowRect(child,&r)&&r.right-r.left>width){best=child;width=r.right-r.left;}
    }
    for(HWND child=GetWindow(frame,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT))
        if(child!=best&&IsWindowVisible(child)&&class_is(child,L"ApplicationFrameTitleBarWindow")){
            if(*auxiliary)return NULL;
            *auxiliary=child;
        }
    return best;
}
static HRESULT fullscreen_pattern(HWND window,IUIAutomationInvokePattern **pattern)
{
    *pattern=NULL;
    if(!class_is(window,L"ApplicationFrameTitleBarWindow"))return E_INVALIDARG;
    IUIAutomation2 *automation=NULL;IUIAutomationElement *title=NULL,*button=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_CUIAutomation8,NULL,CLSCTX_INPROC_SERVER,&IID_IUIAutomation2,(void**)&automation);
    if(SUCCEEDED(hr))hr=IUIAutomation2_put_ConnectionTimeout(automation,1000);
    if(SUCCEEDED(hr))hr=IUIAutomation2_put_TransactionTimeout(automation,1000);
    if(SUCCEEDED(hr))hr=IUIAutomation2_ElementFromHandle(automation,window,&title);
    const PCWSTR ids[]={L"EnterFullScreen",L"ExitFullScreen"};
    for(unsigned i=0;SUCCEEDED(hr)&&!button&&i<ARRAYSIZE(ids);++i){
        IUIAutomationCondition *condition=NULL;VARIANT id={.vt=VT_BSTR};
        id.bstrVal=SysAllocString(ids[i]);
        hr=id.bstrVal?IUIAutomation2_CreatePropertyCondition(automation,UIA_AutomationIdPropertyId,id,&condition):E_OUTOFMEMORY;
        if(SUCCEEDED(hr))hr=IUIAutomationElement_FindFirst(title,TreeScope_Children,condition,&button);
        if(condition)IUIAutomationCondition_Release(condition);
        VariantClear(&id);
    }
    if(SUCCEEDED(hr))hr=button?IUIAutomationElement_GetCurrentPatternAs(button,UIA_InvokePatternId,&IID_IUIAutomationInvokePattern,(void**)pattern):S_FALSE;
    if(hr==S_OK&&!*pattern)hr=S_FALSE;
    if(button)IUIAutomationElement_Release(button);
    if(title)IUIAutomationElement_Release(title);
    if(automation)IUIAutomation2_Release(automation);
    return hr;
}
HRESULT style_uwp_fullscreen_available(HWND title)
{
    if(GetWindowThreadProcessId(title,NULL)==GetCurrentThreadId())return RPC_E_WRONG_THREAD;
    IUIAutomationInvokePattern *pattern=NULL;HRESULT hr=fullscreen_pattern(title,&pattern);
    if(pattern)IUIAutomationInvokePattern_Release(pattern);
    return hr;
}
typedef struct fullscreen_request {HWND frame,title;DWORD thread;HMODULE module;} fullscreen_request;
static BOOL request_valid(const fullscreen_request *r)
{
    DWORD pid=0;
    return GetWindowThreadProcessId(r->frame,&pid)==r->thread&&pid==GetCurrentProcessId()&&
        GetParent(r->title)==r->frame&&GetWindowThreadProcessId(r->title,NULL)==r->thread&&
        class_is(r->frame,L"ApplicationFrameWindow")&&class_is(r->title,L"ApplicationFrameTitleBarWindow");
}
static DWORD WINAPI fullscreen_worker(void *parameter)
{
    fullscreen_request *r=parameter;HMODULE module=r->module;
    HRESULT initialized=CoInitializeEx(NULL,COINIT_MULTITHREADED),hr=initialized;
    IUIAutomationInvokePattern *pattern=NULL;
    if(SUCCEEDED(hr))hr=request_valid(r)?fullscreen_pattern(r->title,&pattern):E_INVALIDARG;
    if(hr==S_OK)hr=request_valid(r)?IUIAutomationInvokePattern_Invoke(pattern):E_INVALIDARG;
    if(pattern)IUIAutomationInvokePattern_Release(pattern);
    if(SUCCEEDED(initialized))CoUninitialize();
    if(hr!=S_OK)OutputDebugStringW(L"style: native UWP full-screen action unavailable or failed\n");
    free(r);FreeLibraryAndExitThread(module,(DWORD)hr);
}
void style_uwp_fullscreen(HWND frame,void *title)
{
    fullscreen_request *r=calloc(1,sizeof(*r));
    if(!r){OutputDebugStringW(L"style: UWP full-screen request allocation failed\n");return;}
    *r=(fullscreen_request){frame,(HWND)title,GetCurrentThreadId(),NULL};
    if(!request_valid(r)||!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (PCWSTR)(uintptr_t)style_uwp_fullscreen,&r->module)){free(r);return;}
    HANDLE thread=CreateThread(NULL,0,fullscreen_worker,r,0,NULL);
    if(thread)CloseHandle(thread);
    else {FreeLibrary(r->module);free(r);OutputDebugStringW(L"style: UWP full-screen worker creation failed\n");}
}
