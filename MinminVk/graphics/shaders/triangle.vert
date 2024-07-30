#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 lightDirection;
    vec4 cameraPosition;
    vec4 lightIntensity;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} pushConst;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

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
} uboMat;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPosWS;

void main() { 
    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragPosWS = vec3(pushConst.modelMatrix * vec4(inPosition, 1.0));
    gl_Position = ubo.proj * ubo.view * vec4(fragPosWS, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = normalize(transpose(inverse(pushConst.modelMatrix)) * vec4(inNormal, 0)).xyz;
}