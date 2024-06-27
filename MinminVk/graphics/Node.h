#pragma once

#include <util/Type.h>
#include <graphics/Animation.h>

namespace Graphics
{

	struct NodeID
	{
		i32 id = -1;
	};

	struct Node
	{
		enum NodeType { EMPTY_NODE, MESH_NODE, CAMERA_NODE, BONE_NODE, ROOT_NODE };
		NodeType nodeType = EMPTY_NODE;
		
		NodeID nodeID;
		NodeID parentNodeID;
		Vector<NodeID> childrenIDs;

		mat4 modelMatrix = mat4(1);
		mat4 parentModelMatrix = mat4(1);

		Vector<SharedPtr<Animation>> animations;
	};

	struct NodeManager
	{
		u32 maxSize;
		u32 cyclicIndex = 1;

		Vector<SharedPtr<Node>> nodes;

		NodeManager(u32 poolSize)
		{
			nodes.resize(poolSize);
			maxSize = poolSize;
			auto rootNode = MakeShared<Node>();
			rootNode->nodeID.id = 0;
			rootNode->nodeType = Node::ROOT_NODE;
			nodes[0] = rootNode;
		}

		SharedPtr<Node> AddNode(mat4 modelMatrix, NodeID parentID, Node::NodeType nodeType)
		{
			SharedPtr<Node> newNode = MakeShared<Node>();
			newNode->nodeID.id = cyclicIndex;
			newNode->nodeType = nodeType;
			newNode->parentNodeID = parentID;
			newNode->modelMatrix = modelMatrix;
			nodes[cyclicIndex] = newNode;
			nodes[newNode->parentNodeID.id]->childrenIDs.push_back(newNode->nodeID);
			
			// find next available slot and record it in cyclicIndex
			u32 curCyclicIndex = cyclicIndex++;
			while (cyclicIndex != curCyclicIndex)
			{
				if (nodes[cyclicIndex] == nullptr)
				{
					break;
				}
				cyclicIndex++;
				if (cyclicIndex == curCyclicIndex)
				{
					// out of place in node pool
					throw;
				}
				cyclicIndex = cyclicIndex % maxSize;
			}

			return newNode;

		}

		void RemoveNode(NodeID nodeID)
		{
			nodes[nodeID.id] = nullptr;
		}

		SharedPtr<Node> GetNode(NodeID nodeID)
		{
			return nodes[nodeID.id];
		}


	};
}