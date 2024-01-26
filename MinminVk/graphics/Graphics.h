#pragma once

#include <util/Type.h>
#include <graphics/Device.h>

#define TRIANGLE_VERTEX_SHADER "trianglevert.spv"
#define TRIANGLE_FRAG_SHADER "trianglefrag.spv"
#define STATUE_IMAGE "statue.jpg"
#define WALL_IMAGE "blue_floor_tiles_01_diff_1k.jpg"
#define BLUE_IMAGE "blue.jpeg"
#define VIKING_IMAGE "viking_room.png"
#define VIKING_MODEL "viking_room.obj"

namespace Graphics
{
	SharedPtr<Device> device;
	RenderContext context;
	SharedPtr<Presentation> presentation;
	SharedPtr<GraphicsPipeline> forwardPipeline;
	SharedPtr<RenderPass> forwardPass;
	SharedPtr<Quad> quad;
	SharedPtr<OBJMesh> vikingRoom;
	SharedPtr<ParticleStructuredBuffer> particleBuffer;
	
	void InitGraphics(void * window)
	{
		presentation = MakeShared<Presentation>();
		device = MakeShared<Device>();

		presentation->Init(window);
		device->Init();
		presentation->InitSwapChain();

		context.device = device;
		context.presentation = presentation;

		Sampler linearClampSampler;
		Texture texture(concat_str(IMAGES_DIR, VIKING_IMAGE));
		
		// forward pass
		{
			forwardPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				MakeShared<BasicUniformBuffer>()
			);

			forwardPass = MakeShared<RenderPass>(forwardPipeline, presentation);

			// Initialize particles
			Vector<ParticleStructuredBuffer::Particle> particles{};
			auto & particle = particles.emplace_back();
			particle.position = vec2(0.5f, 0.5f);
			particle.velocity = vec2(0.f, 0.1f);
			particle.color = vec4(1.f, 0.f, 0.f, 1.0f);
			particleBuffer = MakeShared<ParticleStructuredBuffer>(particles);
			// particlePass = MakeShared<ComputePass>(particleShader, workGroupSz);
			// particlePass->AddBuffer(particleBuffer, binding, WRITE);
    
			quad = MakeShared<Quad>(forwardPipeline->uniformDesc, texture);

			vikingRoom = MakeShared<OBJMesh>(forwardPipeline->uniformDesc, texture, concat_str(OBJ_DIR, VIKING_MODEL));

		}

	}

	void MainRender(const u32 frameID, const f32 deltaTime)
	{
		context.frameID = frameID;

		// forward pass
		{
			context.renderPass = forwardPass;
			// view projection TODO abstract
			{
				forwardPipeline->uniformDesc->transformUniform.view = Math::LookAt(vec3(2.0f, 2.0f, 2.f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
				i32 width = presentation->swapChainDetails.width;
				i32 height = presentation->swapChainDetails.height;
				forwardPipeline->uniformDesc->transformUniform.proj = Math::Perspective(glm::radians(45.0f), width, height, 0.1f, 10.0f);
				forwardPipeline->uniformDesc->transformUniform.proj[1][1] *= -1;
			}
			bool success = device->BeginRecording(context);
			if (!success)
				return;

			// TODO draw multiple (dynamic uniform buffer, multiple descriptor sets)
			// TODO camera control
			vikingRoom->Update(deltaTime);
			vikingRoom->Draw(context);

			// particlePass->Dispatch()
			// particlePass->Draw()

			device->EndRecording(context);
		}

	}

	void CleanUp()
	{
		presentation->CleanUp();
		device->CleanUp();


	}
};
