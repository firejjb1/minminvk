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

	struct CommandList
	{
		u32 commandListID = 0;
		bool isSecondary = false;
		u32 imageIndex = 0;
	};

	struct Device
	{
		Vector<CommandList> commandLists;

		CommandList& GetCommandList(u32 index) { return commandLists[index]; }
		
		void Init();
		bool BeginRecording(RenderContext&);

		void EndRecording(RenderContext&);
		void CleanUp();
	};


}