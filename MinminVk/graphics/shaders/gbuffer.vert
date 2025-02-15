#version 450

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec4 outColor;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec2 outTexCoord;
layout(location = 4) out mat3 fragTBN;

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
    mat4 inverseTransposeModel;
} pushConst;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;


void main() 
{
    outWorldPos = vec3(pushConst.modelMatrix * vec4(inPosition, 1.0));
    gl_Position = ubo.proj * ubo.view * vec4(outWorldPos, 1.0);

    vec3 normal = inNormal;
    vec3 normalW = normalize(vec3(pushConst.inverseTransposeModel * vec4(normalize(normal), 0)));
    outNormal = normalW;
	
	outColor = vec4(inColor, 1);

    outTexCoord = inTexCoord;

    vec3 tangentW = normalize(vec3(pushConst.modelMatrix * vec4(normalize(inTangent.xyz), 0)));
    //tangentW = normalize(tangentW - dot(tangentW, normalW) * normalW);
    vec3 bitangentW = cross(normalW, tangentW) * inTangent.w;

    fragTBN = mat3(tangentW, normalize(bitangentW), normalW);

}