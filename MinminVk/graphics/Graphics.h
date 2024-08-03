#pragma once

#include <util/Type.h>
#include <graphics/Device.h>
#include <graphics/Import.h>
#include <graphics/UIRender.h>
#include <graphics/Camera.h>
#include <Input.h>
#include <UI.h>

#define TRIANGLE_VERTEX_SHADER "trianglevert.spv"
#define TRIANGLE_FRAG_SHADER "trianglefrag.spv"
#define SKINNED_VERTEX_SHADER "skinnedmeshvert.spv"
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
#define HEAD_MODEL "head.obj"
#define HAIR_DATA_FILE "hairdata.txt"

//#define CUBE_GLTF "Cube/Cube.gltf"
//#define CUBE_GLTF "BoxAnimated/BoxAnimated.gltf"
//#define CUBE_GLTF "BoxVertexColors/BoxVertexColors.gltf"
//#define CUBE_GLTF "AnimatedCube/AnimatedCube.gltf"
//#define CUBE_GLTF "RiggedSimple/RiggedSimple.gltf"
//#define CUBE_GLTF "RiggedFigure/RiggedFigure.gltf"
#define CUBE_GLTF "CesiumMan/CesiumMan.gltf"
//#define CUBE_GLTF "CesiumMilkTruck/CesiumMilkTruck.gltf"
//#define CUBE_GLTF "../../../../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf"
//#define CUBE_GLTF "NormalTangentMirrorTest/NormalTangentMirrorTest.gltf"
//#define CUBE_GLTF "building/muranobuilding.gltf"

namespace Graphics
{
	struct ParticlesUniformBuffer : UniformBuffer
	{
		struct Uniform
		{
			mat4 prevHead = mat4(1);
			mat4 curHead = mat4(1);
			float deltaTime;
			u32 numVertexPerStrand;
			u32 frame;

			f32 windStrength = 3;
			vec4 windDirection = vec4(-1.f, -0.f, 0.f, 0.f);
			f32 shockStrength = 50;
			u32 elcIteration = 10;
			f32 stiffnessLocal = 0.5f;
			f32 stiffnessGlobal = 0.1f;
			f32 effectiveRangeGlobal = 1.f;
			f32 capsuleRadius = 0.11f;
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
	SharedPtr<NodeManager> nodeManager;
	RenderContext renderContext;
	ComputeContext computeContext;
	SharedPtr<Presentation> presentation;
	SharedPtr<GraphicsPipeline> forwardPipeline;
	SharedPtr<RenderPass> forwardPass;
	SharedPtr<RenderPass> forwardParticlePass;
	SharedPtr<GraphicsPipeline> forwardSkinnedPipeline;
	SharedPtr<RenderPass> forwardSkinnedPass;
	SharedPtr<UIRender> uiRender;
	SharedPtr<Quad> quad;
	SharedPtr<OBJMesh> vikingRoom;
	SharedPtr<OBJMesh> headMesh;
	Vector<SharedPtr<GLTFMesh>> gltfMeshes;
	Vector<SharedPtr<GLTFSkinnedMesh>> gltfSkinnedMeshes;
	SharedPtr<StructuredBuffer> particleBuffer;
	SharedPtr<StructuredBuffer> particleBufferPrev;
	SharedPtr<ParticlesUniformBuffer> particleUniformBuffer;
	SharedPtr<ComputePipeline> particleComputePipeline;
	SharedPtr<ComputePipeline> particleLSCComputePipeline;
	SharedPtr<ComputePipeline> particleELCWindComputePipeline;
	SharedPtr<GraphicsPipeline> particleRenderPipeline;
	Vector<SharedPtr<Buffer>> computeBuffers;
	Vector<Texture> computeTextures{};
	u32 numVertexPerStrand = 16;
	SharedPtr<Camera> camera;

	const f32 fixedDeltaTime = 0.016f;
	f32 updateTimeAccumulator = 0.f;
	const u32 maxUpdateStepsPerFrame = 5;
	u32 physicsFrameID = 0;

	//  4f position 4f color. can optimize later
	// careful about alignment
	Vector<f32> particles{
		//-0.4f, -0.1f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f,
		//0.5f, 0.2f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
		//0.3f, 0.3f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
		//0.5f, 0.5f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
	};

	void InitUI() 
	{
		uiRender = MakeShared<UIRender>(presentation);
	}

