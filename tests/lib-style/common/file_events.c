#define COBJMACROS
#include <windows.h>
#include <stdio.h>
#include "lib-style/common/file_events.h"
static unsigned calls;
static HRESULT STDMETHODCALLTYPE event_ok(IFileDialogEvents *self,IFileDialog *dialog)
{(void)self;if(dialog!=(IFileDialog *)(UINT_PTR)1)return E_FAIL;++calls;return S_FALSE;}
int main(void)
{
    IFileDialogEventsVtbl vtable={0};vtable.OnFileOk=event_ok;
    IFileDialogEvents listener={.lpVtbl=&vtable};
    style_dialog_event_adapter first={.refs=1,.owner=(void *)(UINT_PTR)1,.target=&listener};
    style_dialog_event_adapter second={.refs=1,.owner=(void *)(UINT_PTR)2,.target=&listener};first.next=&second;
    style_dialog_event_snapshot snapshot={0};
    if(!style_dialog_event_snapshot_init(&first,&snapshot))return 1;
    BOOL ok=snapshot.count==2 && first.refs==2 && second.refs==2 &&
        style_dialog_event_dispatch(&snapshot,(void *)(UINT_PTR)1,STYLE_CLASSIC_OK,NULL)==S_FALSE && calls==1;
    first.next=NULL;second.owner=NULL;style_dialog_event_snapshot_free(&snapshot);
    if(!ok || first.refs!=1 || second.refs!=1 || snapshot.items || snapshot.count)return 2;
    puts("FILE EVENTS PASS snapshot=1 refs=1 owner=1 veto=1");return 0;
}
