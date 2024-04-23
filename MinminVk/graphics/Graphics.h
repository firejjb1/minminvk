#pragma once

#include <util/Type.h>
#include <graphics/Device.h>
#include <graphics/Import.h>

#define TRIANGLE_VERTEX_SHADER "trianglevert.spv"
#define TRIANGLE_FRAG_SHADER "trianglefrag.spv"
#define PARTICLE_COMP_SHADER "particles.spv"
#define PARTICLE_COMP_LSC_SHADER "particlelsc.spv"
#define PARTICLE_COMP_ELC_WIND_SHADER "particleelcwind.spv"
#define PARTICLE_VERT_SHADER "particlesvert.spv"
#define PARTICLE_FRAG_SHADER "particlesfrag.spv"
#define STATUE_IMAGE "statue.jpg"
#define WALL_IMAGE "blue_floor_tiles_01_diff_1k.jpg"
#define BLUE_IMAGE "blue.jpeg"
#define VIKING_IMAGE "viking_room.png"
#define VIKING_MODEL "viking_room.obj"
#define HAIR_DATA_FILE "hairdata.txt"

namespace Graphics
{
	struct ParticlesUniformBuffer : UniformBuffer
	{
		struct Uniform
		{
			float deltaTime;
			u32 numVertexPerStrand;
			u32 frame;
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

	SharedPtr<Device> device;
	RenderContext renderContext;
	ComputeContext computeContext;
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
	SharedPtr<ComputePipeline> particleLSCComputePipeline;
	SharedPtr<ComputePipeline> particleELCWindComputePipeline;
	SharedPtr<GraphicsPipeline> particleRenderPipeline;
	Vector<SharedPtr<Buffer>> computeBuffers;
	Vector<Texture> computeTextures{};
	u32 numVertexPerStrand = 100;

	//  4f position 4f color. can optimize later
	// careful about alignment
	Vector<f32> particles{
		//-0.4f, -0.1f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f,
		//0.5f, 0.2f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
		//0.3f, 0.3f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
		//0.5f, 0.5f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
	};

	void InitGraphics(void * window)
	{
		presentation = MakeShared<Presentation>();
		device = MakeShared<Device>();

		presentation->Init(window);
		device->Init();
		presentation->InitSwapChain();

		renderContext.device = device;
		renderContext.presentation = presentation;

		computeContext.device = device;
		
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

			Import::LoadHairStrands(particles, concat_str(ASSET_DIR, HAIR_DATA_FILE));

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

			ResourceBinding particleInitialBinding;
			particleInitialBinding.binding = 3;
			particleInitialBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;

			particleBuffer = MakeShared<StructuredBuffer>(particles, particleWriteBinding, particleBufferUsage);
			auto particleInitialBuffer = MakeShared<StructuredBuffer>(particles, particleInitialBinding, particleBufferUsage);
			Vector<u32> extendedBufferIDs;
			auto numParticleBuffers = particleBuffer->extendedBufferIDs.size();
			for (int i = 0; i < numParticleBuffers; i++)
				extendedBufferIDs.push_back(particleBuffer->extendedBufferIDs[(i - 1) % numParticleBuffers]);
			particleBufferPrev = MakeShared<StructuredBuffer>(particles, extendedBufferIDs, particleBufferBinding, particleBufferUsage);
			particleUniformBuffer = MakeShared<ParticlesUniformBuffer>();
			computeBuffers.insert(computeBuffers.end(), { particleUniformBuffer, particleBufferPrev, particleBuffer, particleInitialBuffer });
			// Integrate and Global Shape
			particleComputePipeline = MakeShared<ComputePipeline>(MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_COMP_SHADER), Shader::ShaderType::SHADER_COMPUTE, "main"),
				vec3{particles.size() * sizeof(f32) / sizeof(ParticleVertex::Particle),1,1}, vec3{256,1,1}, computeBuffers, computeTextures);