	void InitGraphics(void * window)
	{
		nodeManager = MakeShared<NodeManager>(10000);

		presentation = MakeShared<Presentation>();
		device = MakeShared<Device>();
		presentation->swapChainDetails.mode = Presentation::SwapChainDetails::ModeType::MAILBOX;

		presentation->Init(window);
		device->Init();
		presentation->InitSwapChain();
	
		renderContext.device = device;
		renderContext.presentation = presentation;

		computeContext.device = device;
		
		Sampler linearClampSampler;
		Texture texture(concat_str(IMAGES_DIR, VIKING_IMAGE));
		texture.binding.binding = 0;
		texture.binding.shaderStageType = ResourceBinding::ShaderStageType::FRAGMENT;
		
		i32 width = presentation->swapChainDetails.width;
		i32 height = presentation->swapChainDetails.height;
		camera = MakeShared<Camera>(*nodeManager, UI::cameraPosition, vec3(0.0f, 1.0f, 0.0f), UI::cameraLookDirection, 45, 0.1f, 1000.f, width, height);

		// forward pass
		{
			auto uniformBuffer = MakeShared<BasicUniformBuffer>();
			forwardPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				uniformBuffer,
				Vector<Texture>{},
				Vector<SharedPtr<Buffer>>{}
			);
	
			forwardPass = MakeShared<RenderPass>(forwardPipeline, presentation);

			forwardSkinnedPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, SKINNED_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<SkinnedVertex>(),
				uniformBuffer,
				Vector<Texture>{},
				Vector<SharedPtr<Buffer>>{}
			);

			forwardSkinnedPass = MakeShared<RenderPass>(forwardSkinnedPipeline, presentation, Graphics::RenderPass::AttachmentOpType::DONTCARE);

			quad = MakeShared<Quad>(forwardPipeline, texture);

			// OBJ
			vikingRoom = MakeShared<OBJMesh>(forwardPipeline, texture, concat_str(OBJ_DIR, VIKING_MODEL));
			vikingRoom->node->worldMatrix = Math::Translate(vikingRoom->node->worldMatrix, vec3(0, -2.5f, -5));
			
			headMesh = MakeShared<OBJMesh>(forwardPipeline, concat_str(HAIR_DIR, HEAD_MODEL));
			headMesh->node = nodeManager->AddNode(Math::Translate(Math::Rotate(mat4(1), Math::PI, vec3(0, 0, 1)), vec3(0,1,-2)), camera->node->nodeID, Node::NodeType::MESH_NODE);
			// GLTF
			Import::LoadGLTF(concat_str(GLTF_DIR, CUBE_GLTF), *nodeManager, forwardPipeline, forwardSkinnedPipeline, gltfMeshes, gltfSkinnedMeshes);

		}

		// Compute Pass
		{

			Import::LoadHairStrands(particles, concat_str(HAIR_DIR, HAIR_DATA_FILE));

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
			particleRenderPipeline->depthTestEnable = true;
			particleRenderPipeline->depthWriteEnable = false;
			forwardParticlePass = MakeShared<RenderPass>(particleRenderPipeline, presentation, Graphics::RenderPass::AttachmentOpType::DONTCARE);
			
			// TODO async compute
			// particleRenderPipeline->Wait(particleELCWindComputePipeline->pipelineID);

		}
	}

