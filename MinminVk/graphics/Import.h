#pragma once

#include <util/Type.h>

namespace Graphics
{
	struct BasicVertex;

	struct Import
	{
		static void LoadOBJ(Graphics::BasicVertex& vertices, Vector<u16>& indices, const String& filename);

		static void LoadHairStrands(Vector<f32>& vertices, const String& filename);

		static void LoadGLTF(const String& filename, Graphics::BasicVertex& vertices, Vector<u16>& indices, String& mainTextureURI);
	};
}
