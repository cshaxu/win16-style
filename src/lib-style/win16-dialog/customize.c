/* In-process COM customization facade; owner retains identity and lifetime. */
#define COBJMACROS
#include "customize.h"

static HRESULT STDMETHODCALLTYPE custom_QueryInterface(IFileDialogCustomize *This, REFIID riid, void **ppvObject)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;return p->services->query(p->owner,riid,ppvObject);
}

static ULONG STDMETHODCALLTYPE custom_AddRef(IFileDialogCustomize *This)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;return p->services->add(p->owner);
}

static ULONG STDMETHODCALLTYPE custom_Release(IFileDialogCustomize *This)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;return p->services->release(p->owner);
}

/* Setters disable classic bridging before dispatch; getters do so only on success. */
#define CUSTOM_SET(name, signature, args) \
static HRESULT STDMETHODCALLTYPE custom_##name signature \
{win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_##name args;}
#define CUSTOM_GET(name, signature, args) \
static HRESULT STDMETHODCALLTYPE custom_##name signature \
{win16_dialog_customize *p=(win16_dialog_customize *)This;HRESULT hr=IFileDialogCustomize_##name args; \
 if(SUCCEEDED(hr)){*p->unsupported=TRUE;}return hr;}

CUSTOM_SET(EnableOpenDropDown, (IFileDialogCustomize *This, DWORD dwIDCtl), (p->native ,dwIDCtl))
CUSTOM_SET(AddMenu, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel), (p->native ,dwIDCtl,pszLabel))
CUSTOM_SET(AddPushButton, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel), (p->native ,dwIDCtl,pszLabel))
CUSTOM_SET(AddComboBox, (IFileDialogCustomize *This, DWORD dwIDCtl), (p->native ,dwIDCtl))
CUSTOM_SET(AddRadioButtonList, (IFileDialogCustomize *This, DWORD dwIDCtl), (p->native ,dwIDCtl))
CUSTOM_SET(AddCheckButton, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel, WINBOOL bChecked), (p->native ,dwIDCtl,pszLabel,bChecked))
CUSTOM_SET(AddEditBox, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText), (p->native ,dwIDCtl,pszText))
CUSTOM_SET(AddSeparator, (IFileDialogCustomize *This, DWORD dwIDCtl), (p->native ,dwIDCtl))
CUSTOM_SET(AddText, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText), (p->native ,dwIDCtl,pszText))
CUSTOM_SET(SetControlLabel, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel), (p->native ,dwIDCtl,pszLabel))
CUSTOM_GET(GetControlState, (IFileDialogCustomize *This, DWORD dwIDCtl, CDCONTROLSTATEF *pdwState), (p->native ,dwIDCtl,pdwState))
CUSTOM_SET(SetControlState, (IFileDialogCustomize *This, DWORD dwIDCtl, CDCONTROLSTATEF dwState), (p->native ,dwIDCtl,dwState))
CUSTOM_GET(GetEditBoxText, (IFileDialogCustomize *This, DWORD dwIDCtl, WCHAR **ppszText), (p->native ,dwIDCtl,ppszText))
CUSTOM_SET(SetEditBoxText, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText), (p->native ,dwIDCtl,pszText))
CUSTOM_GET(GetCheckButtonState, (IFileDialogCustomize *This, DWORD dwIDCtl, WINBOOL *pbChecked), (p->native ,dwIDCtl,pbChecked))
CUSTOM_SET(SetCheckButtonState, (IFileDialogCustomize *This, DWORD dwIDCtl, WINBOOL bChecked), (p->native ,dwIDCtl,bChecked))
CUSTOM_SET(AddControlItem, (IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, LPCWSTR pszLabel), (p->native ,dwIDCtl,dwIDItem,pszLabel))
CUSTOM_SET(RemoveControlItem, (IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem), (p->native ,dwIDCtl,dwIDItem))
CUSTOM_SET(RemoveAllControlItems, (IFileDialogCustomize *This, DWORD dwIDCtl), (p->native ,dwIDCtl))
CUSTOM_GET(GetControlItemState, (IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, CDCONTROLSTATEF *pdwState), (p->native ,dwIDCtl,dwIDItem,pdwState))
CUSTOM_SET(SetControlItemState, (IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, CDCONTROLSTATEF dwState), (p->native ,dwIDCtl,dwIDItem,dwState))
CUSTOM_GET(GetSelectedControlItem, (IFileDialogCustomize *This, DWORD dwIDCtl, DWORD *pdwIDItem), (p->native ,dwIDCtl,pdwIDItem))
CUSTOM_SET(SetSelectedControlItem, (IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem), (p->native ,dwIDCtl,dwIDItem))
CUSTOM_SET(StartVisualGroup, (IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel), (p->native ,dwIDCtl,pszLabel))
CUSTOM_SET(EndVisualGroup, (IFileDialogCustomize *This), (p->native))
CUSTOM_SET(MakeProminent, (IFileDialogCustomize *This, DWORD dwIDCtl), (p->native ,dwIDCtl))
CUSTOM_SET(SetControlItemText, (IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, LPCWSTR pszLabel), (p->native ,dwIDCtl,dwIDItem,pszLabel))
#undef CUSTOM_SET
#undef CUSTOM_GET

static IFileDialogCustomizeVtbl custom_vtable={
    custom_QueryInterface,
    custom_AddRef,
    custom_Release,
    custom_EnableOpenDropDown,
    custom_AddMenu,
    custom_AddPushButton,
    custom_AddComboBox,
    custom_AddRadioButtonList,
    custom_AddCheckButton,
    custom_AddEditBox,
    custom_AddSeparator,
    custom_AddText,
    custom_SetControlLabel,
    custom_GetControlState,
    custom_SetControlState,
    custom_GetEditBoxText,
    custom_SetEditBoxText,
    custom_GetCheckButtonState,
    custom_SetCheckButtonState,
    custom_AddControlItem,
    custom_RemoveControlItem,
    custom_RemoveAllControlItems,
    custom_GetControlItemState,
    custom_SetControlItemState,
    custom_GetSelectedControlItem,
    custom_SetSelectedControlItem,
    custom_StartVisualGroup,
    custom_EndVisualGroup,
    custom_MakeProminent,
    custom_SetControlItemText
};

void win16_dialog_customize_init(win16_dialog_customize *adapter,void *owner,IFileDialogCustomize *native,
    BOOL *unsupported,const win16_dialog_customize_services *services)
{
    adapter->iface.lpVtbl=&custom_vtable;adapter->owner=owner;adapter->native=native;
    adapter->unsupported=unsupported;adapter->services=services;
}
