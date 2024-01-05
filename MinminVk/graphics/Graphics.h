#pragma once

#include <util/Type.h>
#include <graphics/Device.h>

#define TRIANGLE_VERTEX_SHADER "trianglevert.spv"
#define TRIANGLE_FRAG_SHADER "trianglefrag.spv"

namespace Graphics
{
	SharedPtr<Device> device;
	RenderContext context;
	SharedPtr<Presentation> presentation;
	SharedPtr<GraphicsPipeline> forwardPipeline;
	SharedPtr<RenderPass> forwardPass;
	SharedPtr<Quad> quad;
	
	void InitGraphics(void * window)
	{
		presentation = MakeShared<Presentation>();
		device = MakeShared<Device>();

		presentation->Init(window);
		device->Init();
		presentation->InitSwapChain();

		context.device = device;
		context.presentation = presentation;
		
		// forward pass
		{
			forwardPipeline = MakeShared<GraphicsPipeline>(
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main"),
				MakeShared<BasicVertex>(),
				MakeShared<BasicUniformBuffer>()
			);

			forwardPass = MakeShared<RenderPass>(forwardPipeline);

			quad = MakeShared<Quad>(forwardPipeline->uniformDesc);
		}

	}

	void MainRender(const u32 frameID, const f32 deltaTime)
	{
		context.frameID = frameID;

		// forward pass
		{
			context.renderPass = forwardPass;

			bool success = device->BeginRecording(context);
			if (!success)
				return;

			quad->Update(deltaTime);
			quad->Draw(context);

			device->EndRecording(context);
		}

	}

	void CleanUp()
	{
		presentation->CleanUp();
		device->CleanUp();


	}
};
