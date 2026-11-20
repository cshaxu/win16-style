#include "theme_file.h"
#include <string.h>

static bool token(unsigned char c)
{
    return (c>='a' && c<='z') || (c>='A' && c<='Z') ||
        (c>='0' && c<='9') || c=='-' || c=='_';
}

static bool label(const char *text)
{
    if(!text || !*text)return false;
    size_t length=strlen(text);
    if(length>=MSSTYLES_THEME_TOKEN_CAPACITY || *text==' ' || text[length-1]==' ')return false;
    for(const unsigned char *p=(const unsigned char *)text;*p;++p)
        if(!token(*p) && *p!=' ' && *p!='.')return false;
    return true;
}

static bool variant_token(msstyles_v4_name name)
{
    size_t units=name.units;
    if(!name.bytes || units<2 || units>MSSTYLES_THEME_TOKEN_CAPACITY)return false;
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
    const msstyles_v4_variant *variant,unsigned host_options)
{
    if(!output || !label(display_name) || !stem || !*stem || strlen(stem)>=MSSTYLES_THEME_TOKEN_CAPACITY || !variant ||
       !variant_token(variant->size) || !variant_token(variant->color) || (host_options&~MSSTYLES_HOST_ALL))return false;
    for(const unsigned char *p=(const unsigned char *)stem;*p;++p)
        if(!token(*p))return false;

    return fprintf(output,"[Theme]\r\nDisplayName=%s\r\n\r\n"
               "[Control Panel\\Desktop]\r\nTileWallpaper=0\r\n\r\n"
               "[VisualStyles]\r\nPath=%s.msstyles\r\nColorStyle=",display_name,stem)>=0 &&
        write_token(output,variant->color) &&
        fputs("\r\nSize=",output)!=EOF && write_token(output,variant->size) &&
        fprintf(output,"\r\n\r\n[WindowAttachment]\r\nVersion=1\r\n"
            "Explorer=%u\r\nTerminal=%u\r\nConsole=%u\r\n\r\n"
            "[MasterThemeSelector]\r\nMTSM=DABJDKT\r\n",
            (host_options&MSSTYLES_HOST_EXPLORER)?1u:0u,
            (host_options&MSSTYLES_HOST_TERMINAL)?1u:0u,
            (host_options&MSSTYLES_HOST_CONSOLE)?1u:0u)>=0;
}

static bool copy_token(char *target,const char *value,const char *extension)
{
    size_t length=strlen(value),suffix=strlen(extension);
    if(*target || length<=suffix || length-suffix>=MSSTYLES_THEME_TOKEN_CAPACITY ||
       strcmp(value+length-suffix,extension))return false;
    for(size_t i=0;i<length-suffix;++i)
        if(!token((unsigned char)value[i]))return false;
    memcpy(target,value,length+1);
    return true;
}

static void read_display_name(char *target,const char *value)
{
    size_t length=strlen(value);
    if(!length || length>=MSSTYLES_THEME_TOKEN_CAPACITY || *value=='@')return;
    for(const unsigned char *p=(const unsigned char *)value;*p;++p)
        if(*p<32 || *p==127)return;
    memcpy(target,value,length+1);
}

enum { host_version_key=8u, host_required_keys=MSSTYLES_HOST_ALL|host_version_key };

static bool read_host_option(char *line,unsigned *seen,msstyles_host_options *hosts)
{
    if(!*line || *line==';' || *line=='#')return true;
    char *value=strchr(line,'=');
    if(!value)return false;
    *value++=0;
    unsigned key;
    if(!strcmp(line,"Version"))key=host_version_key;
    else if(!strcmp(line,"Explorer"))key=MSSTYLES_HOST_EXPLORER;
    else if(!strcmp(line,"Terminal"))key=MSSTYLES_HOST_TERMINAL;
    else if(!strcmp(line,"Console"))key=MSSTYLES_HOST_CONSOLE;
    else return false;
    if((*seen&key) || (strcmp(value,"1") && (key==host_version_key || strcmp(value,"0"))))
        return false;
    *seen|=key;
    if(key!=host_version_key && *value=='1')hosts->enabled|=key;
    return true;
}

bool msstyles_theme_file_read(FILE *input,msstyles_theme_descriptor *descriptor)
{
    if(!input || !descriptor)return false;
    char text[4096];
    size_t length=fread(text,1,sizeof(text),input);
    if(ferror(input) || !length || length==sizeof(text) || memchr(text,0,length))return false;
    text[length]=0;
    msstyles_theme_descriptor parsed={0};
    bool visual=false,theme=false,attachment=false,display_seen=false;
    unsigned host_keys=0;
    for(char *line=text;line;){
        char *next=strchr(line,'\n');
        if(next)*next++=0;
        char *end=line+strlen(line);
        if(end!=line && end[-1]=='\r')end[-1]=0;
        if(*line=='['){
            visual=!strcmp(line,"[VisualStyles]");
            theme=!strcmp(line,"[Theme]");
            attachment=!strcmp(line,"[WindowAttachment]");
            if(attachment){
                if(parsed.hosts.declared)return false;
                parsed.hosts.declared=true;
            }
        }else if(attachment){
            if(!read_host_option(line,&host_keys,&parsed.hosts))return false;
        }else if(theme && !strncmp(line,"DisplayName=",12)){
            if(display_seen)parsed.display_name[0]=0;
            else read_display_name(parsed.display_name,line+12);
            display_seen=true;
        }else if(visual){
            char *value=strchr(line,'=');
            if(value){
                *value++=0;
                if(!strcmp(line,"Path") && !copy_token(parsed.file,value,".msstyles"))return false;
                if(!strcmp(line,"Size") && !copy_token(parsed.size,value,""))return false;
                if(!strcmp(line,"ColorStyle") && !copy_token(parsed.color,value,""))return false;
            }
        }
        line=next;
    }
    if(!parsed.file[0] || !parsed.size[0] || !parsed.color[0] ||
       (parsed.hosts.declared && host_keys!=host_required_keys))return false;
    *descriptor=parsed;
    return true;
}
