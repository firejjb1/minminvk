#pragma once

#include <graphics/Shader.h>
#include <graphics/Texture.h>
#include <graphics/Buffer.h>
#include <graphics/Geometry.h>

namespace Graphics
{
	struct RenderPass;
	struct Attachment;
	struct ComputePass;
	struct Pipeline;
	struct VertexDesc;
	struct BasicUniformBuffer;
	struct Presentation;
	struct RenderContext;
	struct ComputeContext;

	struct RenderPassID { u32 id = 0; RenderPass* pointer; };
	struct ComputePassID { u32 id = 0; ComputePass* pointer; };
	struct PipeLineID { u32 id = 0; Pipeline* pointer; };
	struct DescriptorPoolID { u32 id = 0; };

	struct Pipeline
	{
		// assigned by order they are created, starting with 1
		PipeLineID pipelineID;
		DescriptorPoolID descriptorPoolID;
		Vector<PipeLineID> pipelinesToWait;

		void Wait(PipeLineID pipelineID);
	};

	struct GraphicsPipeline : public Pipeline
	{
		RenderPassID parentRenderPassID;

		// shaders
		SharedPtr<Shader> vertexShader;
		SharedPtr<Shader> fragmentShader;

		// pipeline states 
		enum class StateType 
		{
			STATE_VIEWPORT,
			STATE_SCISSOR,
			STATE_BLEND_CONST
		};

		// states that can be dynamic
		Set<StateType> dynamicStates = {
			StateType::STATE_VIEWPORT,
			StateType::STATE_SCISSOR
		};

		enum class TopologyType
		{
			TOPO_POINT_LIST,
			TOPO_LINE_LIST,
			TOPO_LINE_STRIP,
			TOPO_TRIANGLE_LIST,
			TOPO_TRIANGLE_STRIP,
		};

		TopologyType topologyType = TopologyType::TOPO_TRIANGLE_LIST;

		// x, y, sizex, sizey. size of 0 means matching the swapchain
		vec4u viewport = vec4u(0);
		vec4u scissor = vec4u(0);

		// rasterizer states
		enum class RasterState
		{
			RASTER_DEPTH_CLAMP,
			RASTER_DISCARD,
			RASTER_POLYGON_MODE_FILL,
			RASTER_POLYGON_MODE_LINE,
			RASTER_POLYGON_MODE_POINT,
			RASTER_CULL_MODE_BACK,
			RASTER_CULL_MODE_FRONT,
			RASTER_CULL_MODE_DISABLE,
			RASTER_CULL_MODE_BOTH,
			RASTER_FRONT_FACE_CW,
			RASTER_FRONT_FACE_CCW,
			RASTER_DEPTH_BIAS_ENABLED,
			RASTER_BLEND_ENABLED,
		};

		Set<RasterState> rasterStates =
		{
			RasterState::RASTER_POLYGON_MODE_FILL,
			RasterState::RASTER_FRONT_FACE_CCW,
			RasterState::RASTER_CULL_MODE_BACK
		};

		// optionals
		i32 lineWidth = 1.f;
		i32 depthBiasConstantFactor = 0.f;
		i32 depthBiasClamp = 0.f;
		i32 depthBiasSlopeFactor = 0.f;

		// TODO depth and stencil, multisampling
		enum class BlendFactorType
		{
			BLEND_FACTOR_ONE,
			BLEND_FACTOR_ZERO,
			BLEND_FACTOR_SRC_ALPHA,
			BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		};
		enum class BlendOpType
		{
			BLEND_OP_ADD
		};

		BlendFactorType blendSrcFactorType;
		BlendFactorType blendDestFactorType;
		BlendOpType blendOpType;
		bool blendEnabled = false;

		bool depthTestEnable = true;
		bool depthWriteEnable = true;
		enum class DepthCompareOpType { LESS, GREATER, LEQUAL };
		DepthCompareOpType depthCompareOp = DepthCompareOpType::LESS;

		// not supported
		bool stencilTestEnable = false;

		SharedPtr<VertexDesc> vertexDesc;
		// per pass data
		SharedPtr<BasicUniformBuffer> uniformDesc;
		// per material
		Vector<Texture> textures;
		Vector<SharedPtr<Buffer>> buffers;

		// auto reduced if device doesn't support
		u32 msaaSamples = 8;

		// up to 5 textures for each mesh (base color, metallic, normal, occlusion, emissive)
		u32 numTexPerMesh = 5;

		// up to 100 meshes (to set descriptor sets limit)
		u32 maxNumMeshes = 100;
		u32 currentMeshCount = 0;
		u32 perMeshLayoutID;

		u32 layoutID;
		u32 setID;

