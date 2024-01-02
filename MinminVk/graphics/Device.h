#pragma once

#include <util/Type.h>
#include <graphics/Pipeline.h>

namespace Graphics
{
	struct CommandList
	{
		u32 commandListID = 0;
		bool isSecondary = false;
		u32 imageIndex = 0;
	};

	struct Device
	{
		Vector<CommandList> commandLists;

		// TODO
		CommandList GetCommandList(u32 index) { return commandLists[index]; }
		
		void Init();
		CommandList BeginRecording(SharedPtr<RenderPass>, const u32 frameID);

		void EndRecording(CommandList&, const u32 frameID);
		void CleanUp();
	};
}