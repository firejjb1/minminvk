#pragma once

#include <util/Type.h>

namespace Graphics
{
    struct Material
    {
        virtual const u32 GetDataSize() = 0;
        virtual void* GetData() = 0;
    };

    struct PBRMaterial : public Material
    {
        struct MaterialData
        {
            vec4 baseColor = vec4(1);
            vec4 emissiveColor = vec4(0);
            f32 metallic = 0.f;
            f32 roughness = 0.5f;
            u32 hasAlbedoTex = 0;
            u32 hasMetallicRoughnessTex = 0;
            u32 hasNormalTex = 0;
            u32 hasOcclusionTex = 0;
            u32 hasEmissiveTex = 0;
            f32 padding;
        };
        UniquePtr<MaterialData> material;

        Texture albedoTexture;
        Texture metallicTexture;
        Texture normalTexture;
        Texture occlusionTexture;
        Texture emissiveTexture;

        PBRMaterial()
        {
            material = MakeUnique<MaterialData>();
            albedoTexture.binding.binding = 0;
            metallicTexture.binding.binding = 1;
            normalTexture.binding.binding = 2;
            occlusionTexture.binding.binding = 3;
            emissiveTexture.binding.binding = 4;
        }
        
        const u32 GetDataSize() override
        {
            return sizeof(MaterialData);
        }

        void* GetData() override 
        {
            return (void *)material.get();
        }
    };
}