		GraphicsPipeline(SharedPtr<Shader> vertexShader, SharedPtr<Shader> fragmentShader, SharedPtr<VertexDesc> vertexDesc, 
			SharedPtr<BasicUniformBuffer> uniformDesc, Vector<Texture> textures, Vector<SharedPtr<Buffer>> buffers)
			: vertexShader{ vertexShader }, fragmentShader{ fragmentShader }, vertexDesc{ vertexDesc }, uniformDesc{ uniformDesc }, textures { textures }, buffers{ buffers } 
		{
		}	

		void Init(RenderPassID, Vector<Attachment>& attachments);

		void UpdateTextures(const Vector<Texture>& textures);
	};

	enum class AttachmentOpType { CLEAR, STORE, DONTCARE };

	struct Attachment
	{
		Texture::FormatType formatType = Texture::FormatType::BGRA_SRGB;
		AttachmentOpType loadOp = AttachmentOpType::CLEAR;
		AttachmentOpType storeOp = AttachmentOpType::STORE;
		AttachmentOpType stencilLoadOp = AttachmentOpType::DONTCARE;
		AttachmentOpType stencilStoreOp = AttachmentOpType::DONTCARE;
		AttachmentOpType depthLoadOp = AttachmentOpType::CLEAR;
		AttachmentOpType depthStoreOp = AttachmentOpType::STORE;
		Texture::LayoutType initialLayout = Texture::LayoutType::INPUT_ATTACHMENT;
		Texture::LayoutType finalLayout = Texture::LayoutType::INPUT_ATTACHMENT;
		Texture::UsageType usageType = Texture::UsageType::INPUT_ATTACHMENT;

		Texture texture;

		Attachment(Texture::FormatType formatType = Texture::FormatType::RGBA8_UNORM, AttachmentOpType loadOp = AttachmentOpType::CLEAR, AttachmentOpType storeOp = AttachmentOpType::STORE, 
			Texture::UsageType usageType = EnumBitwiseOr(Texture::UsageType::INPUT_ATTACHMENT, Texture::UsageType::COLOR_ATTACHMENT));

		void Recreate(u32 width, u32 height);
	};

	struct RenderPass
	{
		RenderPassID renderPassID;

		struct SubPass
		{
			SharedPtr<GraphicsPipeline> pso;
			Vector<Attachment> attachments;
		};

		Vector<SubPass> subpasses;

		bool shouldFramebuffersMatchSwapchain = true;

		RenderPass(SharedPtr<GraphicsPipeline> pso, AttachmentOpType loadOp = AttachmentOpType::CLEAR, AttachmentOpType storeOp = AttachmentOpType::STORE)
		{
			SubPass& mainPass = subpasses.emplace_back(SubPass{ pso });
			Attachment framebuffer(Texture::FormatType::BGRA_SRGB);
			framebuffer.loadOp = loadOp;
			framebuffer.storeOp = storeOp;
			framebuffer.depthLoadOp = loadOp;
			framebuffer.depthStoreOp = storeOp;

			mainPass.attachments.push_back(framebuffer);

			for (auto &subpass : subpasses)
				subpass.pso->Init(renderPassID, subpass.attachments);
		}

		RenderPass(Vector<SubPass> subpasses) : 
			subpasses{ subpasses }
		{
			for (auto& subpass : subpasses)
				subpass.pso->Init(renderPassID, subpass.attachments);
		}

	};

	struct PushConstant
	{
		enum class Stage { VERTEX, FRAGMENT, COMPUTE };
		Stage stage;
		Vector<u8> data;
		String name;

		PushConstant(String name, Stage stage, u32 dataSize)
		{
			data.resize(dataSize);
		}

		void SetData(void* newData, u32 dataSize)
		{
			for (int i = 0; i < dataSize; ++i)
			{
				data[i] = *(((u8*)newData) + i);
			}
		}
	};

	struct ComputePipeline : public Pipeline
	{
		Vector<PushConstant> pushConstants;
		SharedPtr<Shader> computeShader;
		vec3 threadSz;
		vec3 invocationSz;
		int layoutID;

		Vector<SharedPtr<Buffer>> buffers;
		Vector<Texture> textures;

		ComputePipeline(SharedPtr<Shader> computeShader, vec3 threadSz, vec3 invocationSz, Vector<SharedPtr<Buffer>>& buffers, Vector<Texture>& textures, Vector<PushConstant> pushConstants = Vector<PushConstant>{})
			: computeShader{computeShader}, threadSz{threadSz}, invocationSz{invocationSz}, buffers{buffers}, textures{textures}, pushConstants{pushConstants}
		{
			Init();
		}

		void Init();

		void Dispatch(ComputeContext & context);

		void UpdateResources(ComputeContext& context, Vector<SharedPtr<Buffer>> &buffers, Vector<Texture> &textures);
	};

}