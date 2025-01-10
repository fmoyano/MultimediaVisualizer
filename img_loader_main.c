#include "img_loader.h"

int main()
{
	int width, height;
	unsigned char* bytes = img_loader_load("data/sample.tga", &width, &height);

	return 0;
}