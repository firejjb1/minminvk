#pragma once

#include "Shader.h"

namespace Graphics
{
	struct Pipeline
	{
		virtual void Init() = 0;
	};


	struct GraphicsPipeline : public Pipeline
	{
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
			RASTER_POLYGON_MODE_FILL,
			RASTER_FRONT_FACE_CW,
			RASTER_CULL_MODE_BACK
		};

		// optionals
		i32 lineWidth;
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

		// assigned by order they are created, starting with 1
		u32 pipelineId = 0;

		GraphicsPipeline(SharedPtr<Shader> vertexShader, SharedPtr<Shader> fragmentShade)
			: vertexShader{ vertexShader }, fragmentShader { fragmentShade } {}

		void Init() override;
	};
}