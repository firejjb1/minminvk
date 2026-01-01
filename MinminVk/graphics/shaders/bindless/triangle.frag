#version 450
#extension GL_EXT_nonuniform_qualifier : require

#include "../lightingcommon.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosWS;
layout(location = 4) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

// Set 0: Per-frame uniforms (traditional)
layout(set = 0, binding = 0) uniform UniformBufferPass {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 lightDirection;
    vec4 cameraPosition;
    vec4 lightIntensity;
} uboPass;

// Set 1, Binding 0: Material storage buffer (all materials)
struct MaterialData {
    vec4 baseColor;
    vec4 emissiveColor;
    float metallic;
    float roughness;
    uint hasAlbedoTex;
    uint hasMetallicRoughnessTex;
    uint hasNormalTex;
    uint hasOcclusionTex;
    uint hasEmissiveTex;
    uint isDoubleSided;
    uint alphaMode;
    float alphaCutoff;
    float occlusionStrength;
};

layout(set = 1, binding = 0) readonly buffer MaterialBuffer {
    MaterialData materials[];
};

// Set 1, Binding 1: Bindless texture array
layout(set = 1, binding = 1) uniform sampler2D textures[];

// Push constants
layout(push_constant) uniform PushConstants {
    layout(offset = 128) uint hasTangent;
    uint materialIndex;
    uint albedoIndex;
    uint metallicIndex;
    uint normalIndex;
    uint occlusionIndex;
    uint emissiveIndex;
} pc;

void main() {
    // Get material data from bindless buffer
    MaterialData mat = materials[pc.materialIndex];

    vec3 lightIntensity = vec3(uboPass.lightIntensity);
    float metallic = mat.metallic;
    float roughness = mat.roughness;
    vec3 n = normalize(fragNormal);
    vec3 l = normalize(uboPass.lightDirection.xyz);
    vec3 v = normalize(uboPass.cameraPosition.xyz - fragPosWS);
    vec3 h = normalize(l + v);

    if (mat.hasMetallicRoughnessTex > 0)
    {
        vec3 mr = texture(textures[nonuniformEXT(pc.metallicIndex)], fragTexCoord).rgb;
        metallic = mr.b;
        roughness = mr.g;
    }

    if (mat.hasNormalTex > 0 && pc.hasTangent > 0)
    {
        vec3 normalTex = texture(textures[nonuniformEXT(pc.normalIndex)], fragTexCoord).rgb;
        n = normalize(normalTex * 2.0 - vec3(1.0));
        n = normalize(fragTBN * n);
    }

    if (mat.isDoubleSided == 1 && !gl_FrontFacing)
    {
        n = -n;
    }

    metallic = clamp(metallic, 0.0, 1.0);
    roughness = clamp(roughness, 0.0, 1.0);
    float intensity = 1.0;
    vec3 albedo = mat.baseColor.xyz * fragColor;

    vec4 colorFromTex = texture(textures[nonuniformEXT(pc.albedoIndex)], fragTexCoord);
    if (mat.hasAlbedoTex > 0)
        albedo *= colorFromTex.rgb;

    // Alpha cutoff
    if (mat.alphaMode == 2)
    {
        if (colorFromTex.a < mat.alphaCutoff)
            discard;
    }

    float NdotV = clampedDot(n, v);
    float NdotH = clampedDot(n, h);
    float LdotH = clampedDot(l, h);
    float VdotH = clampedDot(v, h);
    float NdotL = clampedDot(n, l);

    vec3 l_diffuse = NdotL * lightIntensity * BRDF_lambertian(albedo);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 metal_fresnel = F_Schlick(albedo, vec3(1.0), abs(VdotH));
    float alphaRoughness = roughness * roughness;
    vec3 l_specular_metal = intensity * NdotL * BRDF_specularGGX(alphaRoughness, NdotL, NdotV, NdotH);
    vec3 l_specular_dielectric = l_specular_metal;
    vec3 l_metal_brdf = metal_fresnel * l_specular_metal;
    vec3 dielectric_fresnel = F_Schlick(F0, abs(VdotH));
    vec3 l_dielectric_brdf = mix(l_diffuse, l_specular_dielectric, dielectric_fresnel);
    vec3 l_color = mix(l_dielectric_brdf, l_metal_brdf, metallic);

    if (mat.hasEmissiveTex > 0)
    {
        vec3 emissive = texture(textures[nonuniformEXT(pc.emissiveIndex)], fragTexCoord).rgb * mat.emissiveColor.rgb;
        l_color += emissive;
    }

    outColor = vec4(l_color, colorFromTex.a * mat.baseColor.a);
  }