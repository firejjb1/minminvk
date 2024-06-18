#ifndef IO_UTIL_H
#define IO_UTIL_H

#include <fstream>

#include "Type.h"

#include <tiny_gltf.h>

namespace Util
{
	struct IO
	{
		static Vector<char> ReadFile(const String& filename);
		static void ReadFloats(Vector<f32>& buffer, const String& filename);

		static unsigned char* ReadImage(i32 &width, i32 &height, const String& filename);

		static bool ReadGLTF(tinygltf::Model& model, const String& filename);

	};
}


#endif