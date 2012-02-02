#include <stdlib.h>
#include <png.h>

#include "pngout.h"


static png_structp png_ptr;
static png_infop info_ptr;
static FILE *f;

void imageopen(char * name, int width, int height)
{

    if( !(f = fopen(name, "wb")) )
    {
        fprintf(stderr, "Unable to open file \"%s\"\n", name);
        exit(EXIT_FAILURE);
    }
    
    
    
    png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, 
               NULL,
                NULL, NULL);
                
    info_ptr = png_create_info_struct( png_ptr );
                
    png_init_io(png_ptr, f);
    
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png_ptr, info_ptr);
}



void imagewriteline(unsigned char * line)
{
    png_write_row(png_ptr, (png_bytep) line);
}


void imageclose(void)
{
    png_write_end(png_ptr, info_ptr);

}