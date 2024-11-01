#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in mat3 fragTBN;

layout (location = 0) out vec4 outFB;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outPositionDepth;
layout (location = 3) out vec4 outNormal;
layout (location = 4) out vec4 outSpecular;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 1000.0f;

layout(push_constant) uniform PushConstants {
    //mat4 modelMatrix;
    layout(offset = 128) uint hasTangent;
} pushConst;

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

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() 
{
	outPositionDepth = vec4(inWorldPos, 1.0);

	vec3 N = normalize(inNormal);
    if (uboMat.hasNormalTex > 0 && pushConst.hasTangent > 0)
    {
        vec3 normalTex = texture(texNormal, inTexCoord).rgb;
        N = normalize(normalTex * 2. - vec3(1.));
        N = normalize(fragTBN * N);
    }
    if (uboMat.isDoubleSided == 1 && !gl_FrontFacing)
    {
        N = -N;
    }
	outNormal = vec4(N, 0.0);

 	vec3 albedo = uboMat.baseColor.xyz * inColor.rgb;
    
    vec4 colorFromTex = texture(texColor, inTexCoord);
    if (uboMat.alphaMode == 2)
    {
        if (colorFromTex.a < uboMat.alphaCutoff)
            discard;
    }
    if (uboMat.hasAlbedoTex > 0)
        albedo *= colorFromTex.rgb;
	outAlbedo = vec4(albedo, 0);

	float metallic = uboMat.metallic;
	float roughness = uboMat.roughness;
	if (uboMat.hasMetallicRoughnessTex > 0)
    {
        vec3 mr = texture(texMetallic, inTexCoord).rgb;
        metallic = mr.b;
        roughness = mr.g;
    }
	outSpecular = vec4(metallic, roughness, 0, 0);

	if (uboMat.hasEmissiveTex > 0)
    {
        vec3 emissive = texture(texEmissive, inTexCoord).rgb * uboMat.emissiveColor.rgb;
		// pack emissive into outAlbedo.a, outSpecular.b, outSpecular.a
        outAlbedo = vec4(albedo, emissive.r);
		outSpecular = vec4(outSpecular.rg, emissive.g, emissive.b);
    }

	// Store linearized depth in alpha component
	outPositionDepth.a = linearDepth(gl_FragCoord.z);

    // test
    // outFB = vec4(albedo);
}