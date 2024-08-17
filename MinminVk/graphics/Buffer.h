#pragma once

#include <util/Type.h>
#include <graphics/Resource.h>
#include <graphics/Material.h>

namespace Graphics
{
    struct RenderContext;

    struct Buffer
    {
        enum class BufferType { STRUCTURED, UNIFORM, VERTEX };
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

    struct VertexBuffer : Buffer
    {
        ResourceBinding binding;
        u32 dataSize;
        AccessType accessType;

        VertexBuffer(u32 dataSize, AccessType accessType) : dataSize{ dataSize }, accessType{ accessType } {}

        const ResourceBinding GetBinding() const override
        {
            return binding;
        }
        const BufferType GetBufferType() const override { return Buffer::BufferType::VERTEX; }

        const AccessType GetAccessType() const override { return accessType; }

        const u32 GetBufferSize() const override
        {
            return dataSize;
        }

        const BufferUsageType GetUsageType() const override {
            BufferUsageType usage = BufferUsageType::BUFFER_VERTEX;
            return usage;
        }
    };

    struct UniformBuffer : Buffer
    {
        virtual void Init();
        virtual void* GetData() = 0;
        void UpdateUniformBuffer(int swapID);

    };

    struct BasicUniformBuffer : UniformBuffer
    {
        static const u32 jointMatricesSize = 128;

        struct TransformUniform
        {
            alignas(16) mat4 model = mat4(1);
            alignas(16) mat4 view = mat4(1);
            alignas(16) mat4 proj = mat4(1);
            vec4 lightDirection = vec4(1);
            vec4 cameraPosition;
            vec4 lightIntensity = vec4(1);
            alignas(16) mat4 jointMatrices[jointMatricesSize];
        };

        TransformUniform transformUniform;

        void *GetData() override
        {
            return (void*)&transformUniform;
        }

        const ResourceBinding GetBinding() const override
        {
            ResourceBinding uboBinding;
            uboBinding.binding = 0;
            uboBinding.shaderStageType = ResourceBinding::ShaderStageType::ALL_GRAPHICS;
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

    struct SkeletonUniformBuffer : UniformBuffer
    {
        static const u32 jointMatricesSize = 128;

        Vector<mat4> data;

        void *GetData() override
        {
            return (void*)data.data();
        }

        const ResourceBinding GetBinding() const override
        {
            ResourceBinding uboBinding;
            uboBinding.binding = 3;
            uboBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;
            return uboBinding;
        }

        const BufferType GetBufferType() const override { return Buffer::BufferType::UNIFORM; }
        const AccessType GetAccessType() const override 
        {
            return AccessType::READONLY;
        }
        const u32 GetBufferSize() const override { return data.size() * sizeof(mat4); }
        const BufferUsageType GetUsageType() const override {
            return Buffer::BufferUsageType::BUFFER_UNIFORM;
        }

        SkeletonUniformBuffer() { data.resize(jointMatricesSize); Init(); }

    };
    
    struct BlendWeightsUniformBuffer : UniformBuffer
    {
        static const u32 blendWeightsSize = 8;

        Vector<f32> data;

        void *GetData() override
        {
            return (void*)data.data();
        }

        const ResourceBinding GetBinding() const override
        {
            ResourceBinding uboBinding;
            uboBinding.binding = 4;
            uboBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;
            return uboBinding;
        }

        const BufferType GetBufferType() const override { return Buffer::BufferType::UNIFORM; }
        const AccessType GetAccessType() const override 
        {
            return AccessType::READONLY;
        }
        const u32 GetBufferSize() const override { return data.size() * sizeof(f32); }
        const BufferUsageType GetUsageType() const override {
            return Buffer::BufferUsageType::BUFFER_UNIFORM;
        }

        BlendWeightsUniformBuffer() { data.resize(blendWeightsSize); Init(); }

    };

    struct PBRUniformBuffer : UniformBuffer
    {
      
        PBRMaterial* pbrMaterial;
        PBRMaterial defaultPBRMaterial;

        void* GetData() override
        {
            if (!pbrMaterial)
                return (void*)defaultPBRMaterial.GetData();
            return pbrMaterial->GetData();
        }

        const ResourceBinding GetBinding() const override
        {
            ResourceBinding uboBinding;
            uboBinding.binding = 5;
            uboBinding.shaderStageType = ResourceBinding::ShaderStageType::ALL_GRAPHICS;
            return uboBinding;
        }

        const BufferType GetBufferType() const override 
        {
            return Buffer::BufferType::UNIFORM;
        }

        const AccessType GetAccessType() const override
        {
            return AccessType::READONLY;
        }

        const u32 GetBufferSize() const override 
        { 
            return sizeof(PBRMaterial::MaterialData);
        }

        const BufferUsageType GetUsageType() const override 
        {
            return Buffer::BufferUsageType::BUFFER_UNIFORM;
        }

        PBRUniformBuffer() 
        {
            Init(); 
            pbrMaterial = &defaultPBRMaterial;
        }

    };

    struct StructuredBuffer : Buffer
    {
        ResourceBinding binding;

        Vector<f32> bufferData;

        Vector<BufferUsageType> usageTypes;

        StructuredBuffer(Vector<f32> &data, ResourceBinding &binding, Vector<BufferUsageType> usageTypes)
            : bufferData{ data }, binding{ binding }, usageTypes{ usageTypes } 
        {
            Init();
        }
        
        StructuredBuffer(u8 *data, u32 dataSize, ResourceBinding &binding, Vector<BufferUsageType> usageTypes)
            :  binding{ binding }, usageTypes{ usageTypes } 
        {
            bufferData.resize(dataSize * sizeof(u8) / sizeof(f32));
            u32 numf32 = dataSize * sizeof(u8) / sizeof(f32);
            f32* dataPtr = (f32*)data;
            for (int i = 0; i < numf32; ++i)
            {
                bufferData[i] = *(dataPtr + i);
            }
            Init();
        }

        // initialize using existing buffers. from extendedBufferIDS
        StructuredBuffer(Vector<f32>& data, Vector<u32>& extendedBufferIDs, ResourceBinding& binding, Vector<BufferUsageType> usageTypes)
            : bufferData{ data }, binding { binding }, usageTypes{ usageTypes }
        {
            this->extendedBufferIDs = extendedBufferIDs;
        }

        const ResourceBinding GetBinding() const override
        {
            return binding;
        }
        const BufferType GetBufferType() const override { return Buffer::BufferType::STRUCTURED; }

        const AccessType GetAccessType() const override { return AccessType::WRITE; }

        const u32 GetBufferSize() const override 
        { 
            return bufferData.size() * sizeof(f32); 
        }

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

        // only if also a vertex buffer
        void DrawBuffer(RenderContext& context, u32 numVertex);
    };
}
