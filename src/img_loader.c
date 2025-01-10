#include "img_loader.h"
#include <string.h>
#include <winnt.h>

static void* img_load_tga(HANDLE handle, int* image_width, int* image_height, int* image_depth)
{
	TGA_Header tga_header = {0};
	unsigned long bytes_read = 0;
  //TODO: in while loop just in case it cannot read all in one go
	BOOL res = ReadFile(handle, (void*)&tga_header, sizeof(tga_header), &bytes_read, (OVERLAPPED*)0);
  if (!res)
  {
    printf("Error reading file\n");
    return 0;
  }

  *image_width = tga_header.width;
  *image_height = tga_header.height;
  *image_depth = tga_header.bitsperpixel;

  printf("Bytes read: %lu\n", bytes_read);
  printf("width %d\n", tga_header.width);
  printf("height %d\n", tga_header.height);
  printf("bits per pixel %d\n", tga_header.bitsperpixel);

  void *rgb_data = 0;
  if (tga_header.datatypecode == 2)
  {
    printf("TGA RGB Uncompressed\n");
  
    unsigned long rgb_data_size = tga_header.width * tga_header.height * tga_header.bitsperpixel / 3;
    rgb_data = malloc(rgb_data_size);
    res = ReadFile(handle, rgb_data, rgb_data_size, &bytes_read, (OVERLAPPED*)0);
    if (!res)
    {
      printf("Error reading file\n");
      return 0;
    }
  }

 	printf("Bytes read: %lu\n", bytes_read);

	return rgb_data;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

void* img_loader_load(const char* const filename, int* image_width, int* image_height, int* image_depth)
{
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

  printf("Size of file %s: %lu\n", filename, file_info.nFileSizeLow);

  void* rgb_data = 0;
  const char* file_ext = get_filename_ext(filename);
  if (!strcmp(file_ext, "tga"))
  {
    printf("File extension is .tga\n");
    rgb_data = img_load_tga(handle, image_width, image_height, image_depth);
  }
  else
  {
    printf("File extension not supported\n");
  }

  return rgb_data;
}

