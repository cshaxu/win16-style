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

static HRESULT STDMETHODCALLTYPE custom_EnableOpenDropDown(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_EnableOpenDropDown(p->native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddMenu(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddMenu(p->native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_AddPushButton(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddPushButton(p->native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_AddComboBox(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddComboBox(p->native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddRadioButtonList(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddRadioButtonList(p->native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddCheckButton(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel, WINBOOL bChecked)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddCheckButton(p->native ,dwIDCtl,pszLabel,bChecked);
}

static HRESULT STDMETHODCALLTYPE custom_AddEditBox(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddEditBox(p->native ,dwIDCtl,pszText);
}

static HRESULT STDMETHODCALLTYPE custom_AddSeparator(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddSeparator(p->native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_AddText(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddText(p->native ,dwIDCtl,pszText);
}

static HRESULT STDMETHODCALLTYPE custom_SetControlLabel(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_SetControlLabel(p->native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_GetControlState(IFileDialogCustomize *This, DWORD dwIDCtl, CDCONTROLSTATEF *pdwState)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;
    HRESULT hr=IFileDialogCustomize_GetControlState(p->native ,dwIDCtl,pdwState);
    if(SUCCEEDED(hr))*p->unsupported=TRUE;
    return hr;
}

static HRESULT STDMETHODCALLTYPE custom_SetControlState(IFileDialogCustomize *This, DWORD dwIDCtl, CDCONTROLSTATEF dwState)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_SetControlState(p->native ,dwIDCtl,dwState);
}

static HRESULT STDMETHODCALLTYPE custom_GetEditBoxText(IFileDialogCustomize *This, DWORD dwIDCtl, WCHAR **ppszText)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;
    HRESULT hr=IFileDialogCustomize_GetEditBoxText(p->native ,dwIDCtl,ppszText);
    if(SUCCEEDED(hr))*p->unsupported=TRUE;
    return hr;
}

static HRESULT STDMETHODCALLTYPE custom_SetEditBoxText(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszText)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_SetEditBoxText(p->native ,dwIDCtl,pszText);
}

static HRESULT STDMETHODCALLTYPE custom_GetCheckButtonState(IFileDialogCustomize *This, DWORD dwIDCtl, WINBOOL *pbChecked)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;
    HRESULT hr=IFileDialogCustomize_GetCheckButtonState(p->native ,dwIDCtl,pbChecked);
    if(SUCCEEDED(hr))*p->unsupported=TRUE;
    return hr;
}

static HRESULT STDMETHODCALLTYPE custom_SetCheckButtonState(IFileDialogCustomize *This, DWORD dwIDCtl, WINBOOL bChecked)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_SetCheckButtonState(p->native ,dwIDCtl,bChecked);
}

static HRESULT STDMETHODCALLTYPE custom_AddControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, LPCWSTR pszLabel)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_AddControlItem(p->native ,dwIDCtl,dwIDItem,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_RemoveControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_RemoveControlItem(p->native ,dwIDCtl,dwIDItem);
}

static HRESULT STDMETHODCALLTYPE custom_RemoveAllControlItems(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_RemoveAllControlItems(p->native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_GetControlItemState(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, CDCONTROLSTATEF *pdwState)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;
    HRESULT hr=IFileDialogCustomize_GetControlItemState(p->native ,dwIDCtl,dwIDItem,pdwState);
    if(SUCCEEDED(hr))*p->unsupported=TRUE;
    return hr;
}

static HRESULT STDMETHODCALLTYPE custom_SetControlItemState(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, CDCONTROLSTATEF dwState)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_SetControlItemState(p->native ,dwIDCtl,dwIDItem,dwState);
}

static HRESULT STDMETHODCALLTYPE custom_GetSelectedControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD *pdwIDItem)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;
    HRESULT hr=IFileDialogCustomize_GetSelectedControlItem(p->native ,dwIDCtl,pdwIDItem);
    if(SUCCEEDED(hr))*p->unsupported=TRUE;
    return hr;
}

static HRESULT STDMETHODCALLTYPE custom_SetSelectedControlItem(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_SetSelectedControlItem(p->native ,dwIDCtl,dwIDItem);
}

static HRESULT STDMETHODCALLTYPE custom_StartVisualGroup(IFileDialogCustomize *This, DWORD dwIDCtl, LPCWSTR pszLabel)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_StartVisualGroup(p->native ,dwIDCtl,pszLabel);
}

static HRESULT STDMETHODCALLTYPE custom_EndVisualGroup(IFileDialogCustomize *This)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_EndVisualGroup(p->native);
}

static HRESULT STDMETHODCALLTYPE custom_MakeProminent(IFileDialogCustomize *This, DWORD dwIDCtl)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_MakeProminent(p->native ,dwIDCtl);
}

static HRESULT STDMETHODCALLTYPE custom_SetControlItemText(IFileDialogCustomize *This, DWORD dwIDCtl, DWORD dwIDItem, LPCWSTR pszLabel)
{
    win16_dialog_customize *p=(win16_dialog_customize *)This;*p->unsupported=TRUE;return IFileDialogCustomize_SetControlItemText(p->native ,dwIDCtl,dwIDItem,pszLabel);
}

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
