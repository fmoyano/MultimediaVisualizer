#include "png_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>

#define MAX_SIZE 16*1024*1024

typedef struct PNG_Chunk
{
  unsigned int length;
  unsigned int chunk_type;
  unsigned char* chunk_data;
  unsigned int chunk_CRC;
} PNG_Chunk;

//Changes the order of bytes
//This is required when reading data that should be interpreted as big-endian
//on a little-endian machine, or vice-versa
//In the case of PNGs, data is laid out in network/big-endian order
//and this machine is little endian
//Although most machines are currently little endian, to make this code portable
//we should test the endianness of the machine to know whether we must call this function
unsigned char* fix_endianness(unsigned char* bytes, size_t size)
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
