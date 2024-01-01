#pragma once

namespace Graphics
{
	struct Texture
	{
		enum class FormatType { RGBA8_UNORM, RGBA8_SRGB, R10G10B10A2_UNORM_PACK32, COUNT };
		FormatType formatType;

		enum class LayoutType { 
			COLOR_ATTACHMENT, 
			PRESENT_SRC, 
			TRANSFER_SRC,
			TRANSFER_DST,
			READ_ONLY,
			DEPTH_ATTACHMENT,
			UNDEFINED
		};
	};

}