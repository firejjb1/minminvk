#include "IO.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
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

void IO::ReadFloats(Vector<f32> &buffer, const String& filename)
{
	DebugPrint("opening %s\n", filename.c_str());
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	float value;
	while (file >> value) {
		buffer.push_back(value);
	}
}

unsigned char* IO::ReadImage(i32& width, i32& height, const String& filename)
{
	int texWidth, texHeight, texChannels;
	//stbi_set_flip_vertically_on_load(true);
	unsigned char * data = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!data) {
		throw std::runtime_error("failed to load texture image! " + filename);
	}
	width = texWidth;
	height = texHeight;
	return data;
}

bool IO::ReadGLTF(tinygltf::Model& model, const String& filename)
{
	// loadModel
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename.c_str());

	if (!warn.empty()) {
	//	DebugPrint("WARN: %s\n", warn);
	}

	if (!err.empty()) {
	//	DebugPrint("ERR: %s\n", err);
	}

	if (!res)
		DebugPrint("Failed to load glTF: %s\n", filename.c_str());
	else
		DebugPrint("Loaded glTF: %s\n", filename.c_str());

	return res;
}

}

