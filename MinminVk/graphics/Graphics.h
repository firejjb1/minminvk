#pragma once

#include <util/Type.h>
#include <graphics/Device.h>

#define TRIANGLE_VERTEX_SHADER "trianglevert.spv"
#define TRIANGLE_FRAG_SHADER "trianglefrag.spv"
#define PARTICLE_COMP_SHADER "particles.spv"
#define PARTICLE_VERT_SHADER "particlesvert.spv"
#define PARTICLE_FRAG_SHADER "particlesfrag.spv"
#define STATUE_IMAGE "statue.jpg"
#define WALL_IMAGE "blue_floor_tiles_01_diff_1k.jpg"
#define BLUE_IMAGE "blue.jpeg"
#define VIKING_IMAGE "viking_room.png"
#define VIKING_MODEL "viking_room.obj"

namespace Graphics
{
	struct ParticlesUniformBuffer : UniformBuffer
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

		void* GetData() override
		{
			return (void*)&uniform;
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

		ParticlesUniformBuffer() { Init(); }

	};

	struct Particle {
		vec2 position;
		vec2 velocity;
		vec4 color;
	};


	SharedPtr<Device> device;
	RenderContext context;
	SharedPtr<Presentation> presentation;
	SharedPtr<GraphicsPipeline> forwardPipeline;
	SharedPtr<RenderPass> forwardPass;
	SharedPtr<RenderPass> forwardParticlePass;
	SharedPtr<Quad> quad;
	SharedPtr<OBJMesh> vikingRoom;
	SharedPtr<StructuredBuffer> particleBuffer;
	SharedPtr<StructuredBuffer> particleBufferPrev;
	SharedPtr<ParticlesUniformBuffer> particleUniformBuffer;
	SharedPtr<ComputePipeline> particleComputePipeline;
	SharedPtr<GraphicsPipeline> particleRenderPipeline;
	Vector<SharedPtr<Buffer>> computeBuffers;
	Vector<Texture> computeTextures{};
	
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
			auto uniformBuffer = MakeShared<BasicUniformBuffer>();
			forwardPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				uniformBuffer,
				Vector<Texture>{texture},
				Vector<SharedPtr<Buffer>>{}
			);

			forwardPass = MakeShared<RenderPass>(forwardPipeline, presentation);

			quad = MakeShared<Quad>(forwardPipeline->descriptorPoolID.id, forwardPipeline->uniformDesc, texture);

			vikingRoom = MakeShared<OBJMesh>(forwardPipeline->descriptorPoolID.id, forwardPipeline->uniformDesc, texture, concat_str(OBJ_DIR, VIKING_MODEL));

		}

		// Compute Pass
		{


			// 2f position, 2f velocity, 4f color
			Vector<f32> particles{ 
				0.5f, 0.5f , 0.3f, 0.3f, 1.f, 0.f, 0.f, 1.f, 
				0.5f, 0.5f , -0.3f, -0.3f, 0.f, 1.f, 0.f, 1.f,	
				-0.5f, -0.5f , -0.3f, 0.3f, 1.f, 0.f, 0.f, 1.f, 
				-0.5f, -0.5f , 0.3f, -0.3f, 0.f, 1.f, 0.f, 1.f,	
			};
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

			particleBuffer = MakeShared<StructuredBuffer>(particles, particleWriteBinding, Buffer::AccessType::READONLY, particleBufferUsage);
			Vector<u32> extendedBufferIDs;
			auto numParticleBuffers = particleBuffer->extendedBufferIDs.size();
			for (int i = 0; i < numParticleBuffers; i++)
				extendedBufferIDs.push_back(particleBuffer->extendedBufferIDs[(i - 1) % numParticleBuffers]);
			particleBufferPrev = MakeShared<StructuredBuffer>(particles, extendedBufferIDs, particleBufferBinding, Buffer::AccessType::READONLY, particleBufferUsage);
			particleUniformBuffer = MakeShared<ParticlesUniformBuffer>();
			computeBuffers.insert(computeBuffers.end(), { particleUniformBuffer, particleBufferPrev, particleBuffer });
			particleComputePipeline = MakeShared<ComputePipeline>(MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_COMP_SHADER), Shader::ShaderType::SHADER_COMPUTE, "main"),
				 vec3{particles.size() / sizeof(Particle) / 8,1,1}, vec3{256,1,1}, computeBuffers, computeTextures);

			auto vertShader = MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_VERT_SHADER), Shader::ShaderType::SHADER_VERTEX, "main");
			auto fragShader = MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main");
			particleRenderPipeline = MakeShared<GraphicsPipeline>(vertShader, fragShader, MakeShared<ParticleVertex>(), MakeShared<BasicUniformBuffer>(), Vector<Texture>{},
				Vector<SharedPtr<Buffer>>{});

			particleRenderPipeline->topologyType = Graphics::GraphicsPipeline::TopologyType::TOPO_LINE_LIST;
			
			forwardParticlePass = MakeShared<RenderPass>(particleRenderPipeline, presentation, Graphics::RenderPass::AttachmentOpType::DONTCARE);
			
			// sync
			forwardPipeline->Wait(particleComputePipeline->pipelineID);
			// TODO need to implement inter graphics sync
			//particleRenderPipeline->Wait(forwardPipeline->pipelineID);

			
		}
	}

	void MainRender(const u32 frameID, const f32 deltaTime)
	{
		context.frameID = frameID;

		// particle compute pass
		{
			particleUniformBuffer->uniform.deltaTime = deltaTime;
			particleUniformBuffer->UpdateUniformBuffer(frameID);
			particleComputePipeline->Dispatch(context);

		
		}

		// forward passes
		{
			// pass 1
			context.renderPass = forwardPass;
			bool success = device->BeginRecording(context);
			if (!success)
				return;
			
			device->BeginRenderPass(context);
			{
				// view projection
				{
					forwardPipeline->uniformDesc->transformUniform.view = Math::LookAt(vec3(2.0f, 2.0f, 2.f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
					i32 width = presentation->swapChainDetails.width;
					i32 height = presentation->swapChainDetails.height;
					forwardPipeline->uniformDesc->transformUniform.proj = Math::Perspective(glm::radians(45.0f), width, height, 0.1f, 10.0f);
					forwardPipeline->uniformDesc->transformUniform.proj[1][1] *= -1;
				}

				vikingRoom->Update(deltaTime);
				vikingRoom->Draw(context);
			}
			device->EndRenderPass(context);

			// pass 2
			context.renderPass = forwardParticlePass;
			device->BeginRenderPass(context);

			particleBuffer->DrawBuffer(context, particleBuffer->GetBufferSize() / sizeof(Particle));

			device->EndRenderPass(context);

			// TODO hack, fence and semaphores are per pipeline but should be per command buffer
			context.renderPass = forwardPass;

			device->EndRecording(context);

		}

	}

	void CleanUp()
	{
		presentation->CleanUp();
		device->CleanUp();
	}
};
