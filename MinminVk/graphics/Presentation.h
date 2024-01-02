#pragma once

#include <graphics/Texture.h>

namespace Graphics
{
	struct Presentation
	{
		// pointer to window object (glfw, could be other if added)
		void* window;

		struct SwapChainDetails
		{
			u32 targetImageCount = 2;
			//u32 maxImageCount = 0; // 0 means no maximum
			/*u32 width;
			u32 height;*/
			Texture::FormatType format = Texture::FormatType::RGBA8_SRGB;
			enum class ColorSpaceType { SRGB_NOLINEAR , COUNT };
			ColorSpaceType colorSpace = ColorSpaceType::SRGB_NOLINEAR;
			enum class ModeType { FIFO, FIFO_RELAXED, MAILBOX, IMMEDIATE, COUNT };
			ModeType mode = ModeType::FIFO;

			Vector<TextureID> textureIDs;
		};

		SwapChainDetails swapChainDetails;

		// driver and window system integration
		void Init(void* window);

		// happens after device init
		void InitSwapChain();

		void CleanUp();
	};
}