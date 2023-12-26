#pragma once

namespace Graphics
{
	struct Presentation
	{
		// pointer to window object (glfw, could be other if added)
		void* window;

		// driver and window system integration
		void Init(void* window);

		void CleanUp();
	};
}