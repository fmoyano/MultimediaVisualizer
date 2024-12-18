#ifndef IMG_LOADER
#define IMG_LOADER

#include <windows.h>
#include <stdio.h>

#pragma pack(1)
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

inline void* img_loader_load(const char* const filename)
{
	printf("Size of TGA_Header for %s: %zd\n", filename, sizeof(TGA_Header));

	HANDLE handle = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
  if (handle == INVALID_HANDLE_VALUE)
  {
    printf("Error creating file\n");
    return 0;
  }

  BY_HANDLE_FILE_INFORMATION file_info = {0};
  if (!GetFileInformationByHandle(handle, &file_info))
  {
    printf("Problem obtaining file info\n");
    return 0;
  }

  TGA_Header tga_header = {0};
	unsigned long bytes_read = 0;
  void *buffer = malloc(file_info.nFileSizeLow);
  //TODO: in while loop just in case it cannot read all in one go
	BOOL res = ReadFile(handle, (void*)&tga_header, sizeof(tga_header), &bytes_read, (OVERLAPPED*)0);
  if (!res)
  {
    printf("Error reading file\n");
    return 0;
  }

  printf("Bytes read: %lu\n", bytes_read);
  printf("width %d\n", tga_header.width);
  printf("height %d\n", tga_header.height);
  printf("bits per pixel %d\n", tga_header.bitsperpixel);

  unsigned long rgb_data_size = tga_header.width * tga_header.height * tga_header.bitsperpixel / 3;
  void *rgb_data = malloc(rgb_data_size);
  res = ReadFile(handle, rgb_data, rgb_data_size, &bytes_read, (OVERLAPPED*)0);
  if (!res)
  {
    printf("Error reading file\n");
    return 0;
  }

  printf("Bytes read: %lu\n", bytes_read);

  //TODO: branch depending on file type

	return buffer;
}


#endif