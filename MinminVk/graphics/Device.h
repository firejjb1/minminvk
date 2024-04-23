#pragma once

#include <util/Type.h>
#include <graphics/Pipeline.h>
#include <graphics/Presentation.h>

namespace Graphics
{
	struct Device;

	struct RenderContext
	{
		u32 frameID = 0;
		SharedPtr<RenderPass> renderPass;
		SharedPtr<Presentation> presentation;
		SharedPtr<Device> device;
	};

	struct ComputeContext
	{
		u32 frameID = 0;
		SharedPtr<Device> device;
		SharedPtr<ComputePipeline> computePipeline;
	};

	struct CommandList
	{
		u32 commandListID = 0;
		bool isSecondary = false;
		u32 imageIndex = 0;
	};

	struct Device
	{
		Vector<CommandList> commandLists;
		Vector<CommandList> computeCommandLists;

		CommandList& GetCommandList(u32 index) { return commandLists[index]; }
		CommandList& GetComputeCommandList(u32 index) { return computeCommandLists[index]; }
		
		void Init();
		bool BeginRecording(RenderContext&);
		void EndRecording(RenderContext&);	
		
		bool BeginRecording(ComputeContext&);
		void EndRecording(ComputeContext&);

		void BeginRenderPass(Graphics::RenderContext& context);
		void EndRenderPass(Graphics::RenderContext& context);

		void CleanUp();
	};


}