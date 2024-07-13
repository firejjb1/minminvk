#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texColor;
layout(set = 1, binding = 1) uniform sampler2D texMetallic;
layout(set = 1, binding = 2) uniform sampler2D texNormal;
layout(set = 1, binding = 3) uniform sampler2D texOcclusion;
layout(set = 1, binding = 4) uniform sampler2D texEmissive;

void main() {
    //outColor = vec4(fragTexCoord, 0, 1);
    //outColor = vec4(fragColor, 1);
    //outColor = vec4(fragNormal, 1);
    outColor = vec4(texture(texColor, fragTexCoord).xyz, 1);

}