			particleLSCComputePipeline = MakeShared<ComputePipeline>(MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_COMP_LSC_SHADER), Shader::ShaderType::SHADER_COMPUTE, "main"),
				vec3{ particles.size() * sizeof(f32) / sizeof(ParticleVertex::Particle) / numVertexPerStrand,1,1 }, vec3{ 256,1,1 }, computeBuffers, computeTextures);

			particleELCWindComputePipeline = MakeShared<ComputePipeline>(MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_COMP_ELC_WIND_SHADER), Shader::ShaderType::SHADER_COMPUTE, "main"),
				vec3{ particles.size() * sizeof(f32) / sizeof(ParticleVertex::Particle),1,1 }, vec3{ 256,1,1 }, computeBuffers, computeTextures);

			auto vertShader = MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_VERT_SHADER), Shader::ShaderType::SHADER_VERTEX, "main");
			auto fragShader = MakeShared<Shader>(concat_str(SHADERS_DIR, PARTICLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main");
			particleRenderPipeline = MakeShared<GraphicsPipeline>(vertShader, fragShader, MakeShared<ParticleVertex>(), MakeShared<BasicUniformBuffer>(), Vector<Texture>{},
				Vector<SharedPtr<Buffer>>{});

			particleRenderPipeline->topologyType = Graphics::GraphicsPipeline::TopologyType::TOPO_LINE_STRIP;
			particleRenderPipeline->blendEnabled = true;
			particleRenderPipeline->depthTestEnable = false;
			particleRenderPipeline->depthWriteEnable = false;
			forwardParticlePass = MakeShared<RenderPass>(particleRenderPipeline, presentation, Graphics::RenderPass::AttachmentOpType::DONTCARE);
			
			// sync
			particleRenderPipeline->Wait(particleELCWindComputePipeline->pipelineID);

			// TODO need to implement inter graphics sync
			//particleRenderPipeline->Wait(forwardPipeline->pipelineID);
		}
	}

	void MainRender(const u32 frameID, const f32 deltaTime)
	{
		renderContext.frameID = frameID;
		computeContext.frameID = frameID;

		// particle compute passes
		{
			particleUniformBuffer->uniform.deltaTime = deltaTime;
			particleUniformBuffer->uniform.numVertexPerStrand = numVertexPerStrand;
			particleUniformBuffer->uniform.frame = frameID;
			particleUniformBuffer->UpdateUniformBuffer(frameID);
			
			computeContext.computePipeline = particleComputePipeline;
			device->BeginRecording(computeContext);
			particleComputePipeline->Dispatch(computeContext);
			computeContext.computePipeline = particleLSCComputePipeline;
			particleLSCComputePipeline->Dispatch(computeContext);
			computeContext.computePipeline = particleELCWindComputePipeline;
			particleELCWindComputePipeline->Dispatch(computeContext);

			device->EndRecording(computeContext);
		}

		// forward passes
		{
			// pass 1
			renderContext.renderPass = forwardPass;
			bool success = device->BeginRecording(renderContext);
			if (!success)
				return;
			
			device->BeginRenderPass(renderContext);
			{
				// view projection
				{
					forwardPipeline->uniformDesc->transformUniform.view = Math::LookAt(vec3(2.0f, 2.0f, 2.f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
					i32 width = presentation->swapChainDetails.width;
					i32 height = presentation->swapChainDetails.height;
					forwardPipeline->uniformDesc->transformUniform.proj = Math::Perspective(glm::radians(45.0f), width, height, 0.01f, 10.0f);
					forwardPipeline->uniformDesc->transformUniform.proj[1][1] *= -1;

					//particleRenderPipeline->uniformDesc->transformUniform.proj = forwardPipeline->uniformDesc->transformUniform.proj;
					//particleRenderPipeline->uniformDesc->transformUniform.view = forwardPipeline->uniformDesc->transformUniform.view;
					particleRenderPipeline->uniformDesc->transformUniform.proj = mat4(1);
					particleRenderPipeline->uniformDesc->transformUniform.view = mat4(1);
				}

				vikingRoom->Update(deltaTime);
				vikingRoom->Draw(renderContext);
			}
			device->EndRenderPass(renderContext);

			// pass 2
			renderContext.renderPass = forwardParticlePass;
			device->BeginRenderPass(renderContext);

			particleBuffer->DrawBuffer(renderContext, particleBuffer->GetBufferSize() / sizeof(ParticleVertex::Particle));

			device->EndRenderPass(renderContext);

			device->EndRecording(renderContext);

		}
	}

	void CleanUp()
	{
		presentation->CleanUp();
		device->CleanUp();
	}
};
