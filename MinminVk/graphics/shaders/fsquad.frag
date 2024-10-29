#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput positionDepthAttachment;
//layout (input_attachment_index = 1, binding = 1) uniform subpassInput normalAttachment;
//layout (input_attachment_index = 2, binding = 2) uniform subpassInput albedoAttachment;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(binding = 3) uniform UniformBufferPass {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 lightDirection;
    vec4 cameraPosition;
    vec4 lightIntensity;
} uboPass;


void main() 
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = subpassLoad(positionDepthAttachment).rgb;
	//vec3 normal = subpassLoad(normalAttachment).rgb;
	//vec4 albedo = subpassLoad(albedoAttachment);

	#define ambient 0.005
	
	// Ambient part
	//vec3 fragcolor  = albedo.rgb * ambient;
	
    //vec3 L = -uboPass.lightDirection.xyz;

    //float NdotL = max(0.0, dot(normalize(normal), normalize(L)));
    //vec3 diffuse = uboPass.lightIntensity.rgb * albedo.rgb * NdotL;

    //fragcolor += diffuse;
	  	   
	//outColor = vec4(fragcolor, 1.0);
	outColor = vec4(fragPos, 1);
}
