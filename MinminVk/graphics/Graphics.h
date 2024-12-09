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
#define GBUFFER_VERTEX_SHADER "gbuffervert.spv"
#define GBUFFER_FRAG_SHADER "gbufferfrag.spv"
#define DEFERRED_VERTEX_SHADER "fsquadvert.spv"
#define DEFERRED_FRAG_SHADER "fsquadfrag.spv"
#define PARTICLE_COMP_SHADER "particles.spv"
#define PARTICLE_COMP_LSC_SHADER "particlelsc.spv"
#define PARTICLE_COMP_ELC_WIND_SHADER "particleelcwind.spv"
#define PARTICLE_VERT_SHADER "particlesvert.spv"
#define PARTICLE_FRAG_SHADER "particlesfrag.spv"
#define VERTEX_COMP_SHADER "computevertex.spv"
#define STATUE_IMAGE "statue.jpg"
#define WALL_IMAGE "blue_floor_tiles_01_diff_1k.jpg"
#define BLUE_IMAGE "blue.jpeg"
#define VIKING_IMAGE "viking_room.png"
#define VIKING_MODEL "viking_room.obj"
#define HEAD_MODEL "head.obj"
#define HAIR_DATA_FILE "hairdata.txt"
#define SKYBOX_RIGHT "skybox/right.jpg"
#define SKYBOX_LEFT "skybox/left.jpg"
#define SKYBOX_TOP "skybox/top.jpg"
#define SKYBOX_BOTTOM "skybox/bottom.jpg"
#define SKYBOX_FRONT "skybox/front.jpg"
#define SKYBOX_BACK "skybox/back.jpg"
#define SKYBOX_VERTEX_SHADER "skyboxvert.spv"
#define SKYBOX_FRAG_SHADER "skyboxfrag.spv"

//#define GLTF_FILE "Cube/Cube.gltf"
//#define GLTF_FILE "BoxAnimated/BoxAnimated.gltf"
//#define GLTF_FILE2 "BoxVertexColors/BoxVertexColors.gltf"
#define GLTF_FILE2 "AnimatedCube/AnimatedCube.gltf"
//#define GLTF_FILE "RiggedSimple/RiggedSimple.gltf"
//#define GLTF_FILE "RiggedFigure/RiggedFigure.gltf"
//#define GLTF_FILE2 "CesiumMan/CesiumMan.gltf"
//#define GLTF_FILE "AnimatedMorphCube/AnimatedMorphCube.gltf"
//#define GLTF_FILE "CesiumMilkTruck/CesiumMilkTruck.gltf"
//#define GLTF_FILE3 "../../../../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf"
//#define GLTF_FILE "../../../../glTF-Sample-Models/2.0/BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf"
//#define GLTF_FILE "../../../../glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf"
//#define GLTF_FILE "../../../../glTF-Sample-Models/2.0/SimpleMorph/glTF/SimpleMorph.gltf"
//#define GLTF_FILE "NormalTangentMirrorTest/NormalTangentMirrorTest.gltf"
//#define GLTF_FILE "building/muranobuilding.gltf"
//#define GLTF_FILE "TextureSettingsTest/TextureSettingsTest.gltf"
//#define GLTF_FILE "AlphaBlendModeTest/AlphaBlendModeTest.gltf"
#define GLTF_FILE "lain2/lain_anim.gltf"
#define GLTF_FILE3 "testBlend/testBlend.gltf"
#define GLTF_ELLEN_JOE "ellen_joe_by_ghost73/scene.gltf"

namespace Graphics
{


