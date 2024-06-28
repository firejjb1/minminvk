#include <graphics/Import.h>
#include <graphics/Geometry.h>
#include <graphics/Texture.h>
#include <graphics/Animation.h>

#include <util/IO.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>
#include <deque>

namespace Graphics
{

	void Import::LoadOBJ(Graphics::BasicVertex& vertices, Vector<u16>& indices, const String& filename)
	{
		tinyobj::attrib_t attrib;
		Vector<tinyobj::shape_t> shapes;
		Vector<tinyobj::material_t> materials;
		String warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Graphics::BasicVertex::Vertex, u32> uniqueVertices{};
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices) {
				Graphics::BasicVertex::Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				if (!attrib.texcoords.empty())
				{
					vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
									};
				}

				vertex.color = { 1.0f, 1.0f, 1.0f };
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.vertices.size());
					vertices.vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	 
	void Import::LoadHairStrands(Vector<f32>& vertices, const String& filename)
	{
		Util::IO::ReadFloats(vertices, filename);
	}

	void Import::LoadGLTFMesh(const String filename, tinygltf::Mesh& mesh, tinygltf::Model& model, Graphics::BasicVertex& vertices, Vector<u16>& indices, Texture& mainTexture)
	{
		tinygltf::Accessor positionAccessor;
		tinygltf::Accessor normalAccessor;
		tinygltf::Accessor uvAccessor;
		auto indicesAccessor = model.accessors[mesh.primitives[0].indices];

		if (model.materials[mesh.primitives[0].material].pbrMetallicRoughness.baseColorTexture.index > -1)
		{
			String mainTextureURI = model.images[model.textures[model.materials[mesh.primitives[0].material].pbrMetallicRoughness.baseColorTexture.index].source].uri;
			if (mainTextureURI != "")
			{
				String texturePath;
				texturePath.append(filename);
				texturePath.append("/../");
				texturePath.append(mainTextureURI);
				Texture texture(texturePath);
				mainTexture = texture;
			}
		}
		for (auto& attrib : mesh.primitives[0].attributes)
		{
			if (attrib.first.compare("POSITION") == 0)
			{
				positionAccessor = model.accessors[attrib.second];
				assert(positionAccessor.componentType == 5126);
			}

			if (attrib.first.compare("NORMAL") == 0)
			{
				normalAccessor = model.accessors[attrib.second];
				assert(normalAccessor.componentType == 5126);
			}
			if (attrib.first.compare("TEXCOORD_0") == 0)
			{
				uvAccessor = model.accessors[attrib.second];
				assert(uvAccessor.componentType == 5126);
			}
		}
		// only support f32 types for now

		assert(indicesAccessor.componentType == 5123);

		vertices.vertices.resize(positionAccessor.count);

		auto positionBufferView = model.bufferViews[positionAccessor.bufferView];
		DebugPrint("Positions\n");

		assert(positionAccessor.type == 3);
		u32 startOfPositionBuffer = positionAccessor.byteOffset + positionBufferView.byteOffset;
		u32 stridePositionBuffer = positionBufferView.byteStride == 0 ? sizeof(f32) * 3 : positionBufferView.byteStride;
		for (int i = 0; i < positionAccessor.count; ++i)
		{
			u32 index = (startOfPositionBuffer + stridePositionBuffer * i);
			vertices.vertices[i].pos.x = static_cast<f32>(((f32*)model.buffers[positionBufferView.buffer].data.data())[index / sizeof(f32)]);
			vertices.vertices[i].pos.y = static_cast<f32>(((f32*)model.buffers[positionBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]);
			vertices.vertices[i].pos.z = static_cast<f32>(((f32*)model.buffers[positionBufferView.buffer].data.data())[(index + sizeof(f32) * 2) / sizeof(f32)]);

		}

		// no normals yet
		//auto normalBufferView = model.bufferViews[normalAccessor.bufferView];
		//DebugPrint("Normals\n");
		//for (int i = normalBufferView.byteOffset / sizeof(f32); i < normalBufferView.byteOffset / sizeof(f32) + normalBufferView.byteLength / sizeof(f32); ++i)
		//{
		//	f32 val = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[i]);
		//	DebugPrint("%f ", val);
		//	if ((i + 1) % 3 == 0)
		//		DebugPrint("\n");
		//}

		if (uvAccessor.bufferView != -1)
		{
			auto uvBufferView = model.bufferViews[uvAccessor.bufferView];
			DebugPrint("UV\n");

			assert(uvAccessor.type == 2);
			u32 startOfUVBuffer = uvAccessor.byteOffset + uvBufferView.byteOffset;
			u32 strideUVBuffer = uvBufferView.byteStride == 0 ? sizeof(f32) * 2 : uvBufferView.byteStride;

			for (int i = 0; i < uvAccessor.count; ++i)
			{
				u32 index = (startOfUVBuffer + strideUVBuffer * i);
				vertices.vertices[i].texCoord.x = static_cast<f32>(((f32*)model.buffers[uvBufferView.buffer].data.data())[index / sizeof(f32)]);
				vertices.vertices[i].texCoord.y = static_cast<f32>(((f32*)model.buffers[uvBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]);
				// convert from -1,1 to 0,1
				vertices.vertices[i].texCoord = vertices.vertices[i].texCoord * 0.5f + vec2(0.5f);

			}
		}


		DebugPrint("Indices\n");
		auto indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
		u32 startOfIndicesBuffer = indicesAccessor.byteOffset + indicesBufferView.byteOffset;
		u32 strideIndicesBuffer = indicesBufferView.byteStride == 0 ? sizeof(u16) : indicesBufferView.byteStride;

		for (int i = 0; i < indicesAccessor.count; ++i)
		{
			u32 index = (startOfIndicesBuffer + strideIndicesBuffer * i);
			u16 val = static_cast<u16>(((u16*)model.buffers[indicesBufferView.buffer].data.data())[index / sizeof(u16)]);
			indices.push_back(val);
		}
	}

	void Import::LoadGLTF(const String& filename, NodeManager& nodeManager, int descriptorPoolID, SharedPtr<BasicUniformBuffer> basicUniform, Vector<SharedPtr<GLTFMesh>>& newMeshes)
	{
		tinygltf::Model model;
		Util::IO::ReadGLTF(model, filename);

		for (auto& scene : model.scenes)
		{
			Vector<Vector<SharedPtr<Animation>>> animationToNodes;
			animationToNodes.resize(model.nodes.size());

			std::deque<std::pair<u32, u32>> nodesStack; // node id (gltf ID) and parent id (engine ID). kinda confusing but need to track both
			for (auto& node : scene.nodes)
			{
				nodesStack.push_back(std::make_pair<u32, u32>(node, 0));
			}

			// create animations and add them to nodes
			for (auto& animation : model.animations)
			{
				for (u32 i = 0; i < animation.channels.size(); ++i)
				{
					auto& channel = animation.channels[i];
					auto& sampler = animation.samplers[i];
					Animation::AnimationType animationType = channel.target_path == "rotation" ? Animation::AnimationType::ROTATION : channel.target_path == "translation" ? Animation::AnimationType::TRANSLATION : Animation::AnimationType::SCALE;
					Animation::SamplerType samplerType = sampler.interpolation == "LINEAR" ? Animation::SamplerType::LINEAR : sampler.interpolation == "STEP" ? Animation::SamplerType::STEP : Animation::SamplerType::CUBIC;
					auto& inputAccessor = model.accessors[sampler.input];
					auto& inputBufferView = model.bufferViews[inputAccessor.bufferView];
					Vector<f32> inputVector;
					u32 startOfInputBuffer = inputAccessor.byteOffset + inputBufferView.byteOffset;
					u32 strideOfInputBuffer = inputBufferView.byteStride == 0 ? sizeof(f32) : inputBufferView.byteStride;

					for (int j = 0; j < inputAccessor.count; ++j)
					{
						u32 index = (startOfInputBuffer + strideOfInputBuffer * j);
						f32 val = static_cast<f32>(((f32*)model.buffers[inputBufferView.buffer].data.data())[index / sizeof(f32)]);
						inputVector.push_back(val);
					}

					Vector<vec3> vec3Output;
					Vector<vec4> vec4Output;

					auto& outputAccessor = model.accessors[sampler.output];
					auto& outputBufferView = model.bufferViews[outputAccessor.bufferView];
					if (outputAccessor.type == 3)
					{
						// vec3 output
						u32 startOfOutputBuffer = outputAccessor.byteOffset + outputBufferView.byteOffset;
						u32 strideOfOutputBuffer = outputBufferView.byteStride == 0 ? sizeof(f32) * 3 : outputBufferView.byteStride;
						for (int j = 0; j < outputAccessor.count; ++j)
						{
							u32 index = (startOfOutputBuffer + strideOfOutputBuffer * j);
							vec3 val;
							val.x = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[index / sizeof(f32)]);
							val.y = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]);
							val.z = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[(index + sizeof(f32) * 2) / sizeof(f32)]);
							vec3Output.push_back(val);
						}
					}
					if (outputAccessor.type == 4)
					{
						// vec4 output
						u32 startOfOutputBuffer = outputAccessor.byteOffset + outputBufferView.byteOffset;
						u32 strideOfOutputBuffer = outputBufferView.byteStride == 0 ? sizeof(f32) * 4 : outputBufferView.byteStride;
						for (int j = 0; j < outputAccessor.count; ++j)
						{
							u32 index = (startOfOutputBuffer + strideOfOutputBuffer * j);
							vec4 val;

							val.x = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[index / sizeof(f32)]);
							val.y = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]);
							val.z = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[(index + sizeof(f32) * 2) / sizeof(f32)]);
							val.w = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[(index + sizeof(f32) * 3) / sizeof(f32)]);
							vec4Output.push_back(val);
						}
					}
					auto newAnimation = MakeShared<Animation>(animationType, samplerType, inputVector, vec3Output, vec4Output);
					animationToNodes[channel.target_node].push_back(newAnimation);
				}
			}

			while (nodesStack.size() > 0)
			{
				auto nodeFront = nodesStack.front();
				auto& node = model.nodes[nodeFront.first];
				auto& parent = nodeFront.second;
				nodesStack.pop_front();
				auto matrix = node.matrix;
				mat4 modelMatrix = mat4(1);
				if (matrix.size() > 0)
				{
					modelMatrix = mat4(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);
				}

				NodeID parentID;
				parentID.id = parent;

				Node::NodeType nodeType = Node::NodeType::EMPTY_NODE;
				if (node.camera >= 0)
					nodeType = Node::NodeType::CAMERA_NODE;
				else if (node.mesh >= 0)
					nodeType = Node::NodeType::MESH_NODE;
				else if (node.skin >= 0)
					nodeType = Node::NodeType::BONE_NODE;

				auto newNode = nodeManager.AddNode(modelMatrix, parentID, nodeType);

				if (nodeType == Node::MESH_NODE)
				{
					auto& gltfMesh = model.meshes[node.mesh];
					auto geometry = MakeShared<GLTFMesh>(descriptorPoolID, basicUniform, filename, gltfMesh, model);
					geometry->node = newNode;
					newMeshes.push_back(geometry);
				}

				for (auto& child : node.children)
				{
					nodesStack.push_back(std::make_pair<u32, u32>(child, newNode->nodeID.id));
				}

				newNode->animations = animationToNodes[nodeFront.first];

			}
		}
		// reading first mesh only
		//auto& mesh = model.meshes[0];

		//LoadGLTFMesh(mesh, model, vertices, indices, mainTextureURI);
		

	}

}
