#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "lib-win32-style/application/menu/row_text.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(expr) do{if(!(expr)){fprintf(stderr,"line %d: %s\n",__LINE__,#expr);return 1;}}while(0)

static BOOL same_menu(const classic_menu_metrics *a,const classic_menu_metrics *b)
{
    return a->check_column==b->check_column && a->shortcut_gap==b->shortcut_gap &&
        a->arrow_column==b->arrow_column && a->plain_column==b->plain_column &&
        a->text_flags==b->text_flags && a->bar_padding==b->bar_padding &&
        a->mdi_buttons==b->mdi_buttons && a->row_minimum==b->row_minimum &&
        a->row_padding==b->row_padding && a->mdi_inactive==b->mdi_inactive;
}

static int fixtures(void)
{
    const classic_button_group original={2,1,3,{{CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0},
        {CLASSIC_BUTTON_CLOSE,CLASSIC_BUTTON_RIGHT,2}},0};
    classic_button_group group={0};
    unsigned char bytes[CLASSIC_STYLE_RESOURCE_CAPACITY],reference[CLASSIC_STYLE_RESOURCE_CAPACITY],
        second[CLASSIC_STYLE_RESOURCE_CAPACITY];
    size_t length=classic_style_write(reference,sizeof(reference),&original,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    CHECK(length==40 && !memcmp(reference,"CSTY\x01\0\0\0\x28\0\0\0\x01\0\0\0",16));
    CHECK(!classic_style_write(bytes,length-1,&original,NULL,NULL,NULL,NULL,NULL,NULL,NULL));
    for(size_t end=0;end<length;++end)
        CHECK(classic_style_read(reference,end,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID && !group.count);
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK);
    CHECK(classic_style_write(second,sizeof(second),&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==length && !memcmp(reference,second,length));

    classic_button_group explicit_group=original;
    explicit_group.layout=CLASSIC_BUTTON_LAYOUT_EXPLICIT|CLASSIC_BUTTON_LAYOUT_FILL;
    CHECK(classic_style_write(bytes,sizeof(bytes),&explicit_group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==length);
    CHECK(bytes[6]==28 && classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        group.layout==explicit_group.layout);
    bytes[6]=27;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        group.layout==explicit_group.layout);
    bytes[6]=28;bytes[30]=CLASSIC_BUTTON_LAYOUT_CLIP;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);

    memcpy(bytes,reference,length);
    bytes[4]=2;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_VERSION);
    bytes[4]=1;
    bytes[36]=bytes[32];
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    bytes[36]=reference[36];
    bytes[20]=0xff;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    bytes[20]=reference[20];

    memcpy(bytes+length,"\x6e\0\0\0\x01\0\0\0\xab",9);
    bytes[6]=1;
    bytes[8]=(unsigned char)(length+9);
    bytes[12]=2;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK && group.count==2);
    bytes[length+2]=1;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_REQUIRED_RECORD && group.count==2);
    bytes[16]=0x6e;
    bytes[18]=0;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    bytes[6]=0;
    bytes[8]=(unsigned char)length;
    bytes[12]=1;
    CHECK(classic_style_read(bytes,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_NO_BUTTON_GROUP);

    memcpy(bytes,reference,length);
    memcpy(bytes+length,bytes+16,length-16);
    bytes[12]=2;
    bytes[8]=(unsigned char)(length*2-16);
    CHECK(classic_style_read(bytes,length*2-16,&group,NULL,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    const classic_scroll_metrics fixed={CLASSIC_SCROLL_EXTENT_FIXED,11,STYLE_SCROLL_THUMB_DEFAULT,0,0,{0}};
    classic_scroll_metrics metrics={0},timed=fixed;
    timed.initial_repeat_ms=317;
    CHECK(!classic_style_write(bytes,sizeof(bytes),&original,&timed,NULL,NULL,NULL,NULL,NULL,NULL));
    timed.repeat_ms=67;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&timed,NULL,NULL,NULL,NULL,NULL,NULL)==length+24);
    CHECK(bytes[6]==29 && classic_style_read(bytes,length+24,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        metrics.initial_repeat_ms==317 && metrics.repeat_ms==67);
    bytes[length+22]=0;
    CHECK(classic_style_read(bytes,length+24,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        metrics.repeat_ms==67);
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed,NULL,NULL,NULL,NULL,NULL,NULL)==length+12);
    CHECK(classic_style_read(bytes,length+12,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        metrics.source==fixed.source && metrics.logical_extent==11 && metrics.thumb==STYLE_SCROLL_THUMB_DEFAULT);
    bytes[length+10]=0;
    CHECK(classic_style_read(bytes,length+12,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        metrics.logical_extent==11);
    classic_scroll_metrics drag=fixed;
    drag.drag=(style_scroll_drag){STYLE_SCROLL_DRAG_BAR,3,2};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&drag,NULL,NULL,NULL,NULL,NULL,NULL)==length+23);
    CHECK(bytes[6]==30 && classic_style_read(bytes,length+23,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        metrics.drag.scale==STYLE_SCROLL_DRAG_BAR && metrics.drag.first==3 && metrics.drag.second==2);
    bytes[length+20]=3;
    CHECK(classic_style_read(bytes,length+23,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        metrics.drag.scale==STYLE_SCROLL_DRAG_BAR);

    classic_scroll_metrics paged=fixed;
    paged.thumb=STYLE_SCROLL_THUMB_PAGE;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&paged,NULL,NULL,NULL,NULL,NULL,NULL)==length+21);
    CHECK(bytes[6]==11 && classic_style_read(bytes,length+21,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        metrics.source==fixed.source && metrics.logical_extent==11 && metrics.thumb==STYLE_SCROLL_THUMB_PAGE);
    bytes[length+20]=0;
    CHECK(classic_style_read(bytes,length+21,&group,&metrics,NULL,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        metrics.thumb==STYLE_SCROLL_THUMB_PAGE);
    const classic_menu_metrics menu={22,10,15,5,0,0,0,0,0,0};
    classic_menu_metrics columns={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed,&menu,NULL,NULL,NULL,NULL,NULL)==length+28);
    CHECK(classic_style_read(bytes,length+28,&group,&metrics,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        same_menu(&columns,&menu));
    bytes[length+24]=5;
    CHECK(classic_style_read(bytes,length+28,&group,&metrics,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        same_menu(&columns,&menu));
    classic_menu_metrics inactive=menu;
    inactive.mdi_inactive=CLASSIC_MDI_DEFINED|CLASSIC_MDI_RESTORE;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed,&inactive,NULL,NULL,NULL,NULL,NULL)==length+37);
    CHECK(bytes[6]==31 && classic_style_read(bytes,length+37,&group,&metrics,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        same_menu(&columns,&inactive));
    bytes[length+36]=CLASSIC_MDI_RESTORE;
    CHECK(classic_style_read(bytes,length+37,&group,&metrics,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        same_menu(&columns,&inactive));
    classic_menu_metrics configured=menu;
    configured.text_flags=STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_STIPPLE_DISABLED|
        STYLE_MENU_TEXT_BOLD_DEFAULT|STYLE_MENU_TEXT_CENTER_BAR|STYLE_MENU_COMMAND_GLYPHS;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,&configured,NULL,NULL,NULL,NULL,NULL)==length+26);
    CHECK(bytes[6]==9 && classic_style_read(bytes,length+26,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        same_menu(&columns,&configured));
    bytes[length+24]|=0x40;
    CHECK(classic_style_read(bytes,length+26,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        same_menu(&columns,&configured));
    configured.bar_padding=9;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,&configured,NULL,NULL,NULL,NULL,NULL)==length+35);
    CHECK(bytes[6]==19 && classic_style_read(bytes,length+35,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        same_menu(&columns,&configured));
    bytes[length+34]=65;
    CHECK(classic_style_read(bytes,length+35,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        same_menu(&columns,&configured));
    configured.mdi_buttons=CLASSIC_MDI_DEFINED|CLASSIC_MDI_CLOSE;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,&configured,NULL,NULL,NULL,NULL,NULL)==length+36);
    CHECK(classic_style_read(bytes,length+36,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        same_menu(&columns,&configured));
    bytes[length+35]=CLASSIC_MDI_CLOSE;
    CHECK(classic_style_read(bytes,length+36,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        same_menu(&columns,&configured));
    configured.row_minimum=14;configured.row_padding=1;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,&configured,NULL,NULL,NULL,NULL,NULL)==length+46);
    CHECK(bytes[6]==26 && classic_style_read(bytes,length+46,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        columns.row_minimum==14 && columns.row_padding==1);
    bytes[length+45]=17;
    CHECK(classic_style_read(bytes,length+46,&group,NULL,&columns,NULL,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        columns.row_minimum==14 && columns.row_padding==1);
    configured.row_minimum=0;
    CHECK(!classic_style_write(bytes,sizeof(bytes),&original,NULL,&configured,NULL,NULL,NULL,NULL,NULL));
    configured.row_padding=0;
    configured.mdi_buttons=0;
    configured.bar_padding=65;
    CHECK(!classic_style_write(bytes,sizeof(bytes),&original,NULL,&configured,NULL,NULL,NULL,NULL,NULL));
    configured.bar_padding=0;
    configured.text_flags=STYLE_MENU_TEXT_SHORTCUT_RIGHT;
    CHECK(!classic_style_write(bytes,sizeof(bytes),&original,NULL,&configured,NULL,NULL,NULL,NULL,NULL));
    const classic_dialog_metrics dialog={{430,275,575,350,375,199,8,21},510,460,500,300,185,240,80,65,290,198};
    classic_dialog_metrics dimensions={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,&fixed,&menu,&dialog,NULL,NULL,NULL,NULL)==length+72);
    CHECK(classic_style_read(bytes,length+72,&group,&metrics,&columns,&dimensions,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&dimensions,&dialog,sizeof(dialog)));
    bytes[length+40]=0;bytes[length+41]=0; /* Places width below compact width. */
    CHECK(classic_style_read(bytes,length+72,&group,&metrics,&columns,&dimensions,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        !memcmp(&dimensions,&dialog,sizeof(dialog)));
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,&dimensions,NULL,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        !dimensions.file.compact_width && !dimensions.print_width);
    classic_dialog_policy mixed={CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,CLASSIC_FILE_EXPLORER_LAYOUT,
        CLASSIC_FOLDER_LEGACY_TREE,CLASSIC_PRINT_NATIVE,0,0,0},policy={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,&mixed,NULL,NULL,NULL)==length+12);
    CHECK(classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,&policy,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&mixed,&policy,sizeof(policy)) && group.slots[1].role==CLASSIC_BUTTON_CLOSE);
    const unsigned char legacy_wire[]={1,2,1,0};
    CHECK(!memcmp(bytes+length+8,legacy_wire,sizeof(legacy_wire)));
    for(size_t i=0;i<sizeof(legacy_wire);++i){
        bytes[length+8+i]=3;
        CHECK(classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,&policy,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
            !memcmp(&mixed,&policy,sizeof(policy)));
        bytes[length+8+i]=legacy_wire[i];
    }
    memcpy(bytes+length+12,bytes+length,12);
    bytes[8]=(unsigned char)(length+24);bytes[12]=3;
    CHECK(classic_style_read(bytes,length+24,&group,NULL,NULL,NULL,&policy,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID);
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,NULL,&policy,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        !policy.window_provider);
    mixed.color=CLASSIC_STOCK_NATIVE;mixed.font=CLASSIC_STOCK_OWNER_CHROME;mixed.find=CLASSIC_STOCK_OBSERVED_LAYOUT;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,&mixed,NULL,NULL,NULL)==length+23 &&
        bytes[6]==18 && bytes[length+12]==21);
    CHECK(classic_style_read(bytes,length+23,&group,NULL,NULL,NULL,&policy,NULL,NULL,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&mixed,&policy,sizeof(policy)));
    bytes[length+22]=4;
    CHECK(classic_style_read(bytes,length+23,&group,NULL,NULL,NULL,&policy,NULL,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        !memcmp(&mixed,&policy,sizeof(policy)));
    style_caption_presentation caption={.text_mode=STYLE_CAPTION_TEXT_CENTER_CLIP,
        .separator=STYLE_CAPTION_SEPARATOR_INSIDE},decoded={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,&caption,NULL,NULL)==length+10);
    CHECK(classic_style_read(bytes,length+10,&group,NULL,NULL,NULL,NULL,&decoded,NULL,NULL)==CLASSIC_STYLE_OK &&
        decoded.text_mode==caption.text_mode && decoded.separator==caption.separator);
    bytes[length+9]=3;
    CHECK(classic_style_read(bytes,length+10,&group,NULL,NULL,NULL,NULL,&decoded,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        decoded.separator==caption.separator);
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,NULL,NULL,&decoded,NULL,NULL)==CLASSIC_STYLE_OK &&
        !decoded.text_mode);
    caption.button_source=STYLE_CAPTION_BUTTON_INDEXED;
    caption.button_outline=STYLE_CAPTION_BUTTON_EDGE_OUTLINE;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,&caption,NULL,NULL)==length+12);
    CHECK(classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,NULL,&decoded,NULL,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&decoded,&caption,sizeof(caption)));
    bytes[length+10]=STYLE_CAPTION_BUTTON_SYSTEM;
    CHECK(classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,NULL,&decoded,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        !memcmp(&decoded,&caption,sizeof(caption)));
    const style_border_presentation border={STYLE_BORDER_CLIPPED,STYLE_BORDER_TITLE,
        {STYLE_BORDER_OUTER,STYLE_BORDER_EDGE,STYLE_BORDER_EDGE},
        {STYLE_BORDER_NEXT,STYLE_BORDER_LIGHT,STYLE_BORDER_SHADOW},STYLE_BORDER_KEEP,STYLE_BORDER_EDGE,{0}};
    style_border_presentation restored={0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,&border,NULL)==length+18);
    CHECK(classic_style_read(bytes,length+18,&group,NULL,NULL,NULL,NULL,NULL,&restored,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&restored,&border,sizeof(border)));
    bytes[length+10]=255;
    CHECK(classic_style_read(bytes,length+18,&group,NULL,NULL,NULL,NULL,NULL,&restored,NULL)==CLASSIC_STYLE_INVALID &&
        !memcmp(&restored,&border,sizeof(border)));
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,NULL,NULL,NULL,&restored,NULL)==CLASSIC_STYLE_OK &&
        !restored.fill);
    style_border_presentation geometry=border;
    geometry.geometry=(style_frame_geometry){STYLE_FRAME_MONITOR_PIXELS,7,5,6,1,23,1,0};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,&geometry,NULL)==length+25);
    CHECK(classic_style_read(bytes,length+25,&group,NULL,NULL,NULL,NULL,NULL,&restored,NULL)==CLASSIC_STYLE_OK &&
        !memcmp(&restored,&geometry,sizeof(geometry)));
    bytes[length+18]=255;
    CHECK(classic_style_read(bytes,length+25,&group,NULL,NULL,NULL,NULL,NULL,&restored,NULL)==CLASSIC_STYLE_INVALID &&
        !memcmp(&restored,&geometry,sizeof(geometry)));
    geometry.geometry.scroll_shared_units=1;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,&geometry,NULL)==length+34);
    CHECK(bytes[6]==32 && classic_style_read(bytes,length+34,&group,NULL,NULL,NULL,NULL,NULL,&restored,NULL)==CLASSIC_STYLE_OK &&
        restored.geometry.scroll_shared_units==1);
    bytes[length+33]=2;
    CHECK(classic_style_read(bytes,length+34,&group,NULL,NULL,NULL,NULL,NULL,&restored,NULL)==CLASSIC_STYLE_INVALID &&
        restored.geometry.scroll_shared_units==1);
    classic_control_presentation controls={.separator=STYLE_LINE_RELIEF};
    CHECK(classic_style_read(reference,length,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.separator==STYLE_LINE_DEFAULT);
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&(classic_control_presentation){.separator=STYLE_LINE_FLAT})==length+9);
    CHECK(bytes[6]==10 && classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.separator==STYLE_LINE_FLAT);
    bytes[length+8]=0;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.separator==STYLE_LINE_FLAT);
    const classic_control_presentation mixed_controls={.separator=STYLE_LINE_RELIEF,.push=STYLE_PUSH_STEPPED};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&mixed_controls)==length+18);
    CHECK(bytes[6]==12 && classic_style_read(bytes,length+18,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.separator==STYLE_LINE_RELIEF && controls.push==STYLE_PUSH_STEPPED);
    bytes[length+17]=255;
    CHECK(classic_style_read(bytes,length+18,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.push==STYLE_PUSH_STEPPED);
    const classic_control_presentation lines={.group=STYLE_LINE_RELIEF,.edit=STYLE_LINE_FLAT};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&lines)==length+10);
    CHECK(bytes[6]==13 && classic_style_read(bytes,length+10,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.group==STYLE_LINE_RELIEF && controls.edit==STYLE_LINE_FLAT && !controls.push && !controls.separator);
    bytes[length+9]=255;
    CHECK(classic_style_read(bytes,length+10,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.edit==STYLE_LINE_FLAT);
    const classic_control_presentation progress={.progress=STYLE_PROGRESS_OUTLINED};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&progress)==length+14);
    CHECK(bytes[6]==14 && classic_style_read(bytes,length+14,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        !memcmp(&controls.progress,&progress.progress,sizeof(progress.progress)));
    bytes[length+13]=0;
    CHECK(classic_style_read(bytes,length+14,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.progress.block_whole==4);
    const classic_control_presentation trackbar={.trackbar=STYLE_TRACKBAR_RELIEF};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&trackbar)==length+9);
    CHECK(bytes[6]==15 && classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.trackbar==STYLE_TRACKBAR_RELIEF && !controls.progress.flags);
    bytes[length+8]=0;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.trackbar==STYLE_TRACKBAR_RELIEF);
    const classic_control_presentation header={.header=STYLE_LINE_FLAT};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&header)==length+9);
    CHECK(bytes[6]==16 && classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.header==STYLE_LINE_FLAT && !controls.trackbar);
    bytes[length+8]=255;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.header==STYLE_LINE_FLAT);
    const classic_control_presentation tab={.tab=STYLE_LINE_RELIEF};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&tab)==length+9);
    CHECK(bytes[6]==17 && classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.tab==STYLE_LINE_RELIEF && !controls.header);
    bytes[length+8]=0;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.tab==STYLE_LINE_RELIEF);
    const classic_control_presentation native_edge={.native_edge=STYLE_LINE_FLAT};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&native_edge)==length+9);
    CHECK(bytes[6]==20 && classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.native_edge==STYLE_LINE_FLAT && !controls.edit && !controls.tab);
    bytes[length+8]=3;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.native_edge==STYLE_LINE_FLAT);
    const classic_control_presentation choice={.choice_fallback=CLASSIC_CHOICE_SYMBOLS};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&choice)==length+9);
    CHECK(bytes[6]==22 && classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.choice_fallback==CLASSIC_CHOICE_SYMBOLS && !controls.native_edge);
    bytes[length+8]=255;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.choice_fallback==CLASSIC_CHOICE_SYMBOLS);
    const classic_control_presentation choice_geometry={.choice_metrics={14,13,4,STYLE_CHOICE_DPI}};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&choice_geometry)==length+12);
    CHECK(bytes[6]==23 && classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.choice_metrics.width==14 && controls.choice_metrics.height==13 &&
        controls.choice_metrics.gap==4 && controls.choice_metrics.flags==STYLE_CHOICE_DPI);
    bytes[length+11]|=STYLE_CHOICE_FONT_GAP;
    CHECK(classic_style_read(bytes,length+12,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.choice_metrics.flags==STYLE_CHOICE_DPI);
    const classic_control_presentation spin={.updown={1,1,3}};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&spin)==length+11);
    CHECK(bytes[6]==24 && classic_style_read(bytes,length+11,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.updown.outline==1 && controls.updown.padding==1 && controls.updown.crop==3);
    bytes[length+10]=9;
    CHECK(classic_style_read(bytes,length+11,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.updown.crop==3);
    const classic_control_presentation recolor={.updown_mode=CLASSIC_UPDOWN_RECOLOR};
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,NULL,NULL,&recolor)==length+9);
    CHECK(bytes[6]==25 && classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_OK &&
        controls.updown_mode==CLASSIC_UPDOWN_RECOLOR && !controls.updown.outline);
    bytes[length+8]=3;
    CHECK(classic_style_read(bytes,length+9,&group,NULL,NULL,NULL,NULL,NULL,NULL,&controls)==CLASSIC_STYLE_INVALID &&
        controls.updown_mode==CLASSIC_UPDOWN_RECOLOR);
    caption.icon_mode=STYLE_CAPTION_ICON_NATIVE;
    CHECK(classic_style_write(bytes,sizeof(bytes),&original,NULL,NULL,NULL,NULL,&caption,NULL,NULL)==length+21);
    CHECK(bytes[6]==21 && classic_style_read(bytes,length+21,&group,NULL,NULL,NULL,NULL,&decoded,NULL,NULL)==CLASSIC_STYLE_OK &&
        decoded.icon_mode==STYLE_CAPTION_ICON_NATIVE);
    bytes[length+20]=3;
    CHECK(classic_style_read(bytes,length+21,&group,NULL,NULL,NULL,NULL,&decoded,NULL,NULL)==CLASSIC_STYLE_INVALID &&
        decoded.icon_mode==STYLE_CAPTION_ICON_NATIVE);
    return 0;
}