	SharedPtr<Device> device;
	SharedPtr<NodeManager> nodeManager;
	RenderContext renderContext;
	ComputeContext computeContext;
	SharedPtr<Presentation> presentation;
	SharedPtr<GraphicsPipeline> forwardPipeline;
#ifdef USE_DEFERRED
	SharedPtr<GraphicsPipeline> deferredPipeline;
#endif
	SharedPtr<RenderPass> forwardPass;
	SharedPtr<GraphicsPipeline> forwardTransparentPipeline;
	SharedPtr<RenderPass> forwardTransparentPass;
	SharedPtr<GraphicsPipeline> skyboxPipeline;
	SharedPtr<RenderPass> skyboxPass;
	SharedPtr<RenderPass> forwardParticlePass;
	SharedPtr<UIRender> uiRender;
	SharedPtr<Quad> quad;
	SharedPtr<Cube> cube;
	SharedPtr<OBJMesh> vikingRoom;
	SharedPtr<OBJMesh> headMesh;
	Vector<SharedPtr<GLTFMesh>> gltfMeshes;
	SharedPtr<StructuredBuffer> particleBuffer;
	SharedPtr<StructuredBuffer> particleBufferPrev;
	SharedPtr<ParticlesUniformBuffer> particleUniformBuffer;
	SharedPtr<ComputePipeline> particleComputePipeline;
	SharedPtr<ComputePipeline> particleLSCComputePipeline;
	SharedPtr<ComputePipeline> particleELCWindComputePipeline;
	SharedPtr<GraphicsPipeline> particleRenderPipeline;
	SharedPtr<ComputePipeline> vertexComputePipeline;
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

		TextureCubemap textureSkybox(concat_str(IMAGES_DIR, SKYBOX_RIGHT),concat_str(IMAGES_DIR, SKYBOX_LEFT),concat_str(IMAGES_DIR, SKYBOX_TOP),concat_str(IMAGES_DIR, SKYBOX_BOTTOM),concat_str(IMAGES_DIR, SKYBOX_FRONT),concat_str(IMAGES_DIR, SKYBOX_BACK));
		textureSkybox.binding.binding = 1;
		texture.binding.shaderStageType = ResourceBinding::ShaderStageType::FRAGMENT;

		i32 width = presentation->swapChainDetails.width;
		i32 height = presentation->swapChainDetails.height;
		camera = MakeShared<Camera>(*nodeManager, UI::cameraPosition, vec3(0.0f, 1.0f, 0.0f), UI::cameraLookDirection, 45, 0.1f, 1000.f, width, height);

		// graphics passes
		{
			auto uniformBuffer = MakeShared<BasicUniformBuffer>();
			
	#ifdef USE_DEFERRED
			forwardPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, GBUFFER_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, GBUFFER_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				uniformBuffer,
				Vector<Texture>{},
				Vector<SharedPtr<Buffer>>{}
			);
			Attachment framebuffer(Texture::FormatType::BGRA_SRGB);
			framebuffer.loadOp = Graphics::AttachmentOpType::DONTCARE;
			framebuffer.depthLoadOp = Graphics::AttachmentOpType::DONTCARE;
			Attachment albedo(Texture::FormatType::RGBA8_UNORM);
			Attachment positionDepth(Texture::FormatType::RGB16_SFLOAT);
			Attachment normal(Texture::FormatType::RGB16_SFLOAT);
			Attachment specular(Texture::FormatType::RGBA8_UNORM);
			albedo.texture.binding.binding = 1;
			positionDepth.texture.binding.binding = 2;
			normal.texture.binding.binding = 3;
			specular.texture.binding.binding = 4;

			deferredPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, DEFERRED_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, DEFERRED_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				uniformBuffer,
				Vector<Texture>{albedo.texture, positionDepth.texture, normal.texture, specular.texture},
				Vector<SharedPtr<Buffer>>{}
			);

			Presentation::PsoAttachmentSwapDependent psoAttachmentsToRebuid;
			psoAttachmentsToRebuid.pso = deferredPipeline;
			psoAttachmentsToRebuid.attachments.push_back(albedo);
			psoAttachmentsToRebuid.attachments.push_back(positionDepth);
			psoAttachmentsToRebuid.attachments.push_back(normal);
			psoAttachmentsToRebuid.attachments.push_back(specular);

			presentation->psoAttachmentSwapchainDependent.push_back(psoAttachmentsToRebuid);

			deferredPipeline->depthTestEnable = true;
			deferredPipeline->depthWriteEnable = false;
			deferredPipeline->blendEnabled = true;

			Vector<Attachment> gbufferAttachments{framebuffer, albedo, positionDepth, normal, specular };

			Vector<RenderPass::SubPass> subpasses{
				RenderPass::SubPass{ .pso = forwardPipeline, .attachments = gbufferAttachments } ,
				RenderPass::SubPass{ .pso = deferredPipeline, .attachments = gbufferAttachments }
			};
			forwardPass = MakeShared<RenderPass>(subpasses);
	#else
			forwardPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				uniformBuffer,
				Vector<Texture>{},
				Vector<SharedPtr<Buffer>>{}
			);
			forwardPass = MakeShared<RenderPass>(forwardPipeline, Graphics::AttachmentOpType::DONTCARE);
	#endif
			forwardTransparentPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				uniformBuffer,
				Vector<Texture>{},
				Vector<SharedPtr<Buffer>>{}
			);
			forwardTransparentPipeline->blendEnabled = true;
			forwardTransparentPipeline->depthTestEnable = true;
			forwardTransparentPipeline->depthWriteEnable = false;
			forwardTransparentPass = MakeShared<RenderPass>(forwardTransparentPipeline, Graphics::AttachmentOpType::DONTCARE);

			skyboxPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, SKYBOX_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, SKYBOX_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<PosOnlyVertex>(),
				uniformBuffer,
				Vector<Texture>{ textureSkybox },
				Vector<SharedPtr<Buffer>>{}

			);
			skyboxPipeline->depthWriteEnable = false;
			skyboxPipeline->depthCompareOp = GraphicsPipeline::DepthCompareOpType::LEQUAL;
			skyboxPass = MakeShared<RenderPass>(skyboxPipeline);

			cube = MakeShared<Cube>(skyboxPipeline, textureSkybox);

#ifdef USE_DEFERRED
			quad = MakeShared<Quad>(deferredPipeline, texture);
#else
			quad = MakeShared<Quad>(forwardPipeline, texture);
