/* IOleWindow facade for the native or currently displayed classic dialog. */
#define COBJMACROS
#include "ole_window.h"

static HRESULT STDMETHODCALLTYPE ole_query(IOleWindow *self,REFIID id,void **out)
{win16_dialog_ole_window *p=(win16_dialog_ole_window *)self;return p->services->query(p->owner,id,out);}
static ULONG STDMETHODCALLTYPE ole_add(IOleWindow *self)
{win16_dialog_ole_window *p=(win16_dialog_ole_window *)self;return p->services->add(p->owner);}
static ULONG STDMETHODCALLTYPE ole_release(IOleWindow *self)
{win16_dialog_ole_window *p=(win16_dialog_ole_window *)self;return p->services->release(p->owner);}
static HRESULT STDMETHODCALLTYPE ole_window(IOleWindow *self,HWND *window)
{
    win16_dialog_ole_window *p=(win16_dialog_ole_window *)self;
    if(!window)return E_POINTER;
    if(*p->active_window && IsWindow(*p->active_window)){*window=*p->active_window;return S_OK;}
    return IOleWindow_GetWindow(p->native,window);
}
static HRESULT STDMETHODCALLTYPE ole_help(IOleWindow *self,BOOL enter)
{win16_dialog_ole_window *p=(win16_dialog_ole_window *)self;return IOleWindow_ContextSensitiveHelp(p->native,enter);}
static IOleWindowVtbl ole_vtable={ole_query,ole_add,ole_release,ole_window,ole_help};

void win16_dialog_ole_window_init(win16_dialog_ole_window *adapter,void *owner,IOleWindow *native,
    HWND *active_window,const win16_dialog_customize_services *services)
{
    adapter->iface.lpVtbl=&ole_vtable;adapter->owner=owner;adapter->native=native;
    adapter->active_window=active_window;adapter->services=services;
}
