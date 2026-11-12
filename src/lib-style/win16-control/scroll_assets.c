#include "scroll_assets.h"
#include "scroll_glyphs.h"

static ntw16s_updown_glyph *const glyphs[]={
    &scroll_uparrow,&scroll_dnarrow,&scroll_lfarrow,&scroll_rgarrow,
    &scroll_uparrowd,&scroll_dnarrowd,&scroll_lfarrowd,&scroll_rgarrowd,
    &scroll_uparrowi,&scroll_dnarrowi,&scroll_lfarrowi,&scroll_rgarrowi};

ntw16s_updown_glyph *const *win16_control_scroll_glyphs(void)
{return glyphs;}

const char (*win16_control_scroll_glyph(enum win16_control_scroll_asset asset))[18]
{return asset>=WIN16_SCROLL_UP && asset<=WIN16_SCROLL_RIGHT_DISABLED ? *glyphs[asset] : NULL;}
