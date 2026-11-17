#include "lib-win32-style/application/control/spin.h"
#include "lib-win32-style/application/graphics/bitmap.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SPIN_SCOPE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int main(void)
{
    style_instance instance={0};style_control_spin_state state={.modern=TRUE};
    HWND window=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,16,16,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(window);
    HDC dc=GetDC(window);CHECK(dc);style_bitmap bitmap={0};CHECK(style_bitmap_ensure(&bitmap,dc,(SIZE){16,16}));
    style_control_spin_scope outer={.instance=&instance,.state=&state,.window=window},inner={.instance=&instance,.state=&state,.window=window,.previous_stack=&outer};
    HDC clones[17];for(unsigned i=0;i<17;++i){clones[i]=CreateCompatibleDC(dc);CHECK(clones[i]);style_control_spin_dc_created(&inner,dc,clones[i]);}
    for(unsigned i=0;i<16;++i)CHECK(inner.clones[i]==clones[i]);
    style_control_spin_dc_deleted(&inner,clones[0]);CHECK(!inner.clones[0]);
    state.modern=FALSE;style_control_spin_dc_created(&inner,dc,clones[16]);CHECK(!inner.clones[0]);state.modern=TRUE;
    inner.dc=bitmap.dc;style_control_spin_dc_created(&inner,bitmap.dc,clones[16]);CHECK(inner.clones[0]==clones[16]);
    outer.buffers=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*outer.buffers));inner.buffers=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*inner.buffers));CHECK(outer.buffers && inner.buffers);
    inner.snapshot_dc=bitmap.dc;inner.snapshot_bitmap=bitmap.bitmap;inner.states[0]=1;inner.states[1]=4;memset(inner.buffers->valid,1,sizeof(inner.buffers->valid));
    RECT upper={0,0,16,8};style_control_spin_theme_begin(&inner,bitmap.dc,&upper);CHECK(!inner.states[0] && inner.states[1]==4 && inner.snapshot_dc==bitmap.dc);
    for(int y=0;y<16;++y)for(int x=0;x<16;++x)CHECK(inner.buffers->valid[y*64+x]==(y<8?0:1));
    RECT wrong={1,0,16,8};style_control_spin_theme_begin(&inner,bitmap.dc,&wrong);CHECK(!inner.snapshot_dc && !inner.snapshot_bitmap && !inner.states[1]);
    for(unsigned i=0;i<sizeof(inner.buffers->valid);++i)CHECK(!inner.buffers->valid[i]);
    inner.snapshot_dc=bitmap.dc;inner.snapshot_bitmap=bitmap.bitmap;inner.states[0]=1;inner.states[1]=4;
    style_control_spin_theme_begin(&inner,bitmap.dc,(const RECT *)(UINT_PTR)1);CHECK(!inner.snapshot_dc && !inner.states[0] && !inner.states[1]);
    HBITMAP alternate=CreateBitmap(16,16,1,32,NULL);CHECK(alternate);CHECK(SelectObject(bitmap.dc,alternate)==bitmap.bitmap);
    inner.snapshot_dc=bitmap.dc;inner.snapshot_bitmap=bitmap.bitmap;inner.states[0]=1;style_control_spin_selected(&inner,bitmap.dc);CHECK(!inner.snapshot_dc && !inner.states[0]);
    CHECK(SelectObject(bitmap.dc,bitmap.bitmap)==alternate);CHECK(DeleteObject(alternate));
    inner.snapshot_dc=outer.snapshot_dc=bitmap.dc;inner.snapshot_bitmap=outer.snapshot_bitmap=bitmap.bitmap;
    inner.states[0]=outer.states[0]=1;style_control_spin_object_deleted(&inner,bitmap.bitmap);CHECK(!inner.snapshot_dc && !outer.snapshot_dc && !inner.states[0] && !outer.states[0]);
    inner.snapshot_dc=outer.snapshot_dc=clones[2];inner.snapshot_bitmap=outer.snapshot_bitmap=bitmap.bitmap;outer.clones[0]=clones[2];
    style_control_spin_dc_deleted(&inner,clones[2]);CHECK(!inner.clones[2] && !outer.clones[0] && !inner.snapshot_dc && !outer.snapshot_dc);
    style_control_spin_buffers_clear(&inner);style_control_spin_buffers_clear(&outer);CHECK(!inner.buffers && !outer.buffers);style_control_spin_buffers_clear(&inner);
    for(unsigned i=0;i<17;++i)CHECK(GetObjectType(clones[i])==OBJ_MEMDC && DeleteDC(clones[i]));
    style_bitmap_dispose(&bitmap);CHECK(ReleaseDC(window,dc));CHECK(DestroyWindow(window));
    puts("SPIN_SCOPE PASS clone_cap=16 borrowed_dc=1 print_source=1 half_invalidate=1 invalid_rect=1 bitmap_select=1 nested_delete=1 buffer_owner=1 no_style=1 cleanup=1");return 0;
}
