#pragma once
namespace Graphics
{
	struct ResourceBinding
    {
        u32 binding = 1;
        enum class ShaderStageType { VERTEX, FRAGMENT, COMPUTE, ALL_GRAPHICS };
        ShaderStageType shaderStageType = ShaderStageType::FRAGMENT;
    };

}