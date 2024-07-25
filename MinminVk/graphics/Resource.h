#pragma once
namespace Graphics
{
	struct ResourceBinding
    {
        u32 binding = 0;
        enum class ShaderStageType { VERTEX, FRAGMENT, COMPUTE, ALL_GRAPHICS };
        ShaderStageType shaderStageType = ShaderStageType::FRAGMENT;
    };

}