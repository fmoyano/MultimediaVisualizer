#include "png_loader.h"
#include <stdio.h>
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

  printf("%c\n", *buffer);
  printf("%c\n", *(buffer+1));
  printf("%c\n", *(buffer+2));
  printf("%c\n", *(buffer+3));

  if (validate_signature(buffer))
  {
    printf("Signature valid\n");

    //Read chunks: chunks begins with IHDR and end with IEND
    int pos = 8;
    //while ()

    return 0; //TODO: not sure what we will return
  }
  else
  {
    printf("Signature invalid\n");
    return 0;
  }

  return 0;
}
