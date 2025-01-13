#include "png_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_SIZE 16*1024*1024

typedef struct PNG_Chunk
{
  unsigned int length;
  unsigned int chunk_type;
  unsigned char* chunk_data;
  unsigned int chunk_CRC;
} PNG_Chunk;

typedef enum {Greyscale = 0, 
            Truecolour = 2, IndexedColour, GreyscaleAlpha,
            TruecolourAlpha = 6} PNG_Colour_Type;
static const char* colour_type_names[] =
{
  "Greyscale", "Unknown", "Truecolour", "IndexedColour",
  "GreyscaleAlpha", "Unknown", "TruecolourAlpha"
};
//Changes the order of bytes
//This is required when reading data that should be interpreted as big-endian
//on a little-endian machine, or vice-versa
//In the case of PNGs, data is laid out in network/big-endian order
//and this machine is little endian
//Although most machines are currently little endian, to make this code portable
//we should test the endianness of the machine to know whether we must call this function
static unsigned char* fix_endianness(unsigned char* bytes, size_t size)
{
  if (size == 1) return bytes;

  unsigned char* result = calloc(1, size);
  for (size_t i = 0; i < size / 2; ++i)
  {
    size_t complementary = (size - 1) - i;
    unsigned char byte = bytes[i];
    unsigned char byte_comp = bytes[complementary];
    result[i] = byte_comp;
    result[complementary] = byte;
  }

  return result;
}

typedef void (*chunk_handler)(unsigned char* buffer, int len);

typedef struct Handler_Struct
{
  char* chunk_type;
  chunk_handler handler;
} Handler_Struct;


void ihdr_handler(unsigned char* buffer, int len)
{  
  assert(len == 13);

  int width = *(int*)fix_endianness(buffer, 4);
  int height = *(int*)fix_endianness(buffer + 4, 4);
  unsigned char bit_depth = *(buffer + 8);
  unsigned char color_type = *(buffer + 9);
  unsigned char compression_method = *(buffer + 10);
  unsigned char filter_method = *(buffer + 11);
  unsigned char interlace_method = *(buffer + 12);

  printf("\tWidth: %d\n", width);
  printf("\tHeight: %d\n", height);
  printf("\tBit depth: %d\n", bit_depth);
  printf("\tColor type: %s\n", colour_type_names[color_type]);
  printf("\tCompression method: %d\n", compression_method);
  printf("\tFilter method: %d\n", filter_method);
  printf("\tInterlace method: %d\n", interlace_method);
}

void phys_handler(unsigned char* buffer, int len)
{
  (void)len;

  unsigned int pixels_per_unit_x = *(unsigned int*)fix_endianness(buffer, sizeof(unsigned int));
  unsigned int pixels_per_unit_y = *(unsigned int*)fix_endianness(buffer + 4, sizeof(unsigned int));
  unsigned char unit_specifier = *(buffer + 5);
  
  printf("\tPixels per unit X: %d\n", pixels_per_unit_x);
  printf("\tPixels per unit Y: %d\n", pixels_per_unit_y);
  printf("\tUnit specifier: %d\n", unit_specifier);
}

void itxt_handler(unsigned char* buffer, int len)
{
  (void)len;

  char keyword[80];
  strcpy_s(keyword, sizeof(keyword), (const char*)buffer);
  buffer += strlen(keyword);

  printf("\tKeyword: %s\n", keyword);
  //There is more info that we're not currently interested in
}

void idat_handler(unsigned char* buffer, int len)
{
  (void)len;
  (void)buffer;
  printf("\tHandler for IDAT called!\n");
}

void end_handler(unsigned char* buffer, int len)
{
  (void)len;
  (void)buffer;

  printf("\tHandler for IEND called!\n");
}

static Handler_Struct handlers[] =
{
  {"IHDR", ihdr_handler},
  {"IEND", end_handler},
  {"pHYs", phys_handler},
  {"iTXt", itxt_handler},
  {"IDAT", idat_handler},
  {"NULL", 0}
};

static bool validate_signature(unsigned char* bytes)
{
  return bytes[0] == 0x89 && bytes[1] == 'P' && bytes[2] == 'N' && bytes[3] == 'G' &&
    bytes[4] == 0x0D && bytes[5] == 0x0A && bytes[6] == 0x1A && bytes[7] == 0x0A;
}

void* png_loader_open(const char* const filename)
{
	printf("Filepath of png: %s\n", filename);

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

  unsigned char buffer[MAX_SIZE]; //TODO: read initial signature: https://progbook.org/png.html
  unsigned long bytes_read = 0;
	BOOL res = ReadFile(handle, (void*)buffer, sizeof(buffer), &bytes_read, (OVERLAPPED*)0);
  if (!res)
  {
    printf("Error reading file\n");
    return 0;
  }
  else
  {
    printf("Read %lu bytes\n", bytes_read);
  }

  if (validate_signature(buffer))
  {
    int pos = 8;
    printf("Signature valid\n");

    char chunk_type[5] = {0};
    while(strcmp(chunk_type, "IEND"))
    {
      //Read chunks: chunks begins with IHDR and end with IEND
      unsigned int length = 0;    
      memcpy(&length, &buffer[pos], sizeof(length)); //TODO: fix endianness
      length = *(int*)fix_endianness((unsigned char*)&length, sizeof(length));
      pos += sizeof(length);
      printf("length: %d\n", length);

      memcpy(chunk_type, &buffer[pos], 4);//sizeof(chunk_type));
      //chunk_type = *(int*)fix_endianness((unsigned char*)&chunk_type, sizeof(chunk_type));
      chunk_type[4] = '\0';
      pos += 4;//sizeof(chunk_type);
      printf("chunk_type: %s\n", chunk_type);

      unsigned char* chunk_data = malloc(length);
      memcpy(chunk_data, &buffer[pos], length);
      pos += length;

      for (int i = 0; handlers[i].handler != 0; ++i)
      {
        if (!strcmp(chunk_type, handlers[i].chunk_type))
        {
          handlers[i].handler(chunk_data, length);
        }
      }

      unsigned int chunk_CRC = 0;
      memcpy(&chunk_CRC, &buffer[pos+4], sizeof(chunk_CRC));
      chunk_CRC = *(int*)fix_endianness((unsigned char*)&chunk_CRC, sizeof(chunk_CRC));
      pos += sizeof(chunk_CRC);
      printf("CRC: %d\n", chunk_CRC);
  }

    return 0; //TODO: not sure what we will return
  }
  else
  {
    printf("Signature invalid\n");
    return 0;
  }

  return 0;
}
