#define COBJMACROS
#include "button_style.h"

struct win16_xaml_button_entry {
    struct win16_xaml_button_entry *next;
    IUnknown *identity;
    BOOL installing;
    win16_xaml_button_style style;
};

HRESULT win16_xaml_button_set_add(win16_xaml_button_set *set, IVisualTreeService *service,
                                  IXamlDiagnostics *diagnostics, IInspectable *button,
                                  win16_xaml_button_style **state)
{
    if (!set || !service || !diagnostics || !button || !state) return E_INVALIDARG;
    *state=NULL;
    DWORD thread=GetCurrentThreadId();
    if (set->thread_id && set->thread_id!=thread) return RPC_E_WRONG_THREAD;
    IUnknown *identity=NULL;
    HRESULT hr=IInspectable_QueryInterface(button,&IID_IUnknown,(void **)&identity);
    if (FAILED(hr)) return hr;
    for (win16_xaml_button_entry *entry=set->first;entry;entry=entry->next) {
        if (entry->identity==identity) {
            IUnknown_Release(identity);
            if (entry->installing) return HRESULT_FROM_WIN32(ERROR_BUSY);
            *state=&entry->style; return S_FALSE;
        }
    }
    win16_xaml_button_entry *entry=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*entry));
    if (!entry) { IUnknown_Release(identity); return E_OUTOFMEMORY; }
    entry->identity=identity;
    entry->installing=TRUE; entry->next=set->first; set->first=entry; set->thread_id=thread;
    hr=win16_xaml_button_apply(service,diagnostics,button,&entry->style);
    entry->installing=FALSE;
    if (FAILED(hr)) {
        win16_xaml_button_entry **link=&set->first;
        while (*link && *link!=entry) link=&(*link)->next;
        if (*link) *link=entry->next;
        IUnknown_Release(identity); HeapFree(GetProcessHeap(),0,entry);
        if (!set->first) set->thread_id=0;
        return hr;
    }
    *state=&entry->style;
    return S_OK;
}

HRESULT win16_xaml_button_set_remove(win16_xaml_button_set *set, IVisualTreeService *service,
                                     IXamlDiagnostics *diagnostics, win16_xaml_button_style *state)
{
    if (!set || !service || !diagnostics || !state) return E_INVALIDARG;
    if (set->thread_id && set->thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    win16_xaml_button_entry **link=&set->first;
    while (*link && &(*link)->style!=state) link=&(*link)->next;
    if (!*link) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    win16_xaml_button_entry *entry=*link;
    if (entry->installing) return HRESULT_FROM_WIN32(ERROR_BUSY);
    entry->installing=TRUE;
    HRESULT hr=win16_xaml_button_remove(service,diagnostics,&entry->style);
    entry->installing=FALSE;
    if (FAILED(hr)) return hr;
    /* A nested callback may have inserted entries before this one. */
    link=&set->first;
    while (*link && *link!=entry) link=&(*link)->next;
    if (!*link) return E_UNEXPECTED;
    *link=entry->next; IUnknown_Release(entry->identity); HeapFree(GetProcessHeap(),0,entry);
    if (!set->first) set->thread_id=0;
    return S_OK;
}

HRESULT win16_xaml_button_set_remove_all(win16_xaml_button_set *set, IVisualTreeService *service,
                                         IXamlDiagnostics *diagnostics)
{
    if (!set || !service || !diagnostics) return E_INVALIDARG;
    if (set->thread_id && set->thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    while (set->first) {
        HRESULT hr=win16_xaml_button_set_remove(set,service,diagnostics,&set->first->style);
        if (FAILED(hr)) return hr;
    }
    return S_OK;
}

HRESULT win16_xaml_button_set_discard(win16_xaml_button_set *set)
{
    if (!set) return E_INVALIDARG;
    if (set->thread_id && set->thread_id!=GetCurrentThreadId()) return RPC_E_WRONG_THREAD;
    for (win16_xaml_button_entry *entry=set->first;entry;entry=entry->next)
        if (entry->installing) return HRESULT_FROM_WIN32(ERROR_BUSY);
    while (set->first) {
        win16_xaml_button_entry *entry=set->first; set->first=entry->next;
        win16_xaml_button_discard(&entry->style);
        IUnknown_Release(entry->identity); HeapFree(GetProcessHeap(),0,entry);
    }
    set->thread_id=0;
    return S_OK;
}
