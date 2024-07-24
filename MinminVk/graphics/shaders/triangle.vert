#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} pushConst;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

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