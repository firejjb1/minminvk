//layout(location = 0) out vec2 outUV;

//void main()
//{
 //   outUV = vec2((gl_VertexIndex << 2) & 2, gl_VertexIndex & 2);

//	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
    
//}

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
    mat4 inverseTransposeModel;
} pushConst;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec2 fragTexCoord;

void main() { 
 //   vec3 fragPosWS = vec3(pushConst.modelMatrix * vec4(inPosition, 1.0));
 //   gl_Position = ubo.proj * ubo.view * vec4(fragPosWS, 1.0);
   gl_Position = vec4(inPosition, 1.0);
   fragTexCoord = inTexCoord;
}