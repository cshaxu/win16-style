#include "lib-win32-style/appearance/msstyles/theme_file.h"
#include "lib-win32-style/appearance/msstyles/v4_endian.h"
#include "lib-win32-style/appearance/msstyles/v4_property.h"
#include "classic_source.h"
#include <vsstyle.h>
#include <vssym32.h>
#include <string.h>
#include <wchar.h>

#define THEME_NAME(s) {(const unsigned char *)(s),ARRAYSIZE(s)}

enum { button_class=6, scrollbar_class, window_class };
static const msstyles_v4_name classes[]={
    THEME_NAME(L"documentation"),THEME_NAME(L"sizevariant.NormalSize"),
    THEME_NAME(L"sizevariant.Default"),THEME_NAME(L"colorvariant.NormalColor"),
    THEME_NAME(L"globals"),THEME_NAME(L"sysmetrics"),
    THEME_NAME(L"Button"),THEME_NAME(L"ScrollBar"),THEME_NAME(L"Window")
};
static const msstyles_v4_variant variant={
    THEME_NAME(L"Normal"),THEME_NAME(L"NormalSize"),THEME_NAME(L"NormalColor")
};

static bool bytes(FILE *output,const void *data,size_t size)
{
    return fwrite(data,1,size,output)==size;
}

static bool property(FILE *output,uint32_t name,uint32_t type,
    uint32_t cls,uint32_t part,uint32_t state,uint32_t value)
{
    unsigned char data[4],record[40];
    msstyles_v4_write_word(data,value);
    msstyles_v4_property item={name,type,cls,part,state,0,sizeof(data),data};
    size_t length;
    return msstyles_v4_property_write(record,sizeof(record),ARRAYSIZE(classes),&item,&length) &&
        bytes(output,record,length);
}

static bool caption(FILE *output,uint32_t state,classic_caption_source colors,bool gradient)
{
    return property(output,TMT_FILLCOLOR,TMT_COLOR,window_class,WP_CAPTION,state,colors.start) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,window_class,WP_CAPTION,state,colors.text) &&
        (!gradient ||
         (property(output,TMT_GRADIENTCOLOR1,TMT_COLOR,window_class,WP_CAPTION,state,colors.start) &&
          property(output,TMT_GRADIENTCOLOR2,TMT_COLOR,window_class,WP_CAPTION,state,colors.end)));
}

static bool button(FILE *output,uint32_t cls,uint32_t part,const classic_theme_source *source)
{
    return property(output,TMT_FILLCOLOR,TMT_COLOR,cls,part,0,source->face) &&
        property(output,TMT_BORDERCOLOR,TMT_COLOR,cls,part,0,source->edge) &&
        property(output,TMT_BGTYPE,TMT_ENUM,cls,part,0,BT_BORDERFILL) &&
        property(output,TMT_BORDERTYPE,TMT_ENUM,cls,part,0,BT_RECT) &&
        property(output,TMT_FILLTYPE,TMT_ENUM,cls,part,0,FT_SOLID);
}

static bool write_properties(FILE *output,const classic_theme_source *source)
{
    return button(output,button_class,BP_PUSHBUTTON,source) &&
        button(output,scrollbar_class,SBP_ARROWBTN,source) &&
        property(output,TMT_BGTYPE,TMT_ENUM,window_class,WP_CAPTION,0,BT_BORDERFILL) &&
        property(output,TMT_FILLTYPE,TMT_ENUM,window_class,WP_CAPTION,0,
                 source->gradient?FT_HORZGRADIENT:FT_SOLID) &&
        caption(output,CS_ACTIVE,source->active,source->gradient) &&
        caption(output,CS_INACTIVE,source->inactive,source->gradient);
}

static bool write_classes(FILE *output,const classic_theme_source *source)
{
    (void)source;
    for(size_t i=0;i<ARRAYSIZE(classes);++i){
        unsigned char buffer[64];
        size_t length;
        if(!msstyles_v4_class_write(buffer,sizeof(buffer),classes[i],&length) ||
           !bytes(output,buffer,length))return false;
    }
    return true;
}

static bool write_variant(FILE *output,const classic_theme_source *source)
{
    (void)source;
    unsigned char buffer[128];
    size_t length;
    return msstyles_v4_variant_write(buffer,sizeof(buffer),&variant,&length) &&
        bytes(output,buffer,length);
}

static bool write_base_classes(FILE *output,const classic_theme_source *source)
{
    (void)source;
    unsigned char table[4*(ARRAYSIZE(classes)-3)];
    memset(table,0xff,sizeof(table));
    msstyles_v4_write_word(table,ARRAYSIZE(classes)-4);
    return bytes(output,table,sizeof(table));
}

static bool write_metadata(FILE *output,const classic_theme_source *source)
{
    unsigned char name[64]={0},record[96];
    size_t length=strlen(source->label);
    if(length>=sizeof(name)/2)return false;
    for(size_t i=0;i<length;++i)name[2*i]=(unsigned char)source->label[i];
    msstyles_v4_property item={TMT_DISPLAYNAME,TMT_STRING,0,0,0,0,(uint32_t)((length+1)*2),name};
    size_t written;
    return msstyles_v4_property_write(record,sizeof(record),ARRAYSIZE(classes),&item,&written) &&
        bytes(output,record,written);
}

static bool write_description(FILE *output,const classic_theme_source *source)
{
    return msstyles_theme_file_write(output,source->label,source->stem,&variant);
}

static const struct {
    const char *filename;
    bool (*write)(FILE *,const classic_theme_source *);
} outputs[]={
    {"cmap.bin",write_classes},{"bcmap.bin",write_base_classes},
    {"vmap.bin",write_variant},{"rmap.bin",write_metadata},
    {"normal.bin",write_properties},{NULL,write_description}
};

int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    const classic_theme_source *source=NULL;
    for(size_t i=0;i<ARRAYSIZE(classic_sources);++i)
        if(!strcmp(argv[1],classic_sources[i].stem))source=&classic_sources[i];
    if(!source)return 2;

    char description[32];
    snprintf(description,sizeof(description),"%s.theme",source->stem);
    for(size_t i=0;i<ARRAYSIZE(outputs);++i){
        const char *name=outputs[i].filename?outputs[i].filename:description;
        FILE *output=fopen(name,"wb");
        if(!output){perror(name);return 1;}
        bool written=outputs[i].write(output,source);
        if(fclose(output)!=0 || !written){
            fprintf(stderr,"Failed to write %s\n",name);
            return 1;
        }
    }
    return 0;
}
