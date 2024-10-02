#pragma once

#include <graphics/Presentation.h>

namespace Graphics
{
	struct UIRender
	{
		int poolID;

		UIRender(SharedPtr<Presentation> presentation);

		void Render(Graphics::RenderContext& context);
	};
}