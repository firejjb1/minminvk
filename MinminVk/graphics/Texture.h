#pragma once
#include <graphics/Resource.h>

namespace Graphics
{
	struct Texture;

	struct Sampler
	{
		u32 id = 0;
		enum class FilterType { POINT, LINEAR };
		enum class AddressModeType { REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER };
		FilterType magFilter = FilterType::LINEAR;
		FilterType minFilter = FilterType::LINEAR;
		FilterType mipFilter = FilterType::LINEAR;
		AddressModeType addressModeU = AddressModeType::CLAMP_TO_BORDER;
		AddressModeType addressModeV = AddressModeType::CLAMP_TO_BORDER;
		AddressModeType addressModeW = AddressModeType::CLAMP_TO_BORDER;
		u32 maxAnisotropy = 16;
		u32 maxLOD = 16;
		Sampler();
	};

	struct TextureID
	{
		u32 id = 0; // texture image
		u32 memoryID = 0; // texture memory
		u32 viewID = 0; // texture image view
		u32 samplerID = 0;
	};
	struct Texture
	{
		enum class FormatType { RGBA8_UNORM, RGBA8_SRGB, BGRA_SRGB, R10G10B10A2_UNORM_PACK32, COUNT };
		FormatType formatType = FormatType::RGBA8_SRGB;
		ResourceBinding binding;

		enum class LayoutType { 
			COLOR_ATTACHMENT, 
			PRESENT_SRC, 
			TRANSFER_SRC,
			TRANSFER_DST,
			READ_ONLY,
			DEPTH_ATTACHMENT,
			UNDEFINED 
		};
		
		enum class UsageType : unsigned int {
			COLOR_ATTACHMENT = 1,
			PRESENT_SRC = 2,
			TRANSFER_SRC = 4,
			TRANSFER_DST = 8,
			READ_ONLY = 16,
			SAMPLED = 32,
			DEPTH_ATTACHMENT = 64,
			STORAGE = 128,
			UNDEFINED = 256
		};

		UsageType usageType = EnumBitwiseOr(EnumBitwiseOr(UsageType::TRANSFER_DST, UsageType::SAMPLED), UsageType::TRANSFER_SRC);

		enum class TilingType
		{
			LINEAR,
			OPTIMAL
		};

		TilingType tilingType = TilingType::OPTIMAL;

		TextureID textureID;

		bool autoMipChain = false;

		bool initialized = false;
		
		u32 mipLevels = 1;

		u32 depth = 1;

		Texture(String filename, bool autoMipChain = false);

		Texture(){}
	};

}