#endif // USE_DEFERRED

			// OBJ
			vikingRoom = MakeShared<OBJMesh>(forwardPipeline, texture, concat_str(OBJ_DIR, VIKING_MODEL));
			vikingRoom->node->worldMatrix = Math::Translate(vikingRoom->node->worldMatrix, vec3(0, -2.5f, -5));
			
			headMesh = MakeShared<OBJMesh>(forwardPipeline, concat_str(HAIR_DIR, HEAD_MODEL));
			headMesh->node = nodeManager->AddNode(Math::Translate(Math::Rotate(mat4(1), Math::PI, vec3(0, 0, 1)), vec3(0,1,-2)), camera->node->nodeID, Node::NodeType::MESH_NODE);
			// GLTF
			SharedPtr<Node> gltf1 = Import::LoadGLTF(concat_str(GLTF_DIR, GLTF_FILE), *nodeManager, forwardPipeline, forwardTransparentPipeline, gltfMeshes);
			// TODO: implement a way to manipulate mesh nodes easily
			gltf1->modelMatrix = Math::Translate(mat4(1), vec3(-1, 1.5f, 0));
			SharedPtr<Node> gltf2 = Import::LoadGLTF(concat_str(GLTF_DIR, GLTF_FILE2), *nodeManager, forwardPipeline, forwardTransparentPipeline, gltfMeshes);
			DebugPrint("num gltf meshes: %d\n", gltfMeshes.size());
			gltf2->modelMatrix = Math::Translate(Math::Scale(mat4(1), vec3(0.2f)), vec3(5.f, 0.5f, 0));
			auto gltf3 = Import::LoadGLTF(concat_str(GLTF_DIR, GLTF_FILE3), *nodeManager, forwardPipeline, forwardTransparentPipeline, gltfMeshes);
			gltf3->modelMatrix = Math::Scale(mat4(1), vec3(0.2f));

			auto ellengltf = Import::LoadGLTF(concat_str(GLTF_DIR, GLTF_ELLEN_JOE), *nodeManager, forwardPipeline, forwardTransparentPipeline, gltfMeshes);
			ellengltf->modelMatrix = Math::Translate(Math::Rotate(mat4(1), -Math::PI / 2, vec3(0, 1, 0)), vec3(-1, 0.5f, -5));
		}

		// Compute Passes
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
				vec3{ particles.size() * sizeof(f32) / sizeof(ParticleVertex::Particle),1,1 }, vec3{ 256,1,1 }, computeBuffers, computeTextures);

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
			particleRenderPipeline->lineWidth = 10;
			forwardParticlePass = MakeShared<RenderPass>(particleRenderPipeline, Graphics::AttachmentOpType::DONTCARE);

			for (auto mesh : gltfMeshes)
			{
				// use the first mesh with skeleton or blend shape to init the pipeline
				if (mesh->GetVertexData()->hasSkeleton || mesh->GetVertexData()->hasBlends)
				{
					Vector<SharedPtr<Buffer>> computeVertexBuffers;
					if (gltfMeshes.size() > 0)
					{
						computeVertexBuffers.push_back(gltfMeshes[0]->vertexBuffer);
						computeVertexBuffers.push_back(gltfMeshes[0]->transformedVertexBuffer);
					}
					ResourceBinding jointWeightBufferBinding;
					jointWeightBufferBinding.binding = 2;
					jointWeightBufferBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;
					Vector<Buffer::BufferUsageType> jointWeightsBufferUsage;
					jointWeightsBufferUsage.push_back(Buffer::BufferUsageType::BUFFER_STORAGE);
					jointWeightsBufferUsage.push_back(Buffer::BufferUsageType::BUFFER_TRANSFER_DST);
					Vector<f32> tmpVec{1};
					SharedPtr<StructuredBuffer> jointWeightData = MakeShared<StructuredBuffer>(tmpVec, jointWeightBufferBinding, jointWeightsBufferUsage);
					computeVertexBuffers.push_back(jointWeightData);
					SharedPtr<SkeletonUniformBuffer> skeletonBufferData = MakeShared<SkeletonUniformBuffer>();
					computeVertexBuffers.push_back(skeletonBufferData);
					SharedPtr<BlendWeightsUniformBuffer> blendWeightsUniform = MakeShared<BlendWeightsUniformBuffer>();
					computeVertexBuffers.push_back(blendWeightsUniform);
					ResourceBinding blendDataBufferBinding;
					blendDataBufferBinding.binding = 5;
					blendDataBufferBinding.shaderStageType = ResourceBinding::ShaderStageType::COMPUTE;
					Vector<Buffer::BufferUsageType> blendDataBufferUsage;
					blendDataBufferUsage.push_back(Buffer::BufferUsageType::BUFFER_STORAGE);
					blendDataBufferUsage.push_back(Buffer::BufferUsageType::BUFFER_TRANSFER_DST);
					SharedPtr<StructuredBuffer> blendDataBuffer = MakeShared<StructuredBuffer>(tmpVec, blendDataBufferBinding, blendDataBufferUsage);
					computeVertexBuffers.push_back(blendDataBuffer);
					Vector<Texture> tex{};
					PushConstant vertexConstant("VertexParams", PushConstant::Stage::COMPUTE, sizeof(BasicVertex::ComputeVertexConstant));
					vertexComputePipeline = MakeShared<ComputePipeline>(MakeShared<Shader>(concat_str(SHADERS_DIR, VERTEX_COMP_SHADER), Shader::ShaderType::SHADER_COMPUTE, "main"),
						vec3{ 1,1,1 }, vec3{ 64,1,1 }, computeVertexBuffers, tex, Vector<PushConstant>{vertexConstant}
					);
					break;
				}
			}
			

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

					for (auto& mesh : gltfMeshes)
					{
						auto vertexData = mesh->GetVertexData();
						if (vertexData->hasSkeleton)
						{
							computeContext.computePipeline = vertexComputePipeline;
							mesh->Update(fixedDeltaTime);
							mesh->skeletonMatrixData->UpdateUniformBuffer(computeContext.frameID);
							// set the buffers
							Vector<SharedPtr<Buffer>> newBuffers{mesh->vertexBuffer, mesh->transformedVertexBuffer, mesh->jointWeightData, mesh->skeletonMatrixData };
							Vector<Texture> newTextures;
							// TODO one descriptor set per skinned or morph mesh for compute vertex
							device->BeginRecording(computeContext);
							vertexComputePipeline->UpdateResources(computeContext, newBuffers, newTextures);
							device->EndRecording(computeContext);

							device->BeginRecording(computeContext);
							vertexComputePipeline->pushConstants[0].SetData(&mesh->GetVertexData()->vertexConstant, sizeof(BasicVertex::ComputeVertexConstant));
							vertexComputePipeline->threadSz = vec3(mesh->vertexBuffer->GetBufferSize() / sizeof(BasicVertex::Vertex), 1, 1);
							vertexComputePipeline->Dispatch(computeContext);
							device->EndRecording(computeContext);
						}

						if (vertexData->hasBlends)
						{
							computeContext.computePipeline = vertexComputePipeline;
							mesh->Update(fixedDeltaTime);
							mesh->morphWeightData->UpdateUniformBuffer(computeContext.frameID);
							Vector<SharedPtr<Buffer>> newBuffers{ mesh->vertexBuffer, mesh->transformedVertexBuffer, mesh->morphWeightData, mesh->morphTargetsData };
							Vector<Texture> newTextures;
							device->BeginRecording(computeContext);
							vertexComputePipeline->UpdateResources(computeContext, newBuffers, newTextures);
							vertexComputePipeline->pushConstants[0].SetData(&mesh->GetVertexData()->vertexConstant, sizeof(BasicVertex::ComputeVertexConstant));
							device->EndRecording(computeContext);
							vertexComputePipeline->threadSz = vec3(mesh->vertexBuffer->GetBufferSize() / sizeof(BasicVertex::Vertex), 1, 1);
							device->BeginRecording(computeContext);
							vertexComputePipeline->Dispatch(computeContext);
							device->EndRecording(computeContext);
						}
					}

					
				}

				vikingRoom->Update(fixedDeltaTime);

			}
		}
	}

	void MainRender(const u32 frameID, const f32 deltaTime)
	{
		renderContext.frameID = frameID;
		renderContext.updateFrameID = computeContext.frameID;

		updateTimeAccumulator += deltaTime;
		Update(fixedDeltaTime);

		bool success = device->BeginRecording(renderContext);
		assert(success);
		// pass 0 skybox
		renderContext.renderPass = skyboxPass;
		device->BeginRenderPass(renderContext);
		cube->Draw(renderContext);
		device->EndRenderPass(renderContext);
		// forward passes
		{
			// pass 1 - meshes
			renderContext.renderPass = forwardPass;

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
 			device->BeginRenderPass(renderContext);
 			{
				
 				vikingRoom->Draw(renderContext);
 				for (auto& mesh : gltfMeshes)
 				{
 					// opaque or mask alpha
 					if (mesh->material->material->alphaMode != PBRMaterial::ALPHA_MODE::ALPHA_TRANSPARENT)
 						mesh->Draw(renderContext);
 				}

 				headMesh->Draw(renderContext);

 			}
 			{
 				// full screen quad
 #ifdef USE_DEFERRED
 				renderContext.subPass++;
 				device->BeginSubPass(renderContext);

 				quad->Draw(renderContext);
 #endif
 			}

 			device->EndRenderPass(renderContext);

			 // pass 2 - hair
			 renderContext.renderPass = forwardParticlePass;
			 device->BeginRenderPass(renderContext);

			 // TODO: find better way to attach hair
			 renderContext.renderPass->subpasses[0].pso->uniformDesc->transformUniform.model = headMesh->node->worldMatrix;
			 particleBuffer->DrawBuffer(renderContext, particleBuffer->GetBufferSize() / sizeof(ParticleVertex::Particle));

			 device->EndRenderPass(renderContext);

			 // pass 3 transparent meshes
			 renderContext.renderPass = forwardTransparentPass;
			 device->BeginRenderPass(renderContext);
			 {
			 	for (auto& mesh : gltfMeshes)
			 	{
			 		if (mesh->material->material->alphaMode == PBRMaterial::ALPHA_MODE::ALPHA_TRANSPARENT)
			 			mesh->Draw(renderContext);
			 	}
			 }
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
