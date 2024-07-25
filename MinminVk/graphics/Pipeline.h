#pragma once

#include <graphics/Shader.h>
#include <graphics/Texture.h>
#include <graphics/Buffer.h>
#include <graphics/Geometry.h>

namespace Graphics
{
	struct RenderPass;
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
		// shaders, states, resources
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
			RasterState::RASTER_FRONT_FACE_CW,
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
		enum class DepthCompareOpType { LESS, GREATER };
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

		GraphicsPipeline(SharedPtr<Shader> vertexShader, SharedPtr<Shader> fragmentShader, SharedPtr<VertexDesc> vertexDesc, 
			SharedPtr<BasicUniformBuffer> uniformDesc, Vector<Texture> textures, Vector<SharedPtr<Buffer>> buffers)
			: vertexShader{ vertexShader }, fragmentShader{ fragmentShader }, vertexDesc{ vertexDesc }, uniformDesc{ uniformDesc }, textures {
			textures
		}, buffers{ buffers } {}

		void Init(RenderPassID);
	};

	struct RenderPass
	{
		RenderPassID renderPassID;

		SharedPtr<GraphicsPipeline> pso;
		bool isFrameBufferCreated = false;

		Texture::FormatType formatType = Texture::FormatType::BGRA_SRGB;
		u32 numSamples = 4;
		enum class AttachmentOpType { CLEAR, STORE, DONTCARE };
		AttachmentOpType loadOp = AttachmentOpType::CLEAR;
		AttachmentOpType storeOp = AttachmentOpType::STORE;
		AttachmentOpType stencilLoadOp = AttachmentOpType::DONTCARE;
		AttachmentOpType stencilStoreOp = AttachmentOpType::DONTCARE;

		AttachmentOpType depthLoadOp = AttachmentOpType::CLEAR;
		AttachmentOpType depthStoreOp = AttachmentOpType::STORE;

		Texture::LayoutType initialLayout = Texture::LayoutType::UNDEFINED;
		Texture::LayoutType finalLayout = Texture::LayoutType::COLOR_ATTACHMENT;

		bool writeToDepth = true;
		u32 numSamplesDepth = 4;

		struct ColorAttachment
		{
			TextureID textureID;
		};

		ColorAttachment colorAttachment;

		struct SubPass
		{
			u32 colorAttachmentIndex = 0;
			enum class SubPassType { GRAPHICS };
			SubPassType subPassType = SubPassType::GRAPHICS;

			u32 colorAttachmentCount = 1;
		};
		SubPass subPass;

		bool shouldFramebuffersMatchSwapchain = true;
		struct FrameBuffer
		{
			Vector<TextureID> textureIDs{};
			// 0 value should match swapchain
			u32 width = 0;
			u32 height = 0;
			u32 numLayers = 1;
		};

		Vector<FrameBuffer> frameBuffers;

		RenderPass(SharedPtr<GraphicsPipeline> pso, SharedPtr<Presentation> presentation, AttachmentOpType loadOp = AttachmentOpType::CLEAR, AttachmentOpType storeOp = AttachmentOpType::STORE) : pso{pso}, loadOp{loadOp}, storeOp{storeOp}
			,depthLoadOp{loadOp}, depthStoreOp{storeOp}
		{
			Init(presentation);
			pso->Init(renderPassID);
		}

		void Init(SharedPtr<Presentation> presentation);
	};

	struct ComputePipeline : public Pipeline
	{
		SharedPtr<Shader> computeShader;
		vec3 threadSz;
		vec3 invocationSz;
		int layoutID;

		Vector<SharedPtr<Buffer>> buffers;
		Vector<Texture> textures;

		ComputePipeline(SharedPtr<Shader> computeShader, vec3 threadSz, vec3 invocationSz, Vector<SharedPtr<Buffer>> &buffers, Vector<Texture> &textures)
			: computeShader{computeShader}, threadSz{threadSz}, invocationSz{invocationSz}, buffers{buffers}, textures{textures}
		{
			Init();
		}

		void Init();

		void Dispatch(ComputeContext & context);

		void UpdateResources(Vector<SharedPtr<Buffer>> &buffers, Vector<Texture> &textures);
	};

	// struct ComputePass
	// {
	// 	ComputePassID computePassID;
	// 	SharedPtr<ComputePipeline> pso;

	// };

}