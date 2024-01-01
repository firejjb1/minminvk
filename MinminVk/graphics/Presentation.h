#pragma once

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
			enum class FormatType { RGBA8_UNORM, RGBA8_SRGB, R10G10B10A2_UNORM_PACK32, COUNT };
			FormatType format = FormatType::RGBA8_SRGB;
			enum class ColorSpaceType { SRGB_NOLINEAR , COUNT };
			ColorSpaceType colorSpace = ColorSpaceType::SRGB_NOLINEAR;
			enum class ModeType { FIFO, FIFO_RELAXED, MAILBOX, IMMEDIATE, COUNT };
			ModeType mode = ModeType::FIFO;
		};

		SwapChainDetails swapChainDetails;

		// driver and window system integration
		void Init(void* window);

		// happens after device init
		void InitSwapChain();

		void CleanUp();
	};
}