#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    //outColor = vec4(fragTexCoord, 0, 1);
    //outColor = vec4(fragNormal, 1);
    outColor = vec4(1);
    //outColor = vec4(texture(texSampler, fragTexCoord).xyz, 1);

}