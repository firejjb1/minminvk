#version 450

#extension GL_KHR_vulkan_glsl : enable 

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosWS;
layout(location = 4) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
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
} uboMat;

// From https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/brdf.glsl
//
// Fresnel
//
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// https://github.com/wdas/brdf/tree/master/src/brdfs
// https://google.github.io/filament/Filament.md.html
//
const float M_PI = 3.141592653589793;
float clampedDot(vec3 x, vec3 y)
{
    return clamp(dot(x, y), 0.0, 1.0);
}


// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 F_Schlick(vec3 f0, vec3 f90, float VdotH) 
{
    return f0 + (f90 - f0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

float F_Schlick(float f0, float f90, float VdotH)
{
    float x = clamp(1.0 - VdotH, 0.0, 1.0);
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return f0 + (f90 - f0) * x5;
}

float F_Schlick(float f0, float VdotH)
{
    float f90 = 1.0; //clamp(50.0 * f0, 0.0, 1.0);
    return F_Schlick(f0, f90, VdotH);
}

vec3 F_Schlick(vec3 f0, float f90, float VdotH)
{
    float x = clamp(1.0 - VdotH, 0.0, 1.0);
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return f0 + (f90 - f0) * x5;
}

vec3 F_Schlick(vec3 f0, float VdotH)
{
    float f90 = 1.0; //clamp(dot(f0, vec3(50.0 * 0.33)), 0.0, 1.0);
    return F_Schlick(f0, f90, VdotH);
}

vec3 Schlick_to_F0(vec3 f, vec3 f90, float VdotH) {
    float x = clamp(1.0 - VdotH, 0.0, 1.0);
    float x2 = x * x;
    float x5 = clamp(x * x2 * x2, 0.0, 0.9999);

    return (f - f90 * x5) / (1.0 - x5);
}

float Schlick_to_F0(float f, float f90, float VdotH) {
    float x = clamp(1.0 - VdotH, 0.0, 1.0);
    float x2 = x * x;
    float x5 = clamp(x * x2 * x2, 0.0, 0.9999);

    return (f - f90 * x5) / (1.0 - x5);
}

vec3 Schlick_to_F0(vec3 f, float VdotH) {
    return Schlick_to_F0(f, vec3(1.0), VdotH);
}

float Schlick_to_F0(float f, float VdotH) {
    return Schlick_to_F0(f, 1.0, VdotH);
}


// Smith Joint GGX
// Note: Vis = G / (4 * NdotL * NdotV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    float GGX = GGXV + GGXL;
    if (GGX > 0.0)
    {
        return 0.5 / GGX;
    }
    return 0.0;
}


// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float D_GGX(float NdotH, float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    float f = (NdotH * NdotH) * (alphaRoughnessSq - 1.0) + 1.0;
    return alphaRoughnessSq / (M_PI * f * f);
}


float lambdaSheenNumericHelper(float x, float alphaG)
{
    float oneMinusAlphaSq = (1.0 - alphaG) * (1.0 - alphaG);
    float a = mix(21.5473, 25.3245, oneMinusAlphaSq);
    float b = mix(3.82987, 3.32435, oneMinusAlphaSq);
    float c = mix(0.19823, 0.16801, oneMinusAlphaSq);
    float d = mix(-1.97760, -1.27393, oneMinusAlphaSq);
    float e = mix(-4.32054, -4.85967, oneMinusAlphaSq);
    return a / (1.0 + b * pow(x, c)) + d * x + e;
}


float lambdaSheen(float cosTheta, float alphaG)
{
    if (abs(cosTheta) < 0.5)
    {
        return exp(lambdaSheenNumericHelper(cosTheta, alphaG));
    }
    else
    {
        return exp(2.0 * lambdaSheenNumericHelper(0.5, alphaG) - lambdaSheenNumericHelper(1.0 - cosTheta, alphaG));
    }
}


float V_Sheen(float NdotL, float NdotV, float sheenRoughness)
{
    sheenRoughness = max(sheenRoughness, 0.000001); //clamp (0,1]
    float alphaG = sheenRoughness * sheenRoughness;

    return clamp(1.0 / ((1.0 + lambdaSheen(NdotV, alphaG) + lambdaSheen(NdotL, alphaG)) *
        (4.0 * NdotV * NdotL)), 0.0, 1.0);
}


//Sheen implementation-------------------------------------------------------------------------------------
// See  https://github.com/sebavan/glTF/tree/KHR_materials_sheen/extensions/2.0/Khronos/KHR_materials_sheen

