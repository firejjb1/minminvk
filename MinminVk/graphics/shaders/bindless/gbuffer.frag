#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in mat3 fragTBN;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 1000.0f;

layout (location = 0) out vec4 outFB;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outPositionDepth;
layout (location = 3) out vec4 outNormal;
layout (location = 4) out vec4 outSpecular;

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

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() {
    // Get material data from bindless buffer
    MaterialData mat = materials[pc.materialIndex];

    vec3 N = normalize(inNormal);

    if (mat.hasNormalTex > 0 && pc.hasTangent > 0)
    {
        vec3 normalTex = texture(textures[nonuniformEXT(pc.normalIndex)], inTexCoord).rgb;
        N = normalize(normalTex * 2.0 - vec3(1.0));
        N = normalize(fragTBN * N);
    }

    if (mat.isDoubleSided == 1 && !gl_FrontFacing)
    {
        N = -N;
    }
    outNormal = vec4(N, 0.0);

    float metallic = mat.metallic;
	float roughness = mat.roughness;
	if (mat.hasMetallicRoughnessTex > 0)
    {
        vec3 mr = texture(textures[nonuniformEXT(pc.metallicIndex)], inTexCoord).rgb;
        metallic = mr.b;
        roughness = mr.g;
    }
	outSpecular = vec4(metallic, roughness, 0, 0);

    vec3 albedo = mat.baseColor.xyz * inColor.rgb;

    vec4 colorFromTex = texture(textures[nonuniformEXT(pc.albedoIndex)], inTexCoord);
    if (mat.hasAlbedoTex > 0)
        albedo *= colorFromTex.rgb;
    outAlbedo = vec4(albedo, 0);

    // Alpha cutoff
    if (mat.alphaMode == 2)
    {
        if (colorFromTex.a < mat.alphaCutoff)
            discard;
    }

    if (mat.hasEmissiveTex > 0)
    {
        vec3 emissive = texture(textures[nonuniformEXT(pc.emissiveIndex)], inTexCoord).rgb * mat.emissiveColor.rgb;
        // pack emissive into outAlbedo.a, outSpecular.b, outSpecular.a
        outAlbedo = vec4(albedo, emissive.r);
		outSpecular = vec4(outSpecular.rg, emissive.g, emissive.b);
    }

    // Store linearized depth in alpha component
	outPositionDepth.a = linearDepth(gl_FragCoord.z);
  }