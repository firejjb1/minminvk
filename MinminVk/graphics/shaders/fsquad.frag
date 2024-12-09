#version 450

#include "lightingcommon.glsl"

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferPass {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 lightDirection;
    vec4 cameraPosition;
    vec4 lightIntensity;
} uboPass;

layout (input_attachment_index = 0, binding = 1) uniform subpassInput albedoAttachment;
layout (input_attachment_index = 1, binding = 2) uniform subpassInput positionDepthAttachment;
layout (input_attachment_index = 2, binding = 3) uniform subpassInput normalAttachment;
layout (input_attachment_index = 3, binding = 4) uniform subpassInput specularAttachment;

void main() 
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = subpassLoad(positionDepthAttachment).rgb;
	vec3 normal = subpassLoad(normalAttachment).rgb;
	vec4 albedo = subpassLoad(albedoAttachment);
	vec4 specular = subpassLoad(specularAttachment);
    float metallic = specular.r;
    float roughness = specular.g;
	  	   
	vec3 lightIntensity = vec3(uboPass.lightIntensity);
    vec3 l_metal_brdf = vec3(0.0);

    vec3 n = normalize(normal);
    vec3 l = normalize(uboPass.lightDirection.xyz);
    vec3 v = normalize(uboPass.cameraPosition.xyz - fragPos); 
    vec3 h = normalize(l + v);         

    metallic = clamp(metallic, 0, 1);
    roughness = clamp(roughness, 0, 1);
    float intensity = 1; // TODO (light attenuation)

    float NdotV = clampedDot(n, v);
    float NdotH = clampedDot(n, h);
    float LdotH = clampedDot(l, h);
    float VdotH = clampedDot(v, h);
    float NdotL = clampedDot(n, l);
    vec3 l_diffuse = NdotL * lightIntensity * BRDF_lambertian(albedo.rgb);
    vec3 l_specular_dielectric = vec3(0.0);
    vec3 l_specular_metal = vec3(0.0);
    vec3 l_dielectric_brdf = vec3(0.0);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo.rgb, metallic);
    vec3 metal_fresnel = F_Schlick(albedo.rgb, vec3(1.0), abs(VdotH));
    float alphaRoughness = roughness * roughness;
    l_specular_metal = intensity * NdotL * BRDF_specularGGX(alphaRoughness, NdotL, NdotV, NdotH);
    l_specular_dielectric = l_specular_metal;
     l_metal_brdf = metal_fresnel * l_specular_metal;
    vec3 dielectric_fresnel = F_Schlick(F0, abs(VdotH));
    l_dielectric_brdf = mix(l_diffuse, l_specular_dielectric, dielectric_fresnel);
    vec3 l_color = mix(l_dielectric_brdf, l_metal_brdf, metallic);

    vec3 emissive = vec3(albedo.a, specular.b, specular.a);
    l_color += emissive;
    
    // HACK: set alpha 0 if albedo is zero so that skybox can render 
	outColor = vec4(l_color, (albedo.r * albedo.g * albedo.b) > 0);
	//outColor = vec4(inUV, 0, 1);
}
