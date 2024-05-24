#pragma once

#include <graphics/presentation.h>

namespace Graphics
{
	struct UIRender
	{
		int poolID;

		UIRender(SharedPtr<Presentation> presentation);

		void Render(Graphics::RenderContext& context);
	};
}