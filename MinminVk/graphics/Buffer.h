#include <util/Type.h>

namespace Graphics
{
    struct BufferBinding
    {
        u32 binding;
        enum class ShaderStageType { VERTEX, FRAGMENT, COMPUTE, ALL_GRAPHICS };
        ShaderStageType shaderStageType;
    };

    struct Buffer
    {
        enum class BufferType { SHADERSTORAGE, UNIFORM };
        BufferType bufferType;

        u32 layoutID = 0;
        u32 bufferID = 0;
        virtual BufferBinding GetBinding() = 0;
    };


    struct BasicUniformBuffer : Buffer
    {
        struct TransformUniform
        {
            alignas(16) mat4 model = mat4(1);
            alignas(16) mat4 view = mat4(1);
            alignas(16) mat4 proj = mat4(1);
        };

        TransformUniform transformUniform;

        BufferBinding GetBinding() override
        {
            BufferBinding uboBinding;
            uboBinding.binding = 0;
            uboBinding.shaderStageType = BufferBinding::ShaderStageType::VERTEX;
            return uboBinding;
        }
    };
}
