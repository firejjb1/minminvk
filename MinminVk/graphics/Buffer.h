#pragma once

#include <util/Type.h>
#include <graphics/Resource.h>

namespace Graphics
{
    struct Buffer
    {
        enum class BufferType { STRUCTURED, UNIFORM };
        enum class BufferUsageType
        {
            NONE = 0,
            BUFFER_VERTEX = 1,
            BUFFER_TRANSFER_DST = 2,
            BUFFER_STORAGE = 4,
            BUFFER_TRANSFER_SRC = 8,
            BUFFER_INDEX = 16,
            BUFFER_UNIFORM = 32
        };
        enum class AccessType { READONLY, WRITE };
        u32 layoutID = 0;
        // sometimes a buffer is needed for each frame in flight. don't want to declare multiple buffers for each, managed by backend
        Vector<u32> extendedBufferIDs;
        virtual const ResourceBinding GetBinding() const = 0;
        virtual const BufferType GetBufferType() const = 0;
        virtual const AccessType GetAccessType() const = 0;
        virtual const u32 GetBufferSize() const = 0;
        virtual const BufferUsageType GetUsageType() const = 0;
        private:
        BufferType bufferType;
        AccessType accessType;
    };

    struct UniformBuffer : Buffer
    {
        virtual void Init();

    };

    struct BasicUniformBuffer : UniformBuffer
    {
        struct TransformUniform
        {
            alignas(16) mat4 model = mat4(1);
            alignas(16) mat4 view = mat4(1);
            alignas(16) mat4 proj = mat4(1);
        };

        TransformUniform transformUniform;

        const ResourceBinding GetBinding() const override
        {
            ResourceBinding uboBinding;
            uboBinding.binding = 0;
            uboBinding.shaderStageType = ResourceBinding::ShaderStageType::VERTEX;
            return uboBinding;
        }

        const BufferType GetBufferType() const override { return Buffer::BufferType::UNIFORM; }
        const AccessType GetAccessType() const override 
        {
            return AccessType::READONLY;
        }
        const u32 GetBufferSize() const override { return sizeof(TransformUniform); }
        const BufferUsageType GetUsageType() const override {
            return Buffer::BufferUsageType::BUFFER_UNIFORM;
        }

        BasicUniformBuffer() { Init(); }

    };

    struct StructuredBuffer : Buffer
    {
        ResourceBinding binding;

        Vector<f32> bufferData;

        AccessType accessType;

        Vector<BufferUsageType> usageTypes;

        StructuredBuffer(Vector<f32> &data, ResourceBinding &binding, AccessType accessType, Vector<BufferUsageType> usageTypes)
            : bufferData{ data }, binding{ binding }, accessType{ accessType }, usageTypes{ usageTypes } 
        {
            Init();
        }

        const ResourceBinding GetBinding() const override
        {
            return binding;
        }
        const BufferType GetBufferType() const override { return Buffer::BufferType::STRUCTURED; }

        const AccessType GetAccessType() const override { return accessType; }

        const u32 GetBufferSize() const override { return sizeof(bufferData); }

        void Init();

        const BufferUsageType GetUsageType() const override {
            BufferUsageType usage = BufferUsageType::NONE;
            if (usageTypes.size() > 0)
            {
                usage = usageTypes[0];
                for (int i = 1; i < usageTypes.size(); ++i)
                {
                    usage = EnumBitwiseOr(usage, usageTypes[i]);
                }
            }
            return usage;
        }
    };
}
