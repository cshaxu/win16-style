#include "lib-style/common/clip.h"
#include <stdio.h>

#define CHECK(value) do { if(!(value)){fprintf(stderr,"CLIP_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
    HBITMAP bitmap=CreateBitmap(64,64,1,32,NULL);CHECK(bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old && old!=HGDI_ERROR);
    POINT points[2]={{10,10},{30,30}},origin;
    CHECK(style_clip_covers(dc,points));
    HRGN clip=CreateRectRgn(0,0,40,40),hole=CreateRectRgn(15,15,20,20);CHECK(clip && hole);
    CHECK(SelectClipRgn(dc,clip)!=ERROR && style_clip_covers(dc,points));
    CHECK(SelectClipRgn(dc,hole)!=ERROR && !style_clip_covers(dc,points));
    CHECK(CombineRgn(clip,clip,hole,RGN_DIFF)!=ERROR && SelectClipRgn(dc,clip)!=ERROR);
    CHECK(!style_clip_covers(dc,points));
    CHECK(SetViewportOrgEx(dc,3,4,NULL) && SetWindowOrgEx(dc,1,2,NULL));
    CHECK(!style_clip_covers(dc,points));
    CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4);
    CHECK(GetWindowOrgEx(dc,&origin) && origin.x==1 && origin.y==2);
    style_clip_snapshot snapshot;
    style_clip_snapshot_capture(&snapshot,dc);CHECK(snapshot.region && snapshot.kind==1);
    CHECK(SelectClipRgn(dc,NULL)!=ERROR && style_clip_snapshot_apply(&snapshot,dc));
    CHECK(!style_clip_covers(dc,points));
    SetLastError(1234);style_clip_snapshot_dispose(&snapshot);
    CHECK(!snapshot.region && snapshot.kind==-1 && GetLastError()==1234);
    CHECK(SelectClipRgn(dc,NULL)!=ERROR);
    style_clip_snapshot_capture(&snapshot,dc);CHECK(snapshot.region && snapshot.kind==0);
    CHECK(SelectClipRgn(dc,hole)!=ERROR && style_clip_snapshot_apply(&snapshot,dc));
    CHECK(style_clip_covers(dc,points));style_clip_snapshot_dispose(&snapshot);
    style_clip_snapshot_capture(&snapshot,NULL);CHECK(snapshot.region && snapshot.kind==-1);
    CHECK(!style_clip_snapshot_apply(&snapshot,dc));style_clip_snapshot_dispose(&snapshot);
    CHECK(SelectClipRgn(dc,NULL)!=ERROR && DeleteObject(hole) && DeleteObject(clip));
    CHECK(SelectObject(dc,old)==bitmap && DeleteObject(bitmap) && DeleteDC(dc));
    puts("CLIP_OK full=1 partial=1 hole=1 mapping_restored=1 snapshot=1");return 0;
}