// Estevez and Kulla http://www.aconty.com/pdf/s2017_pbs_imageworks_sheen.pdf
float D_Charlie(float sheenRoughness, float NdotH)
{
    sheenRoughness = max(sheenRoughness, 0.000001); //clamp (0,1]
    float alphaG = sheenRoughness * sheenRoughness;
    float invR = 1.0 / alphaG;
    float cos2h = NdotH * NdotH;
    float sin2h = 1.0 - cos2h;
    return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * M_PI);
}


//https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
vec3 BRDF_lambertian(vec3 diffuseColor)
{
    // see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    return (diffuseColor / M_PI);
}

//  https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
vec3 BRDF_specularGGX(float alphaRoughness, float NdotL, float NdotV, float NdotH)
{
    float Vis = V_GGX(NdotL, NdotV, alphaRoughness);
    float D = D_GGX(NdotH, alphaRoughness);

    return vec3(Vis * D);
}


#ifdef MATERIAL_ANISOTROPY
// GGX Distribution Anisotropic (Same as Babylon.js)
// https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf Addenda
float D_GGX_anisotropic(float NdotH, float TdotH, float BdotH, float anisotropy, float at, float ab)
{
    float a2 = at * ab;
    vec3 f = vec3(ab * TdotH, at * BdotH, a2 * NdotH);
    float w2 = a2 / dot(f, f);
    return a2 * w2 * w2 / M_PI;
}

// GGX Mask/Shadowing Anisotropic (Same as Babylon.js - smithVisibility_GGXCorrelated_Anisotropic)
// Heitz http://jcgt.org/published/0003/02/03/paper.pdf
float V_GGX_anisotropic(float NdotL, float NdotV, float BdotV, float TdotV, float TdotL, float BdotL, float at, float ab)
{
    float GGXV = NdotL * length(vec3(at * TdotV, ab * BdotV, NdotV));
    float GGXL = NdotV * length(vec3(at * TdotL, ab * BdotL, NdotL));
    float v = 0.5 / (GGXV + GGXL);
    return clamp(v, 0.0, 1.0);
}

vec3 BRDF_specularGGXAnisotropy(float alphaRoughness, float anisotropy, vec3 n, vec3 v, vec3 l, vec3 h, vec3 t, vec3 b)
{
    // Roughness along the anisotropy bitangent is the material roughness, while the tangent roughness increases with anisotropy.
    float at = mix(alphaRoughness, 1.0, anisotropy * anisotropy);
    float ab = clamp(alphaRoughness, 0.001, 1.0);

    float NdotL = clamp(dot(n, l), 0.0, 1.0);
    float NdotH = clamp(dot(n, h), 0.001, 1.0);
    float NdotV = dot(n, v);

    float V = V_GGX_anisotropic(NdotL, NdotV, dot(b, v), dot(t, v), dot(t, l), dot(b, l), at, ab);
    float D = D_GGX_anisotropic(NdotH, dot(t, h), dot(b, h), anisotropy, at, ab);

    return vec3(V * D);
}
#endif


// f_sheen
vec3 BRDF_specularSheen(vec3 sheenColor, float sheenRoughness, float NdotL, float NdotV, float NdotH)
{
    float sheenDistribution = D_Charlie(sheenRoughness, NdotH);
    float sheenVisibility = V_Sheen(NdotL, NdotV, sheenRoughness);
    return sheenColor * sheenDistribution * sheenVisibility;
}

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
    if (uboMat.hasNormalTex > 0)
    {
        vec3 normalTex = texture(texNormal, fragTexCoord).rgb;
        n = normalize(normalTex * 2. - vec3(1.));
        n = normalize(fragTBN * n);
    }
    
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
    F0      = mix(F0, albedo, metallic);
    vec3 metal_fresnel = F_Schlick(albedo, vec3(1.0), abs(VdotH));
    l_specular_metal = intensity * NdotL * BRDF_specularGGX(roughness, NdotL, NdotV, NdotH);
    l_specular_dielectric = l_specular_metal;
     l_metal_brdf = metal_fresnel * l_specular_metal;
    vec3 dielectric_fresnel = F_Schlick(F0, abs(VdotH));
    l_dielectric_brdf = mix(l_diffuse, l_specular_dielectric, dielectric_fresnel);
    vec3 l_color = mix(l_dielectric_brdf, l_metal_brdf, metallic);

    outColor = vec4(l_color, 1);
    //outColor = vec4( texture(texColor, fragTexCoord).rgb, 1);
    //outColor = vec4(uboMat.baseColor.xyz, 1);
    //outColor = vec4(n, 1);
    //outColor = vec4(roughness,0,0, 1);
}