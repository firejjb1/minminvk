#pragma once

#include <util/Type.h>

namespace Graphics
{
    struct Material
    {
        virtual size_t GetDataSize() = 0;
        virtual void* GetData() = 0;
    };

    struct PBRMaterial : public Material
    {
        struct MaterialData
        {
            vec4 baseColor;
            vec4 metallicRoughness;
        };
        UniquePtr<MaterialData> material;
        
        size_t GetDataSize() override
        {
            return sizeof(PBRMaterial);
        }

        void* GetData() override 
        {
            return (void *)material.get();
        }
    };
}