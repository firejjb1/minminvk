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

		auto normalBufferView = model.bufferViews[normalAccessor.bufferView];
		DebugPrint("Normals\n");
		u32 startOfNormalBuffer = normalAccessor.byteOffset + normalBufferView.byteOffset;
		u32 strideNormalBuffer = normalBufferView.byteStride == 0 ? sizeof(f32) * 3 : normalBufferView.byteStride;

		for (int i = 0; i < normalAccessor.count; ++i)
		{
			u32 index = (startOfNormalBuffer + strideNormalBuffer * i);
			vertices.vertices[i].normal.x = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[index / sizeof(f32)]);
			vertices.vertices[i].normal.y = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]);
			vertices.vertices[i].normal.z = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[(index + sizeof(f32) * 2) / sizeof(f32)]);
		}

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

	void Import::LoadGLTFSkinnedMesh(const String filename, tinygltf::Mesh & mesh, tinygltf::Model & model, Graphics::SkinnedVertex & vertices, Vector<u16>&indices, Texture & mainTexture)
	{
		tinygltf::Accessor positionAccessor;
		tinygltf::Accessor normalAccessor;
		tinygltf::Accessor uvAccessor;
		tinygltf::Accessor weightsAccessor;
		tinygltf::Accessor jointsAccessor;
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
			if (attrib.first.compare("WEIGHTS_0") == 0)
			{
				weightsAccessor = model.accessors[attrib.second];
				assert(weightsAccessor.componentType == 5126);
			}
			if (attrib.first.compare("JOINTS_0") == 0)
			{
				jointsAccessor = model.accessors[attrib.second];
				assert(jointsAccessor.componentType == 5123);
			}
		}

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

		auto normalBufferView = model.bufferViews[normalAccessor.bufferView];
		DebugPrint("Normals\n");
		u32 startOfNormalBuffer = normalAccessor.byteOffset + normalBufferView.byteOffset;
		u32 strideNormalBuffer = normalBufferView.byteStride == 0 ? sizeof(f32) * 3 : normalBufferView.byteStride;

		for (int i = 0; i < normalAccessor.count; ++i)
		{
			u32 index = (startOfNormalBuffer + strideNormalBuffer * i);
			vertices.vertices[i].normal.x = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[index / sizeof(f32)]);
			vertices.vertices[i].normal.y = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]);
			vertices.vertices[i].normal.z = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[(index + sizeof(f32) * 2) / sizeof(f32)]);
		}

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
				assert(vertices.vertices[i].texCoord.x <= uvAccessor.maxValues[0] && vertices.vertices[i].texCoord.x >= uvAccessor.minValues[0]);
				assert(vertices.vertices[i].texCoord.y <= uvAccessor.maxValues[1] && vertices.vertices[i].texCoord.y >= uvAccessor.minValues[1]);
				// convert from -1,1 to 0,1
				if (uvAccessor.minValues[0] < 0.f)
					vertices.vertices[i].texCoord = vertices.vertices[i].texCoord * 0.5f + vec2(0.5f);

			}
		}

		if (weightsAccessor.bufferView != -1)
		{
			auto weightsBufferView = model.bufferViews[weightsAccessor.bufferView];
			DebugPrint("Weights\n");

			assert(weightsAccessor.type == 4);
			u32 startOfWeightsBuffer = weightsAccessor.byteOffset + weightsBufferView.byteOffset;
			u32 strideWeightsBuffer = weightsBufferView.byteStride == 0 ? sizeof(f32) * 4 : weightsBufferView.byteStride;

			for (int i = 0; i < weightsAccessor.count; ++i)
			{
				u32 index = (startOfWeightsBuffer + strideWeightsBuffer * i);
				vertices.vertices[i].weights.x = static_cast<f32>(((f32*)model.buffers[weightsBufferView.buffer].data.data())[index / sizeof(f32)]);
				vertices.vertices[i].weights.y = static_cast<f32>(((f32*)model.buffers[weightsBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]);
				vertices.vertices[i].weights.z = static_cast<f32>(((f32*)model.buffers[weightsBufferView.buffer].data.data())[(index + sizeof(f32) * 2) / sizeof(f32)]);
				vertices.vertices[i].weights.w = static_cast<f32>(((f32*)model.buffers[weightsBufferView.buffer].data.data())[(index + sizeof(f32) * 3) / sizeof(f32)]);

			}
		}
		
		if (jointsAccessor.bufferView != -1)
		{
			auto jointsBufferView = model.bufferViews[jointsAccessor.bufferView];
			DebugPrint("Joints\n");

			assert(jointsAccessor.type == 4);
			u32 startOfJointsBuffer = jointsAccessor.byteOffset + jointsBufferView.byteOffset;
			u32 strideJointsBuffer = jointsBufferView.byteStride == 0 ? sizeof(u16) * 4 : jointsBufferView.byteStride;

			for (int i = 0; i < jointsAccessor.count; ++i)
			{
				u32 index = (startOfJointsBuffer + strideJointsBuffer * i);
				vertices.vertices[i].joints.x = static_cast<u32>(((u16*)model.buffers[jointsBufferView.buffer].data.data())[index / sizeof(u16)]);
				vertices.vertices[i].joints.y = static_cast<u32>(((u16*)model.buffers[jointsBufferView.buffer].data.data())[(index + sizeof(u16)) / sizeof(u16)]);
				vertices.vertices[i].joints.z = static_cast<u32>(((u16*)model.buffers[jointsBufferView.buffer].data.data())[(index + sizeof(u16) * 2) / sizeof(u16)]);
				vertices.vertices[i].joints.w = static_cast<u32>(((u16*)model.buffers[jointsBufferView.buffer].data.data())[(index + sizeof(u16) * 3) / sizeof(u16)]);

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


	void Import::LoadGLTF(const String& filename, NodeManager& nodeManager, SharedPtr<GraphicsPipeline> forwardPipeline, SharedPtr<GraphicsPipeline> forwardSkinnedPipeline, Vector<SharedPtr<GLTFMesh>>& newMeshes, Vector<SharedPtr<GLTFSkinnedMesh>> &newSkinnedMeshes)
	{
		tinygltf::Model model;
		Util::IO::ReadGLTF(model, filename);
		Vector<std::pair<SharedPtr<GLTFSkinnedMesh>, Vector<int>>> nodeToJoints;
		Vector<SharedPtr<PBRMaterial>> pbrMaterials;

		for (auto& scene : model.scenes)
		{
			Vector<Vector<SharedPtr<Animation>>> animationToNodes;
			animationToNodes.resize(model.nodes.size());

			std::deque<std::pair<u32, u32>> nodesStack; // node id (gltf ID) and parent id (engine ID). kinda confusing but need to track both
			for (auto& node : scene.nodes)
			{
				nodesStack.push_back(std::make_pair<u32, u32>(node, 0));
			}

			for (auto& material : model.materials)
			{
				f32 metallic = material.pbrMetallicRoughness.metallicFactor;
				f32 roughness = material.pbrMetallicRoughness.roughnessFactor;
				auto colorVec = material.pbrMetallicRoughness.baseColorFactor;
				auto emissiveVec = material.emissiveFactor;
	
				vec4 baseColor = vec4(colorVec[0], colorVec[1], colorVec[2], colorVec[3]);
				auto pbr = MakeShared<PBRMaterial>();
				pbr->material->baseColor = baseColor;
				pbr->material->metallic = metallic;
				pbr->material->roughness = roughness;
				pbr->material->emissiveColor = vec4(emissiveVec[0], emissiveVec[1], emissiveVec[2], 0);
				pbr->material->hasAlbedoTex = material.pbrMetallicRoughness.baseColorTexture.index >= 0;
				pbr->material->hasNormalTex = material.normalTexture.index >= 0;
				pbr->material->hasOcclusionTex = material.occlusionTexture.index >= 0;
				pbr->material->hasMetallicRoughnessTex = material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0;
				pbr->material->hasOcclusionTex = material.occlusionTexture.index >= 0;
				pbr->material->hasEmissiveTex = material.emissiveTexture.index >= 0;

				pbrMaterials.push_back(pbr);
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

				mat4 newrot(1);
				mat4 newscale(1);
				mat4 newtrans(1);

				if (node.translation.size() == 3)
				{
					newtrans = Math::Translate(mat4(1), vec3(node.translation[0], node.translation[1], node.translation[2]));
				}
				if (node.rotation.size() == 4)
				{
					quat quatRot(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
					newrot = Math::RotateQuat(quatRot);
				}
				if (node.scale.size() == 3)
				{
					newscale = Math::Scale(mat4(1), vec3(node.scale[0], node.scale[1], node.scale[2]));
				}
				modelMatrix = newtrans * newrot * newscale * modelMatrix;

				NodeID parentID;
				parentID.id = parent;

				Node::NodeType nodeType = Node::NodeType::EMPTY_NODE;
				if (node.camera >= 0)
					nodeType = Node::NodeType::CAMERA_NODE;
				else if (node.skin >= 0)
					nodeType = Node::NodeType::SKINNED_MESH_NODE;
				else if (node.mesh >= 0)
					nodeType = Node::NodeType::MESH_NODE;
			

				auto newNode = nodeManager.AddNode(modelMatrix, parentID, nodeType);

				if (nodeType == Node::MESH_NODE)
				{
					auto& gltfMesh = model.meshes[node.mesh];
					auto geometry = MakeShared<GLTFMesh>(forwardPipeline, filename, gltfMesh, model);
					geometry->node = newNode;
					if (gltfMesh.primitives[0].material >= 0)
						geometry->material = pbrMaterials[gltfMesh.primitives[0].material]; // TODO fix for multiple prims
					newMeshes.push_back(geometry);
				}
				else if (nodeType == Node::SKINNED_MESH_NODE)
				{
					auto& gltfSkinnedMesh = model.meshes[node.mesh];
					auto geometry = MakeShared<GLTFSkinnedMesh>(forwardSkinnedPipeline, filename, gltfSkinnedMesh, model);
					geometry->node = newNode;
					if (gltfSkinnedMesh.primitives[0].material >= 0)
						geometry->material = pbrMaterials[gltfSkinnedMesh.primitives[0].material]; // TODO fix for multiple prims
					newSkinnedMeshes.push_back(geometry);
					// get inverse bind matrices
					auto inverseBindMatAccessor = model.accessors[model.skins[node.skin].inverseBindMatrices];
					auto inverseBindBufferView = model.bufferViews[inverseBindMatAccessor.bufferView];
					Vector<mat4> invBindMatrices;
					u32 startOfInvBindBuffer = inverseBindMatAccessor.byteOffset + inverseBindBufferView.byteOffset;
					u32 strideOfinvBindBuffer = inverseBindBufferView.byteStride == 0 ? sizeof(f32) * 16 : inverseBindBufferView.byteStride;
					for (int j = 0; j < inverseBindMatAccessor.count; ++j)
					{
						u32 index = (startOfInvBindBuffer + strideOfinvBindBuffer * j);
						mat4 matrix(
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[index / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32)) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 2) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 3) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 4) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 5) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 6) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 7) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 8) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 9) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 10) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 11) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 12) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 13) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 14) / sizeof(f32)]),
							static_cast<f32>(((f32*)model.buffers[inverseBindBufferView.buffer].data.data())[(index + sizeof(f32) * 15) / sizeof(f32)])
						);
						invBindMatrices.push_back(matrix);
					}
					geometry->SetInverseBindMatrices(invBindMatrices);

					// prepare joint vectors
					nodeToJoints.push_back(std::make_pair(geometry, model.skins[node.skin].joints));
				}

				for (auto& child : node.children)
				{
					nodesStack.push_back(std::make_pair<u32, u32>(child, newNode->nodeID.id));
				}

				newNode->animations = animationToNodes[nodeFront.first];
				newNode->gltfID = nodeFront.first;
			}
		}

		for (auto res : nodeToJoints)
		{
			SharedPtr<GLTFSkinnedMesh> node = res.first;
			Vector<int> jointIDs = res.second;
			Vector<SharedPtr<Node>> joints;

			for (int jointID : jointIDs)
			{
				for (auto n : nodeManager.nodes)
				{
					if (n && n->gltfID == jointID)
					{
						joints.push_back(n);
					}
				}
			}

			node->SetJoints(joints);
		}
		// reading first mesh only
		auto& mesh = model.meshes[0];

		//LoadGLTFMesh(mesh, model, vertices, indices, mainTextureURI);
		

	}

}
