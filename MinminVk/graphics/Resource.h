#pragma once
namespace Graphics
{
	struct ResourceBinding
    {
        u32 binding;
        enum class ShaderStageType { VERTEX, FRAGMENT, COMPUTE, ALL_GRAPHICS };
        ShaderStageType shaderStageType;
    };

}