static int packaged(PCWSTR root,PCWSTR name,unsigned expected_count,unsigned expected_last,unsigned expected_gap,
    unsigned measure)
{
    WCHAR path[MAX_PATH];
    CHECK(swprintf(path,ARRAYSIZE(path),L"%ls/%ls/%ls.msstyles",root,name,name)>0);
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(module);
    HRSRC resource=FindResourceW(module,L"CLASSIC_STYLE",MAKEINTRESOURCEW(10));
    DWORD size=resource?SizeofResource(module,resource):0;
    HGLOBAL handle=resource?LoadResource(module,resource):NULL;
    classic_button_group group;
    classic_scroll_metrics metrics;
    classic_menu_metrics columns;
    classic_dialog_metrics dialog;
    classic_dialog_policy policy;
    style_caption_presentation caption;
    style_border_presentation border;
    classic_control_presentation controls;
    enum classic_style_result result=classic_style_read(handle?LockResource(handle):NULL,size,&group,&metrics,&columns,&dialog,&policy,&caption,&border,&controls);
    const WORD language=MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);
    for(WORD id=201;id<=203;++id)
        CHECK(FindResourceExW(module,MAKEINTRESOURCEW(5),MAKEINTRESOURCEW(id),language));
    FreeLibrary(module);
    CHECK(result==CLASSIC_STYLE_OK && group.count==expected_count);
    CHECK(group.layout==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?CLASSIC_BUTTON_LAYOUT_EXPLICIT:
        (CLASSIC_BUTTON_LAYOUT_EXPLICIT|CLASSIC_BUTTON_LAYOUT_CLIP|CLASSIC_BUTTON_LAYOUT_FILL)));
    CHECK(group.slots[group.count-1].role==expected_last && group.slots[group.count-1].gap_before==expected_gap);
    CHECK(metrics.source==measure && !metrics.logical_extent);
    CHECK(metrics.drag.scale==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?STYLE_SCROLL_DRAG_BAR:STYLE_SCROLL_DRAG_LOGICAL) &&
        metrics.drag.first==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?8:4) &&
        metrics.drag.second==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?2:1));
    CHECK(metrics.initial_repeat_ms==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?0:200) &&
        metrics.repeat_ms==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?0:50));
    CHECK(metrics.thumb==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?STYLE_SCROLL_THUMB_PAGE:STYLE_SCROLL_THUMB_FIXED));
    CHECK(controls.separator==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?STYLE_LINE_RELIEF:STYLE_LINE_FLAT));
    CHECK(controls.push==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?STYLE_PUSH_RELIEF:STYLE_PUSH_STEPPED));
    CHECK(controls.group==controls.separator && controls.edit==controls.separator);
    const style_progress_layout outlined=STYLE_PROGRESS_OUTLINED,inset=STYLE_PROGRESS_INSET;
    CHECK(!memcmp(&controls.progress,measure==CLASSIC_SCROLL_EXTENT_SYSTEM?&inset:&outlined,sizeof(outlined)));
    CHECK(controls.trackbar==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?STYLE_TRACKBAR_RELIEF:STYLE_TRACKBAR_OUTLINED));
    CHECK(controls.header==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?STYLE_LINE_RELIEF:STYLE_LINE_FLAT));
    CHECK(controls.tab==controls.header);
    CHECK(controls.native_edge==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?STYLE_LINE_RELIEF:STYLE_LINE_DEFAULT));
    CHECK(controls.choice_fallback==(measure==CLASSIC_SCROLL_EXTENT_SYSTEM?CLASSIC_CHOICE_SYMBOLS:CLASSIC_CHOICE_NATIVE));
    const style_choice_metrics choice=controls.choice_metrics;
    CHECK(choice.height==13);
    if(measure==CLASSIC_SCROLL_EXTENT_SYSTEM)
        CHECK(choice.width==13 && !choice.gap && choice.flags==(STYLE_CHOICE_FONT_GAP|STYLE_CHOICE_ALIGN));
    else CHECK(choice.width==14 && choice.gap==4 && choice.flags==STYLE_CHOICE_DPI);
    if(measure==CLASSIC_SCROLL_EXTENT_SYSTEM)
        CHECK(controls.updown_mode==CLASSIC_UPDOWN_RECOLOR && !controls.updown.outline &&
            !controls.updown.padding && !controls.updown.crop);
    else CHECK(controls.updown_mode==CLASSIC_UPDOWN_ART && controls.updown.outline==1 &&
        controls.updown.padding==1 && controls.updown.crop==3);
    const classic_dialog_policy expected=measure==CLASSIC_SCROLL_EXTENT_SYSTEM?
        (classic_dialog_policy){CLASSIC_WINDOW_PROVIDER_NONCLIENT,CLASSIC_FILE_EXPLORER_LAYOUT,
            CLASSIC_FOLDER_OBSERVED_NATIVE,CLASSIC_PRINT_GENERAL_PAGE,CLASSIC_STOCK_OBSERVED_LAYOUT,
            CLASSIC_STOCK_OBSERVED_LAYOUT,CLASSIC_STOCK_OBSERVED_LAYOUT}:
        (classic_dialog_policy){CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,CLASSIC_FILE_LEGACY_LAYOUT,
            CLASSIC_FOLDER_LEGACY_TREE,CLASSIC_PRINT_LEGACY_BRIDGE,CLASSIC_STOCK_OWNER_CHROME,
            CLASSIC_STOCK_OWNER_CHROME,CLASSIC_STOCK_OWNER_CHROME};
    CHECK(!memcmp(&policy,&expected,sizeof(policy)));
    if(measure==CLASSIC_SCROLL_EXTENT_CROSS_AXIS){
        CHECK(caption.text_mode==STYLE_CAPTION_TEXT_CENTER_CLIP && caption.separator==STYLE_CAPTION_SEPARATOR_AFTER &&
            caption.icon_mode==STYLE_CAPTION_ICON_HIDDEN);
        CHECK(caption.button_source==STYLE_CAPTION_BUTTON_INDEXED && caption.button_outline==STYLE_CAPTION_BUTTON_EDGE_OUTLINE);
        CHECK(border.fill==STYLE_BORDER_CLIPPED && border.dialog_fill==STYLE_BORDER_TITLE &&
            border.outer.leading==STYLE_BORDER_EDGE && border.outer.trailing==STYLE_BORDER_EDGE &&
            border.inner.anchor==STYLE_BORDER_INNER && border.dialog_inner==STYLE_BORDER_LIGHT &&
            border.divider==STYLE_BORDER_EDGE);
        CHECK(border.geometry.scale==STYLE_FRAME_LOGICAL && border.geometry.sizeable==4 &&
            border.geometry.fixed==3 && border.geometry.dialog==6 && border.geometry.line==1 &&
            border.geometry.caption_floor==20 && border.geometry.separator_units==1 && border.geometry.scroll_shared_units==1);
        CHECK(columns.check_column==18 && columns.shortcut_gap==8 && columns.arrow_column==12 && columns.plain_column==4 &&
            columns.bar_padding==6 &&
            columns.mdi_buttons==(CLASSIC_MDI_DEFINED|CLASSIC_MDI_SYSTEM|CLASSIC_MDI_RESTORE) &&
            columns.mdi_inactive==(CLASSIC_MDI_DEFINED|CLASSIC_MDI_MINIMIZE|CLASSIC_MDI_CLOSE) && columns.text_flags==(STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_STIPPLE_DISABLED));
    }else{
        CHECK(caption.text_mode==STYLE_CAPTION_TEXT_LEFT_ELLIPSIS && caption.separator==STYLE_CAPTION_SEPARATOR_INSIDE &&
            caption.icon_mode==STYLE_CAPTION_ICON_NATIVE);
        CHECK(caption.button_source==STYLE_CAPTION_BUTTON_SYSTEM && caption.button_outline==STYLE_CAPTION_BUTTON_NO_OUTLINE);
        CHECK(border.fill==STYLE_BORDER_PERIMETER && border.inner.anchor==STYLE_BORDER_NEXT &&
            border.inner.leading==STYLE_BORDER_LIGHT && border.inner.trailing==STYLE_BORDER_SHADOW &&
            border.dialog_inner==STYLE_BORDER_KEEP && border.divider==STYLE_BORDER_KEEP);
        CHECK(border.geometry.scale==STYLE_FRAME_MONITOR_PIXELS && border.geometry.sizeable==4 &&
            border.geometry.fixed==3 && !border.geometry.caption_floor && !border.geometry.separator_units && !border.geometry.scroll_shared_units);
        CHECK(columns.check_column==22 && !columns.shortcut_gap && columns.arrow_column==16 && columns.plain_column==16 &&
            !columns.bar_padding && columns.text_flags==(STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_SHORTCUT_RIGHT|
                STYLE_MENU_TEXT_BOLD_DEFAULT|STYLE_MENU_TEXT_CENTER_BAR|STYLE_MENU_COMMAND_GLYPHS));
    }
    CHECK(dialog.file.compact_width==427 && dialog.file.compact_height==272 &&
        dialog.file.places_width==565 && dialog.file.places_height==345 &&
        dialog.file.client_dlu_width==370 && dialog.file.client_dlu_height==197 &&
        dialog.file.gap==7 && dialog.file.bottom_reserve==20);
    CHECK(dialog.print_width==500 && dialog.print_full_height==450 && dialog.print_large_height==490);
    CHECK(dialog.color_width==298 && dialog.color_height==184 && dialog.find_width==236 &&
        dialog.find_native_height==74 && dialog.find_classic_height==62 && dialog.font_width==287 && dialog.font_height==196);
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==1)return fixtures();
    if(argc!=2)return 2;
    return packaged(argv[1],L"Win16",3,CLASSIC_BUTTON_MAXIMIZE,0,CLASSIC_SCROLL_EXTENT_CROSS_AXIS) ||
        packaged(argv[1],L"Win2K",4,CLASSIC_BUTTON_CLOSE,2,CLASSIC_SCROLL_EXTENT_SYSTEM);
}
