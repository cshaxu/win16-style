#include "theme_file.h"
#include <string.h>

static bool token(unsigned char c)
{
    return (c>='a' && c<='z') || (c>='A' && c<='Z') ||
        (c>='0' && c<='9') || c=='-' || c=='_';
}

static bool label(const char *text)
{
    if(!text || !*text || *text==' ' || text[strlen(text)-1]==' ')return false;
    for(const unsigned char *p=(const unsigned char *)text;*p;++p)
        if(!token(*p) && *p!=' ' && *p!='.')return false;
    return true;
}

static bool variant_token(msstyles_v4_name name)
{
    size_t units=name.units;
    if(!name.bytes || units<2 || units>SIZE_MAX/2)return false;
    for(size_t pos=0;pos<units-1;++pos)
        if(!token(name.bytes[2*pos]) || name.bytes[2*pos+1])return false;
    size_t end=2*(units-1);
    return !name.bytes[end] && !name.bytes[end+1];
}

static bool write_token(FILE *output,msstyles_v4_name name)
{
    for(size_t pos=0;pos<name.units-1;++pos)
        if(fputc(name.bytes[2*pos],output)==EOF)return false;
    return true;
}

bool msstyles_theme_file_write(FILE *output,const char *display_name,const char *stem,
    const msstyles_v4_variant *variant)
{
    if(!output || !label(display_name) || !stem || !*stem || !variant ||
       !variant_token(variant->size) || !variant_token(variant->color))return false;
    for(const unsigned char *p=(const unsigned char *)stem;*p;++p)
        if(!token(*p))return false;

    return fprintf(output,"[Theme]\r\nDisplayName=%s\r\n\r\n"
               "[Control Panel\\Desktop]\r\nTileWallpaper=0\r\n\r\n"
               "[VisualStyles]\r\nPath=%s.msstyles\r\nColorStyle=",display_name,stem)>=0 &&
        write_token(output,variant->color) &&
        fputs("\r\nSize=",output)!=EOF && write_token(output,variant->size) &&
        fputs("\r\n\r\n[MasterThemeSelector]\r\nMTSM=DABJDKT\r\n",output)!=EOF;
}
