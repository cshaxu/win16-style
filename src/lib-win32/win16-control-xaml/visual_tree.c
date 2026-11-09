#define COBJMACROS
#include "visual_tree.h"
#include <roapi.h>
#include <winstring.h>

typedef struct winui_tree winui_tree;
typedef struct {
    HRESULT (STDMETHODCALLTYPE *query)(winui_tree *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *add_ref)(winui_tree *);
    ULONG (STDMETHODCALLTYPE *release)(winui_tree *);
    HRESULT (STDMETHODCALLTYPE *get_iids)(winui_tree *, ULONG *, IID **);
    HRESULT (STDMETHODCALLTYPE *get_name)(winui_tree *, HSTRING *);
    HRESULT (STDMETHODCALLTYPE *get_trust)(winui_tree *, TrustLevel *);
    void (STDMETHODCALLTYPE *unused[4])(void);
    HRESULT (STDMETHODCALLTYPE *get_child)(winui_tree *, IInspectable *, INT32, IInspectable **);
    HRESULT (STDMETHODCALLTYPE *get_count)(winui_tree *, IInspectable *, INT32 *);
    HRESULT (STDMETHODCALLTYPE *get_parent)(winui_tree *, IInspectable *, IInspectable **);
} winui_tree_vtable;
struct winui_tree { const winui_tree_vtable *vtable; };
static const IID tree_iid={0x5aece43c,0x7651,0x5bb5,{0x85,0x5c,0x21,0x98,0x49,0x6e,0x45,0x5e}};
static const IID dependency_iid={0xe7beaee7,0x160e,0x50f7,{0x87,0x89,0xd6,0x34,0x63,0xf9,0x79,0xfa}};

HRESULT win16_xaml_visit_ancestors(IInspectable *start, win16_xaml_ancestor_visitor visitor, void *context)
{
    static const wchar_t name_value[]=L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    if (!start || !visitor) return E_INVALIDARG;
    HSTRING name=NULL;
    winui_tree *tree=NULL;
    IInspectable *current=start;
    IInspectable_AddRef(current);
    HRESULT hr=WindowsCreateString(name_value,(UINT32)(ARRAYSIZE(name_value)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&tree_iid,(void **)&tree);
    unsigned int depth=0;
    for (;SUCCEEDED(hr) && current && depth<64;depth++) {
        if (!visitor(current,depth,context)) break;
        IInspectable *dependency=NULL,*parent=NULL;
        hr=IInspectable_QueryInterface(current,&dependency_iid,(void **)&dependency);
        /* XAML Diagnostics may report non-visual objects in the same tree. */
        if (hr==E_NOINTERFACE) hr=S_FALSE;
        if (hr==S_OK) hr=tree->vtable->get_parent(tree,dependency,&parent);
        if (dependency) IInspectable_Release(dependency);
        IInspectable_Release(current); current=parent;
    }
    if (SUCCEEDED(hr) && current && depth==64) hr=HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
    if (current) IInspectable_Release(current);
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
    return hr;
}

HRESULT win16_xaml_visit_descendants(IInspectable *start,win16_xaml_ancestor_visitor visitor,void *context)
{
    static const wchar_t name_value[]=L"Microsoft.UI.Xaml.Media.VisualTreeHelper";
    typedef struct item { IInspectable *object; unsigned int depth; } item;
    item stack[256]; unsigned int count=0,visited=0;
    if (!start || !visitor) return E_INVALIDARG;
    HSTRING name=NULL; winui_tree *tree=NULL;
    HRESULT hr=WindowsCreateString(name_value,(UINT32)(ARRAYSIZE(name_value)-1),&name);
    if (SUCCEEDED(hr)) hr=RoGetActivationFactory(name,&tree_iid,(void **)&tree);
    if (SUCCEEDED(hr)) { IInspectable_AddRef(start); stack[count++]=(item){start,0}; }
    while (SUCCEEDED(hr) && count && visited++<2048) {
        item current=stack[--count];
        BOOL descend=visitor(current.object,current.depth,context);
        IInspectable *dependency=NULL;
        if (descend && current.depth<63) {
            hr=IInspectable_QueryInterface(current.object,&dependency_iid,(void **)&dependency);
            if (hr==E_NOINTERFACE) hr=S_FALSE;
            if (hr==S_OK) {
                INT32 children=0;
                hr=tree->vtable->get_count(tree,dependency,&children);
                if (SUCCEEDED(hr) && (children<0 || (unsigned int)children>ARRAYSIZE(stack)-count))
                    hr=HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                for (INT32 i=children-1;SUCCEEDED(hr) && i>=0;--i) {
                    IInspectable *child=NULL;
                    hr=tree->vtable->get_child(tree,dependency,i,&child);
                    if (SUCCEEDED(hr) && child) stack[count++]=(item){child,current.depth+1};
                }
            }
        }
        if (dependency) IInspectable_Release(dependency);
        IInspectable_Release(current.object);
    }
    if (SUCCEEDED(hr) && count && visited>2048) hr=HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    while (count) { --count; IInspectable_Release(stack[count].object); }
    if (tree) tree->vtable->release(tree);
    if (name) WindowsDeleteString(name);
    return hr;
}
