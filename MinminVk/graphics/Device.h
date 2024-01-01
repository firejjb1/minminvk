#pragma once

#include <util/Type.h>
#include <graphics/Pipeline.h>

namespace Graphics
{
	struct CommandList
	{
		u32 commandListID = 0;
		u32 swapChainID = 0;
		bool isSecondary = false;
	};

	struct Device
	{
		Vector<CommandList> commandLists;

		// TODO
		CommandList GetCommandList() { return commandLists[0]; }
		
		void Init();
		CommandList BeginRecording(SharedPtr<RenderPass>);
		void CleanUp();
	};
}