	void Update(const f32 fixedDeltaTime)
	{
		u32 curSteps = 0;
		while (updateTimeAccumulator > fixedDeltaTime)
		{
			updateTimeAccumulator -= fixedDeltaTime;
			curSteps++;
			physicsFrameID++;
			computeContext.frameID = physicsFrameID;

			if (curSteps > maxUpdateStepsPerFrame)
				break;

			auto prevHeadMat = headMesh->node->worldMatrix;
			camera->Update(fixedDeltaTime);
			nodeManager->Update(fixedDeltaTime);
			// all the fixed updates
			{
				// particle compute passes
				{
					particleUniformBuffer->uniform.windStrength = UI::windStrength;
					particleUniformBuffer->uniform.windDirection = vec4(UI::windDirection, 0);
					particleUniformBuffer->uniform.shockStrength = UI::shockStrength;
					particleUniformBuffer->uniform.elcIteration = UI::elcIteration;
					particleUniformBuffer->uniform.stiffnessLocal = UI::stiffnessLocal;
					particleUniformBuffer->uniform.stiffnessGlobal = UI::stiffnessGlobal;
					particleUniformBuffer->uniform.effectiveRangeGlobal = UI::effectiveRangeGlobal;
					particleUniformBuffer->uniform.capsuleRadius = UI::capsuleRadius;
					particleUniformBuffer->uniform.prevHead = Math::Inverse(headMesh->node->worldMatrix) * prevHeadMat;

					if (UI::rotateHead)
						headMesh->Update(fixedDeltaTime);
					if (UI::resetHeadPos)
						headMesh->node->worldMatrix = mat4(1);

					particleUniformBuffer->uniform.curHead = Math::Inverse(headMesh->node->worldMatrix) * headMesh->node->worldMatrix;

					particleUniformBuffer->uniform.deltaTime = fixedDeltaTime;
					particleUniformBuffer->uniform.numVertexPerStrand = numVertexPerStrand;
					particleUniformBuffer->uniform.frame = physicsFrameID;
					particleUniformBuffer->UpdateUniformBuffer(physicsFrameID);

					computeContext.computePipeline = particleComputePipeline;
					device->BeginRecording(computeContext);
					particleComputePipeline->Dispatch(computeContext);
					computeContext.computePipeline = particleLSCComputePipeline;
					particleLSCComputePipeline->Dispatch(computeContext);
					computeContext.computePipeline = particleELCWindComputePipeline;
					particleELCWindComputePipeline->Dispatch(computeContext);

					device->EndRecording(computeContext);
				}

				vikingRoom->Update(fixedDeltaTime);



				for (auto& skinnedMesh : gltfSkinnedMeshes)
				{
					skinnedMesh->Update(fixedDeltaTime);
				}

			}
		}
	}

	void MainRender(const u32 frameID, const f32 deltaTime)
	{
		renderContext.frameID = frameID;

		updateTimeAccumulator += deltaTime;
		Update(fixedDeltaTime);
		// forward passes
		{
			// pass 1 - meshes
			renderContext.renderPass = forwardPass;
			bool success = device->BeginRecording(renderContext);
			if (!success)
				return;
			
			device->BeginRenderPass(renderContext);
			{
				// view projection
				{
					forwardPipeline->uniformDesc->transformUniform.view = camera->GetCameraMatrix();
					i32 width = presentation->swapChainDetails.width;
					i32 height = presentation->swapChainDetails.height;
					forwardPipeline->uniformDesc->transformUniform.proj = camera->GetProjectionMatrix();
					forwardPipeline->uniformDesc->transformUniform.proj[1][1] *= -1;
					forwardPipeline->uniformDesc->transformUniform.cameraPosition = vec4(camera->GetPosition(), 1);
					forwardPipeline->uniformDesc->transformUniform.lightDirection = vec4(UI::lightDirection, 0);
					forwardPipeline->uniformDesc->transformUniform.lightIntensity = vec4(UI::lightIntensity, 1);

					particleRenderPipeline->uniformDesc->transformUniform.proj = forwardPipeline->uniformDesc->transformUniform.proj;
					particleRenderPipeline->uniformDesc->transformUniform.view = forwardPipeline->uniformDesc->transformUniform.view;
				}
				
				vikingRoom->Draw(renderContext);
				
				// non-skinned 
				for (auto& mesh : gltfMeshes)
				{
					mesh->Draw(renderContext);
				}

				headMesh->Draw(renderContext);

			}
			device->EndRenderPass(renderContext);

			// pass 2 - skinned meshes
			renderContext.renderPass = forwardSkinnedPass;
			device->BeginRenderPass(renderContext);
			{
				for (auto& mesh : gltfSkinnedMeshes)
				{
					mesh->Draw(renderContext);
				}
			}
			device->EndRenderPass(renderContext);

			// pass 3 - hair
			renderContext.renderPass = forwardParticlePass;
			device->BeginRenderPass(renderContext);

			renderContext.renderPass->pso->uniformDesc->transformUniform.model = headMesh->node->worldMatrix;
			particleBuffer->DrawBuffer(renderContext, particleBuffer->GetBufferSize() / sizeof(ParticleVertex::Particle));

			device->EndRenderPass(renderContext);

			// UI pass
			renderContext.shouldRenderUI = true;

			device->EndRecording(renderContext);
		}
	}

	void CleanUp()
	{
		presentation->CleanUp();
		device->CleanUp();
	}
};
