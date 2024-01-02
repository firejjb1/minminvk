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
				MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main")
			);

			forwardPass = MakeShared<RenderPass>(forwardPipeline);
		}


		

	}

	void MainRender(const u32 frameID)
	{
		// basic forward pass
		// 0. wait for previous frame and acquire image from swapchain
		// 1. get renderpass and get command buffer
		// 2. set pipeline state command
		context.frameID = frameID;
		context.renderPass = forwardPass;

		bool success = device->BeginRecording(context);
		if (!success)
			return;
		// 3. for each mesh
		//	 3.1 bind resources commands
		//	 3.2 draw command

		device->EndRecording(context);

		// 3.3 send to swapchain


	}

	void CleanUp()
	{
		presentation->CleanUp();
		device->CleanUp();


	}
};
