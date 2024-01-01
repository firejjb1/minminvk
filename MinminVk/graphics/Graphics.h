#pragma once

#include "Type.h"
#include "Device.h"
#include "Presentation.h"
#include "Pipeline.h"

#define TRIANGLE_VERTEX_SHADER "trianglevert.spv"
#define TRIANGLE_FRAG_SHADER "trianglefrag.spv"

namespace Graphics
{
	SharedPtr<Device> device;
	SharedPtr<Presentation> presentation;
	SharedPtr<GraphicsPipeline> forwardPipeline;
	
	void InitGraphics(void * window)
	{
		presentation = MakeShared<Presentation>();
		device = MakeShared<Device>();

		presentation->Init(window);
		device->Init();
		presentation->InitSwapChain();

		// TODO parse glsl file and compile them in Shader class
		forwardPipeline = MakeShared<GraphicsPipeline>(
			MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_VERTEX_SHADER), Shader::ShaderType::SHADER_VERTEX, "main"),
			MakeShared<Shader>(concat_str(SHADERS_DIR, TRIANGLE_FRAG_SHADER), Shader::ShaderType::SHADER_FRAGMENT, "main")
		);
		forwardPipeline->Init();

	}

	void MainRender()
	{

	}

	void CleanUp()
	{
		presentation->CleanUp();
		device->CleanUp();


	}
};
