#pragma once

#include <util/Type.h>
#include <graphics/Animation.h>

namespace Graphics
{

	struct NodeID
	{
		i32 id = -1;
	};

	struct NodeManager;

	struct Node
	{
		enum NodeType { EMPTY_NODE, MESH_NODE, CAMERA_NODE, BONE_NODE, ROOT_NODE, SKINNED_MESH_NODE };
		NodeType nodeType = EMPTY_NODE;
		
		String name;
		NodeID nodeID;
		NodeID parentNodeID;
		Vector<NodeID> childrenIDs;

		mat4 modelMatrix = mat4(1);
		mat4 parentModelMatrix = mat4(1);
		mat4 worldMatrix = mat4(1);

		Vector<f32> morphWeights;

		bool isDirty = true;

		Vector<SharedPtr<Animation>> animations;
		f32 minAnimationTime = 0.f;
		f32 maxAnimationTime = 5.f;
		f32 timer = 0;

		// only for import
		i32 gltfID = 0;
	
		void Update(f32 deltaTime, NodeManager& nodeManager);
	};

	struct NodeManager
	{
		u32 maxSize;
		u32 cyclicIndex = 1;

		f32 timer = 0;

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

		void AddParent(SharedPtr<Node> newNode, NodeID parentID)
		{
			newNode->parentNodeID = parentID;
			nodes[newNode->parentNodeID.id]->childrenIDs.push_back(newNode->nodeID);
		}

		SharedPtr<Node> AddNode(mat4 modelMatrix, NodeID parentID, Node::NodeType nodeType, String name = "")
		{
			SharedPtr<Node> newNode = MakeShared<Node>();
			newNode->nodeID.id = cyclicIndex;
			newNode->nodeType = nodeType;
			newNode->modelMatrix = modelMatrix;
			newNode->name = name;
			nodes[cyclicIndex] = newNode;
			AddParent(newNode, parentID);
			
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

		void Update(f32 deltaTime)
		{
			timer += deltaTime;
			if (timer > Animation::maxAnimationTime)
				timer = 0;
			for (SharedPtr<Node> node : nodes)
			{
				if (node)
					node->Update(deltaTime, *this);
			}
		}


	};
}