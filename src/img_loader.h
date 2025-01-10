#ifndef IMG_LOADER
#define IMG_LOADER

#include <windows.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short colourmaporigin;
   short colourmaplength;
   char  colourmapdepth;
   short x_origin;
   short y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
} TGA_Header;
#pragma pack(pop)

void* img_loader_load(const char* const filename, int* image_width, int* image_height, int* image_depth);

#endif