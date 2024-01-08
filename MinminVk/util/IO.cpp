#include "IO.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Util
{
    
Vector<char> IO::ReadFile(const String& filename)
{
    DebugPrint("opening %s\n", filename.c_str());
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

unsigned char* IO::ReadImage(i32& width, i32& height, const String& filename)
{
	int texWidth, texHeight, texChannels;
	unsigned char * data = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!data) {
		throw std::runtime_error("failed to load texture image!");
	}
	width = texWidth;
	height = texHeight;
	return data;
}

}

