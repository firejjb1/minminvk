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
        enum class BufferType { STRUCTURED, UNIFORM };
        enum class BufferUsageType
        {
            BUFFER_VERTEX = 1,
            BUFFER_TRANSFER_DST = 2,
            BUFFER_STORAGE = 4,
            BUFFER_TRANSFER_SRC = 8,
            BUFFER_INDEX = 16,
            BUFFER_UNIFORM = 32
        };
        enum class AccessType { READONLY, WRITE };
        u32 layoutID = 0;
        u32 bufferID = 0;
        virtual BufferBinding GetBinding() = 0;
        virtual BufferType GetBufferType() = 0;
        virtual AccessType GetAccessType() = 0;
        virtual u32 GetBufferSize() = 0;
        virtual BufferUsageType GetUsageType() = 0;
        private:
        BufferType bufferType;
        AccessType accessType;
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

        BufferType GetBufferType() override { return Buffer::BufferType::UNIFORM; }
        AccessType GetAccessType() override 
        {
            return AccessType::READONLY;
        }
        u32 GetBufferSize() override { return sizeof(TransformUniform); }
        BufferUsageType GetUsageType() override {
            return Buffer::BufferUsageType::BUFFER_UNIFORM;
        }

    };

    struct ParticleStructuredBuffer : Buffer
    {
        struct Particle {
            vec2 position;
            vec2 velocity;
            vec4 color;
        };

        Vector<Particle> bufferData;

        BufferBinding GetBinding() override 
        {
             BufferBinding binding;
             binding.binding = 0;
             binding.shaderStageType = BufferBinding::ShaderStageType::COMPUTE;
             return binding;
        }

        BufferType GetBufferType() override { return Buffer::BufferType::STRUCTURED; }
        AccessType GetAccessType() override 
        {
            return AccessType::READONLY;
        }
        u32 GetBufferSize() override { return sizeof(bufferData); }

        void Init();

        BufferUsageType GetUsageType() override {
            return EnumBitwiseOr(EnumBitwiseOr(Buffer::BufferUsageType::BUFFER_VERTEX,
                Buffer::BufferUsageType::BUFFER_STORAGE), Buffer::BufferUsageType::BUFFER_TRANSFER_DST);
        }

        ParticleStructuredBuffer(Vector<Particle> &particles)
            : bufferData { particles }
        {
            Init();
        }
    };
}
