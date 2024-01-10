#pragma once

#include <util/Type.h>

namespace Graphics
{
	struct BasicVertex;

	struct Import
	{
		static void LoadOBJ(Graphics::BasicVertex& vertices, Vector<u16>& indices, const String& filename);
	};
}
