#version 450

#include "lightingcommon.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosWS;
layout(location = 4) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    //mat4 modelMatrix;
    layout(offset = 128) uint hasTangent;
} pushConst;

layout(binding = 0) uniform UniformBufferPass {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 lightDirection;
    vec4 cameraPosition;
    vec4 lightIntensity;
} uboPass;

layout(set = 1, binding = 0) uniform sampler2D texColor;
layout(set = 1, binding = 1) uniform sampler2D texMetallic;
layout(set = 1, binding = 2) uniform sampler2D texNormal;
layout(set = 1, binding = 3) uniform sampler2D texOcclusion;
layout(set = 1, binding = 4) uniform sampler2D texEmissive;

layout(set = 1, binding = 5) uniform UniformBufferMat {
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
} uboMat;

void main() {

    vec3 lightIntensity = vec3(uboPass.lightIntensity);
    vec3 l_metal_brdf = vec3(0.0);
    float metallic = uboMat.metallic; 
    float roughness = uboMat.roughness; 
    vec3 n = normalize(fragNormal);
    vec3 l = normalize(uboPass.lightDirection.xyz);
    vec3 v = normalize(uboPass.cameraPosition.xyz - fragPosWS); 
    vec3 h = normalize(l + v);         

    if (uboMat.hasMetallicRoughnessTex > 0)
    {
        vec3 mr = texture(texMetallic, fragTexCoord).rgb;
        metallic = mr.b;
        roughness = mr.g;
    }
    if (uboMat.hasNormalTex > 0 && pushConst.hasTangent > 0)
    {
        vec3 normalTex = texture(texNormal, fragTexCoord).rgb;
        n = normalize(normalTex * 2. - vec3(1.));
        n = normalize(fragTBN * n);
    }

    if (uboMat.isDoubleSided == 1 && !gl_FrontFacing)
    {
        n = -n;
    }
    
    metallic = clamp(metallic, 0, 1);
    roughness = clamp(roughness, 0, 1);
    float intensity = 1; // TODO (light attenuation)
    vec3 albedo = uboMat.baseColor.xyz * fragColor;
    
    vec4 colorFromTex = texture(texColor, fragTexCoord);
    if (uboMat.hasAlbedoTex > 0)
        albedo *= colorFromTex.rgb;

    // cutoff
    if (uboMat.alphaMode == 2)
    {
        if (colorFromTex.a < uboMat.alphaCutoff)
            discard;
    }

    if (uboMat.hasOcclusionTex > 0)
    {
        // direct lighting unaffected
        // TODO enable when implemented indirect lighting
        //vec3 occ = texture(texOcclusion, fragTexCoord).rgb;
        //float occFactor = 1.0 + uboMat.occlusionStrength * (occ.r - 1.0);
    }


    float NdotV = clampedDot(n, v);
    float NdotH = clampedDot(n, h);
    float LdotH = clampedDot(l, h);
    float VdotH = clampedDot(v, h);
    float NdotL = clampedDot(n, l);
    vec3 l_diffuse = NdotL * lightIntensity * BRDF_lambertian(albedo);
    vec3 l_specular_dielectric = vec3(0.0);
    vec3 l_specular_metal = vec3(0.0);
    vec3 l_dielectric_brdf = vec3(0.0);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    vec3 metal_fresnel = F_Schlick(albedo, vec3(1.0), abs(VdotH));
    float alphaRoughness = roughness * roughness;
    l_specular_metal = intensity * NdotL * BRDF_specularGGX(alphaRoughness, NdotL, NdotV, NdotH);
    l_specular_dielectric = l_specular_metal;
     l_metal_brdf = metal_fresnel * l_specular_metal;
    vec3 dielectric_fresnel = F_Schlick(F0, abs(VdotH));
    l_dielectric_brdf = mix(l_diffuse, l_specular_dielectric, dielectric_fresnel);
    vec3 l_color = mix(l_dielectric_brdf, l_metal_brdf, metallic);

    if (uboMat.hasEmissiveTex > 0)
    {
        vec3 emissive = texture(texEmissive, fragTexCoord).rgb * uboMat.emissiveColor.rgb;
        l_color += emissive;
    }


    outColor = vec4(l_color, colorFromTex.a * uboMat.baseColor.a);
    //outColor = vec4( texture(texColor, fragTexCoord).rgb, 1);
    //outColor = vec4(1);
    //outColor = vec4(fragNormal, 1);
    //outColor = vec4(roughness,0,0, 1);
}