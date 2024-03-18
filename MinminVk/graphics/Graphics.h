#pragma once

#include <util/Type.h>
#include <graphics/Device.h>

#define TRIANGLE_VERTEX_SHADER "trianglevert.spv"
#define TRIANGLE_FRAG_SHADER "trianglefrag.spv"
#define PARTICLE_COMP_SHADER "particles.spv"
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
	SharedPtr<StructuredBuffer> particleBuffer;
	SharedPtr<StructuredBuffer> particleBufferPrev;
	SharedPtr<ComputePipeline> computePipeline;
	
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
		texture.binding.binding = 1;
		texture.binding.shaderStageType = ResourceBinding::ShaderStageType::FRAGMENT;
		
		// forward pass
		{
			forwardPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				MakeShared<BasicUniformBuffer>(),
				Vector<Texture>{texture},
				Vector<SharedPtr<Buffer>>{}
			);

			forwardPass = MakeShared<RenderPass>(forwardPipeline, presentation);

			quad = MakeShared<Quad>(forwardPipeline->descriptorPoolID.id, forwardPipeline->uniformDesc, texture);

			vikingRoom = MakeShared<OBJMesh>(forwardPipeline->descriptorPoolID.id, forwardPipeline->uniformDesc, texture, concat_str(OBJ_DIR, VIKING_MODEL));

		}

		// Compute Pass
		{
			struct ParticlesUniformBuffer : Buffer
			{
				struct Uniform
				{
					float deltaTime;;
				};

				Uniform uniform;

				const ResourceBinding GetBinding() const override
				{
					ResourceBinding uboBinding;
					uboBinding.binding = 0;
					uboBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;
					return uboBinding;
				}

				const BufferType GetBufferType() const override { return Buffer::BufferType::UNIFORM; }
				const AccessType GetAccessType() const override 
				{
					return AccessType::READONLY;
				}
				const u32 GetBufferSize() const override { return sizeof(uniform); }
				const BufferUsageType GetUsageType() const override {
					return Buffer::BufferUsageType::BUFFER_UNIFORM;
				}

			};
			struct Particle {
				vec2 position;
				vec2 velocity;
				vec4 color;
			};
			// 2f position, 2f velocity, 3f color
			Vector<f32> particles{ 0.5f, 0.5f , 0.f, 0.1f, 1.f, 0.f, 0.f, 1.f};
			ResourceBinding particleBufferBinding;
			particleBufferBinding.binding = 1;
			particleBufferBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;
			Vector<Buffer::BufferUsageType> particleBufferUsage;
			particleBufferUsage.push_back(Buffer::BufferUsageType::BUFFER_VERTEX);
			particleBufferUsage.push_back(Buffer::BufferUsageType::BUFFER_STORAGE);
			particleBufferUsage.push_back(Buffer::BufferUsageType::BUFFER_TRANSFER_DST);
			// same buffer, write descriptor
			ResourceBinding particleWriteBinding;
			particleWriteBinding.binding = 2;
			particleWriteBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;

			particleBufferPrev = MakeShared<StructuredBuffer>(particles, particleBufferBinding, Buffer::AccessType::READONLY, particleBufferUsage);
			particleBuffer = MakeShared<StructuredBuffer>(particles, particleWriteBinding, Buffer::AccessType::READONLY, particleBufferUsage);

			Vector<SharedPtr<Buffer>> computeBuffers {MakeShared<ParticlesUniformBuffer>(), particleBufferPrev, particleBuffer};
			Vector<Texture> computeTextures {};
			computePipeline = MakeShared<ComputePipeline>(MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_COMP_SHADER), Shader::ShaderType::SHADER_COMPUTE, "main"),
				 vec3{8,8,8}, vec3{256,1,1}, computeBuffers, computeTextures);
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
