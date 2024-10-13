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

	Vector<SharedPtr<Sampler>> Import::textureSamplers;

	void Import::LoadHairStrands(Vector<f32>& vertices, const String& filename)
	{
		Util::IO::ReadFloats(vertices, filename);
	}

	void Import::LoadTextures(const String filename, tinygltf::Primitive& mesh, tinygltf::Model& model, Texture& mainTexture, Texture& metallic, Texture& normal, Texture& occlusion, Texture& emissive)
	{
		if (mesh.material < 0)
			return;
		auto modelMat = model.materials[mesh.material];
		String dirName = filename.substr(0, filename.find_last_of("\\/"));
		if (modelMat.pbrMetallicRoughness.baseColorTexture.index > -1)
		{
			auto tex = model.textures[modelMat.pbrMetallicRoughness.baseColorTexture.index];
			String mainTextureURI = model.images[tex.source].uri;
			if (mainTextureURI != "")
			{
				String texturePath;
				texturePath.append(dirName);
				texturePath.append("/");
				texturePath.append(mainTextureURI);
				Texture texture(texturePath);
				if (tex.sampler > -1)
					texture.textureID.samplerID = textureSamplers[tex.sampler]->id;
				mainTexture = texture;
			}
		}
		if (modelMat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
		{
			auto tex = model.textures[modelMat.pbrMetallicRoughness.metallicRoughnessTexture.index];
			String metallicTextureURI = model.images[tex.source].uri;
			if (metallicTextureURI != "")
			{
				String texturePath;
				texturePath.append(dirName);
				texturePath.append("/");
				texturePath.append(metallicTextureURI);
				Texture texture(texturePath, Texture::FormatType::RGBA8_UNORM);
				if (tex.sampler > -1)
					texture.textureID.samplerID = textureSamplers[tex.sampler]->id;
				texture.binding.binding = 1;
				metallic = texture;
			}
		}
		if (modelMat.normalTexture.index > -1)
		{
			auto tex = model.textures[modelMat.normalTexture.index];
			String normalTexURI = model.images[tex.source].uri;
			if (normalTexURI != "")
			{
				String texturePath;
				texturePath.append(dirName);
				texturePath.append("/");
				texturePath.append(normalTexURI);
				Texture texture(texturePath, Texture::FormatType::RGBA8_UNORM);
				if (tex.sampler > -1)
					texture.textureID.samplerID = textureSamplers[tex.sampler]->id;
				texture.binding.binding = 2;
				normal = texture;
			}
		}
		if (modelMat.occlusionTexture.index > -1)
		{
			auto tex = model.textures[modelMat.occlusionTexture.index];
			String occlusionTexURI = model.images[tex.source].uri;
			if (occlusionTexURI != "")
			{
				String texturePath;
				texturePath.append(dirName);
				texturePath.append("/");
				texturePath.append(occlusionTexURI);
				Texture texture(texturePath, Texture::FormatType::RGBA8_UNORM);
				if (tex.sampler > -1)
					texture.textureID.samplerID = textureSamplers[tex.sampler]->id;
				texture.binding.binding = 3;
				occlusion = texture;
			}
		}
		if (modelMat.emissiveTexture.index > -1)
		{
			auto tex = model.textures[modelMat.emissiveTexture.index];
			String emissiveTexture = model.images[tex.source].uri;
			if (emissiveTexture != "")
			{
				String texturePath;
				texturePath.append(dirName);
				texturePath.append("/");
				texturePath.append(emissiveTexture);
				Texture texture(texturePath);
				if (tex.sampler > -1)
					texture.textureID.samplerID = textureSamplers[tex.sampler]->id;
				texture.binding.binding = 4;
				emissive = texture;
			}
		}
	}

	vec2 ReadGLTFFloat2(u32 index, Vector<unsigned char>& data)
	{
		f32* ptr = (f32*)data.data();
		auto sz = sizeof(f32);
		return vec2{ ptr[index / sz], ptr[(index + sz) / sz] };
	}

	vec3 ReadGLTFFloat3(u32 index, Vector<unsigned char>& data)
	{
		f32* ptr = (f32*)data.data();
		auto sz = sizeof(f32);
		return vec3{ ptr[index / sz], ptr[(index + sz) / sz], ptr[(index + sz * 2) / sz] };
	}
	
	vec4 ReadGLTFFloat4(u32 index, Vector<unsigned char>& data)
	{
		f32* ptr = (f32*)data.data();
		auto sz = sizeof(f32);
		return vec4{ ptr[index / sz], ptr[(index + sz) / sz], ptr[(index + sz * 2) / sz], ptr[(index + sz * 3) / sz] };
	}

	vec4u16 ReadGLTFU16x4(u32 index, Vector<unsigned char>& data)
	{
		u16* ptr = (u16*)data.data();
		auto sz = sizeof(u16);
		return vec4u16{ ptr[index / sz], ptr[(index + sz) / sz], ptr[(index + sz * 2) / sz], ptr[(index + sz * 3) / sz] };
	}
	
	vec4u16 ReadGLTFU8x4(u32 index, Vector<unsigned char>& data)
	{
		u8* ptr = (u8*)data.data();
		auto sz = sizeof(u8);
		return vec4u16{ ptr[index / sz], ptr[(index + sz) / sz], ptr[(index + sz * 2) / sz], ptr[(index + sz * 3) / sz] };
	}


	template <typename T>
	void ReadGLTFUVs(size_t uvCount, u32 startOfUVBuffer, u32 strideUVBuffer, T& vertices, tinygltf::Buffer& buffer)
	{
		for (int i = 0; i < uvCount; ++i)
		{
			u32 index = (startOfUVBuffer + strideUVBuffer * i);
			vertices[i].texCoord = ReadGLTFFloat2(index, buffer.data);
		}
	}



	void Import::LoadGLTFMesh(const String filename, tinygltf::Primitive& mesh, tinygltf::Model& model, Graphics::BasicVertex& vertices, Vector<u16>& indices, Texture& mainTexture, Texture& metallic, Texture& normal, Texture& occlusion, Texture& emissive)
	{
		tinygltf::Accessor positionAccessor;
		tinygltf::Accessor normalAccessor;
		tinygltf::Accessor uvAccessor;
		tinygltf::Accessor tangentAccessor;
		tinygltf::Accessor colorAccessor;

		tinygltf::Accessor weightsAccessor;
		tinygltf::Accessor jointsAccessor;

		auto indicesAccessor = model.accessors[mesh.indices];

		LoadTextures(filename, mesh, model, mainTexture, metallic, normal, occlusion, emissive);
		
		for (auto& attrib : mesh.attributes)
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
				vertices.hasNormal = true;
			}
			if (attrib.first.compare("TEXCOORD_0") == 0)
			{
				uvAccessor = model.accessors[attrib.second];
				assert(uvAccessor.componentType == 5126);
			}
			if (attrib.first.compare("TANGENT") == 0)
			{
				tangentAccessor = model.accessors[attrib.second];
				assert(tangentAccessor.componentType == 5126);
				vertices.hasTangent = true;
			}
			if (attrib.first.compare("COLOR_0") == 0)
			{
				colorAccessor = model.accessors[attrib.second];
				assert(colorAccessor.componentType == 5126 || colorAccessor.componentType == 5123);
			}
			if (attrib.first.compare("WEIGHTS_0") == 0)
			{
				weightsAccessor = model.accessors[attrib.second];
				assert(weightsAccessor.componentType == 5126);
				vertices.hasSkeleton = true;
			}
			if (attrib.first.compare("JOINTS_0") == 0)
			{
				jointsAccessor = model.accessors[attrib.second];
				assert(jointsAccessor.componentType == 5123 || jointsAccessor.componentType == 5121);
			}
		}
		if (mesh.targets.size() > 0)
		{
			vertices.hasBlends = true;
			vertices.blendVertices.resize(positionAccessor.count * mesh.targets.size());
		}

		int targetIndex = 0;
		for (auto& target : mesh.targets)
		{
			if (target.find("POSITION") != target.end())
			{
				auto positionTargetAccessor = model.accessors[target["POSITION"]];
				auto positionTargetBufferView = model.bufferViews[positionTargetAccessor.bufferView];

				assert(positionTargetAccessor.type == 3);
				u32 startOfPositionBuffer = positionTargetAccessor.byteOffset + positionTargetBufferView.byteOffset;
				u32 stridePositionBuffer = positionTargetBufferView.byteStride == 0 ? sizeof(f32) * 3 : positionTargetBufferView.byteStride;
				for (int i = 0; i < positionTargetAccessor.count; ++i)
				{
					u32 index = (startOfPositionBuffer + stridePositionBuffer * i);
					vertices.blendVertices[targetIndex * positionTargetAccessor.count + i].position = vec4(ReadGLTFFloat3(index, model.buffers[positionTargetBufferView.buffer].data), 0);
				}
			}
			if (target.find("NORMAL") != target.end())
			{
				auto normalTargetAccessor = model.accessors[target["NORMAL"]];
				auto normalTargetBufferView = model.bufferViews[normalTargetAccessor.bufferView];

				assert(normalTargetAccessor.type == 3);
				u32 startOfNormalBuffer = normalTargetAccessor.byteOffset + normalTargetBufferView.byteOffset;
				u32 strideNormalBuffer = normalTargetBufferView.byteStride == 0 ? sizeof(f32) * 3 : normalTargetBufferView.byteStride;
				for (int i = 0; i < normalTargetAccessor.count; ++i)
				{
					u32 index = (startOfNormalBuffer + strideNormalBuffer * i);
					vertices.blendVertices[targetIndex * normalTargetAccessor.count + i].normal = vec4(ReadGLTFFloat3(index, model.buffers[normalTargetBufferView.buffer].data), 0);
				}
			}
			if (target.find("TANGENT") != target.end())
			{
				auto tangentTargetAccessor = model.accessors[target["TANGENT"]];
				auto tangentTargetBufferView = model.bufferViews[tangentTargetAccessor.bufferView];

				assert(tangentTargetAccessor.type == 3);
				u32 startOfTangentBuffer = tangentTargetAccessor.byteOffset + tangentTargetBufferView.byteOffset;
				u32 strideTangentBuffer = tangentTargetBufferView.byteStride == 0 ? sizeof(f32) * 3 : tangentTargetBufferView.byteStride;
				for (int i = 0; i < tangentTargetAccessor.count; ++i)
				{
					u32 index = (startOfTangentBuffer + strideTangentBuffer * i);
					vertices.blendVertices[targetIndex * tangentTargetAccessor.count + i].tangent = vec4(ReadGLTFFloat3(index, model.buffers[tangentTargetBufferView.buffer].data), 0);
				}
			}

			targetIndex++;
		}

		assert(indicesAccessor.componentType == 5125 || indicesAccessor.componentType == 5123 || indicesAccessor.componentType == 5121);

		vertices.vertices.resize(positionAccessor.count);
		if (vertices.hasSkeleton == true)
			vertices.jointVertices.resize(positionAccessor.count);

		auto positionBufferView = model.bufferViews[positionAccessor.bufferView];
		assert(positionAccessor.type == 3);
		u32 startOfPositionBuffer = positionAccessor.byteOffset + positionBufferView.byteOffset;
		u32 stridePositionBuffer = positionBufferView.byteStride == 0 ? sizeof(f32) * 3 : positionBufferView.byteStride;
		for (int i = 0; i < positionAccessor.count; ++i)
		{
			u32 index = (startOfPositionBuffer + stridePositionBuffer * i);
			vertices.vertices[i].pos = ReadGLTFFloat3(index, model.buffers[positionBufferView.buffer].data);
		}

		if (normalAccessor.bufferView != -1)
		{
			auto normalBufferView = model.bufferViews[normalAccessor.bufferView];
			u32 startOfNormalBuffer = normalAccessor.byteOffset + normalBufferView.byteOffset;
			u32 strideNormalBuffer = normalBufferView.byteStride == 0 ? sizeof(f32) * 3 : normalBufferView.byteStride;

			for (int i = 0; i < normalAccessor.count; ++i)
			{
				u32 index = (startOfNormalBuffer + strideNormalBuffer * i);
				vertices.vertices[i].normal = ReadGLTFFloat3(index, model.buffers[normalBufferView.buffer].data);
			}
		}

		if (uvAccessor.bufferView != -1)
		{
			auto uvBufferView = model.bufferViews[uvAccessor.bufferView];
			assert(uvAccessor.type == 2);
			u32 startOfUVBuffer = uvAccessor.byteOffset + uvBufferView.byteOffset;
			u32 strideUVBuffer = uvBufferView.byteStride == 0 ? sizeof(f32) * 2 : uvBufferView.byteStride;

			ReadGLTFUVs(uvAccessor.count, startOfUVBuffer, strideUVBuffer, vertices.vertices, model.buffers[uvBufferView.buffer]);

		}

		if (tangentAccessor.bufferView != -1)
		{
			auto tangentBufferView = model.bufferViews[tangentAccessor.bufferView];
			assert(tangentAccessor.type == 4);
			u32 startOfTangentBuffer = tangentAccessor.byteOffset + tangentBufferView.byteOffset;
			u32 strideTangentBuffer = tangentBufferView.byteStride == 0 ? sizeof(f32) * 4 : tangentBufferView.byteStride;

			for (int i = 0; i < tangentAccessor.count; ++i)
			{
				u32 index = (startOfTangentBuffer + strideTangentBuffer * i);
				vertices.vertices[i].tangent = ReadGLTFFloat4(index, model.buffers[tangentBufferView.buffer].data);
			}
		}

		if (colorAccessor.bufferView != -1)
		{
			auto colorBufferView = model.bufferViews[colorAccessor.bufferView];
			assert(colorAccessor.type == 3 || colorAccessor.type == 4);
			if (colorAccessor.type == 3)
			{
				u32 startOfColorBuffer = colorAccessor.byteOffset + colorBufferView.byteOffset;
				u32 strideColorBuffer = colorBufferView.byteStride == 0 ? sizeof(f32) * 3 : colorBufferView.byteStride;
				if (colorAccessor.componentType == 5123)
					strideColorBuffer = colorBufferView.byteStride == 0 ? sizeof(u16) * 3 : colorBufferView.byteStride;

				for (int i = 0; i < colorAccessor.count; ++i)
				{
					u32 index = (startOfColorBuffer + strideColorBuffer * i);
					vertices.vertices[i].color = ReadGLTFFloat3(index, model.buffers[colorBufferView.buffer].data);
				}
			}
			if (colorAccessor.type == 4)
			{
				u32 startOfColorBuffer = colorAccessor.byteOffset + colorBufferView.byteOffset;
				u32 strideColorBuffer = colorBufferView.byteStride == 0 ? sizeof(f32) * 4 : colorBufferView.byteStride;

				for (int i = 0; i < colorAccessor.count; ++i)
				{
					u32 index = (startOfColorBuffer + strideColorBuffer * i);
					vertices.vertices[i].color = vec3(ReadGLTFFloat4(index, model.buffers[colorBufferView.buffer].data));
				}
			}
		}

		if (weightsAccessor.bufferView != -1)
		{
			auto weightsBufferView = model.bufferViews[weightsAccessor.bufferView];
			assert(weightsAccessor.type == 4);
			u32 startOfWeightsBuffer = weightsAccessor.byteOffset + weightsBufferView.byteOffset;
			u32 strideWeightsBuffer = weightsBufferView.byteStride == 0 ? sizeof(f32) * 4 : weightsBufferView.byteStride;

			for (int i = 0; i < weightsAccessor.count; ++i)
			{
				u32 index = (startOfWeightsBuffer + strideWeightsBuffer * i);
				vertices.jointVertices[i].weights = ReadGLTFFloat4(index, model.buffers[weightsBufferView.buffer].data);
			}
		}

		if (jointsAccessor.bufferView != -1)
		{
			auto jointsBufferView = model.bufferViews[jointsAccessor.bufferView];
			assert(jointsAccessor.type == 4);
			u32 startOfJointsBuffer = jointsAccessor.byteOffset + jointsBufferView.byteOffset;
			u32 strideJointsBuffer = jointsBufferView.byteStride == 0 ? sizeof(u16) * 4 : jointsBufferView.byteStride;
			if (jointsAccessor.componentType == 5121)
				strideJointsBuffer = jointsBufferView.byteStride == 0 ? sizeof(u8) * 4 : jointsBufferView.byteStride;

			for (int i = 0; i < jointsAccessor.count; ++i)
			{
				u32 index = (startOfJointsBuffer + strideJointsBuffer * i);
				if (jointsAccessor.componentType == 5123)
					vertices.jointVertices[i].joints = ReadGLTFU16x4(index, model.buffers[jointsBufferView.buffer].data);
				else if (jointsAccessor.componentType == 5121)
					vertices.jointVertices[i].joints = ReadGLTFU8x4(index, model.buffers[jointsBufferView.buffer].data);
			}
		}
		auto indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
		u32 startOfIndicesBuffer = indicesAccessor.byteOffset + indicesBufferView.byteOffset;
		u32 strideIndicesBuffer;
		if (indicesAccessor.componentType == 5121)
			strideIndicesBuffer = indicesBufferView.byteStride == 0 ? sizeof(u8) : indicesBufferView.byteStride;
		else if (indicesAccessor.componentType == 5125)
			strideIndicesBuffer = indicesBufferView.byteStride == 0 ? sizeof(u32) : indicesBufferView.byteStride;
		else
			strideIndicesBuffer = indicesBufferView.byteStride == 0 ? sizeof(u16) : indicesBufferView.byteStride;

		for (int i = 0; i < indicesAccessor.count; ++i)
		{
			u32 index = (startOfIndicesBuffer + strideIndicesBuffer * i);
			u16 val;
			if (indicesAccessor.componentType == 5121)
				val = static_cast<u16>(((u8*)model.buffers[indicesBufferView.buffer].data.data())[index / sizeof(u8)]);
			if (indicesAccessor.componentType == 5123)
				val = static_cast<u16>(((u16*)model.buffers[indicesBufferView.buffer].data.data())[index / sizeof(u16)]);
			if (indicesAccessor.componentType == 5125)
				val = static_cast<u32>(((u32*)model.buffers[indicesBufferView.buffer].data.data())[index / sizeof(u32)]);
			indices.push_back(val);
		}
	}

	void Import::LoadGLTF(const String& filename, NodeManager& nodeManager, SharedPtr<GraphicsPipeline> forwardPipeline, SharedPtr<GraphicsPipeline> forwardTransparentPipeline, Vector<SharedPtr<GLTFMesh>>& newMeshes)
	{
		u32 curNumNodes = nodeManager.cyclicIndex;
		tinygltf::Model model;
		Util::IO::ReadGLTF(model, filename);
		Vector<std::pair<SharedPtr<GLTFMesh>, Vector<int>>> nodeToJoints;
		Vector<SharedPtr<PBRMaterial>> pbrMaterials;

		for (auto& scene : model.scenes)
		{
			for (auto& sampler : model.samplers)
			{
				int magfilter = sampler.magFilter; // 9728 NEAREST 9729 LINEAR
				int minfilter = sampler.minFilter; // 9728 NEAREST 9729 LINEAR 9984 NEAREST_MIPMAP_NEAREST 9985 LINEAR_MIPMAP_NEAREST 9986 NEAREST_MIPMAP_LINEAR 9987 LINEAR_MIPMAP_LINEAR
				int wrapS = sampler.wrapS; // 33071 CLAMP_TO_EDGE 33648 MIRRORED_REPEAT 10497 REPEAT
				int wrapT = sampler.wrapT; // 33071 CLAMP_TO_EDGE 33648 MIRRORED_REPEAT 10497 REPEAT

				Sampler::FilterType tmag;
				Sampler::FilterType tmin;
				Sampler::AddressModeType taddrU;
				Sampler::AddressModeType taddrV;
				if (magfilter == 9728)
					tmag = Sampler::FilterType::POINT;
				else if (magfilter == 9729)
					tmag = Sampler::FilterType::LINEAR;
				if (minfilter == 9728)
					tmin = Sampler::FilterType::POINT;
				else
					tmin = Sampler::FilterType::LINEAR;


				if (wrapS == 33071)
					taddrU = Sampler::AddressModeType::CLAMP_TO_EDGE;
				else if (wrapS == 33648)
					taddrU = Sampler::AddressModeType::MIRRORED_REPEAT;
				else if (wrapS == 10497)
					taddrU = Sampler::AddressModeType::REPEAT;
				if (wrapT == 33071)
					taddrV = Sampler::AddressModeType::CLAMP_TO_EDGE;
				else if (wrapT == 33648)
					taddrV = Sampler::AddressModeType::MIRRORED_REPEAT;
				else if (wrapT == 10497)
					taddrV = Sampler::AddressModeType::REPEAT;

				auto newSampler = MakeShared<Sampler>(tmag, tmin, taddrU, taddrV);
				textureSamplers.push_back(newSampler);
			}

			if (textureSamplers.empty())
			{
				// default sampler
				textureSamplers.push_back(MakeShared<Sampler>());
			}

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
				pbr->material->hasEmissiveTex = material.emissiveTexture.index >= 0;
				pbr->material->isDoubleSided = material.doubleSided ? 1 : 0;
				pbr->material->alphaMode = material.alphaMode == "OPAQUE" ? PBRMaterial::ALPHA_MODE::ALPHA_OPAQUE : material.alphaMode == "MASK" ? PBRMaterial::ALPHA_MODE::ALPHA_MASK : PBRMaterial::ALPHA_MODE::ALPHA_TRANSPARENT;
				pbr->material->alphaCutoff = material.alphaCutoff;
				pbr->material->occlusionStrength = material.occlusionTexture.strength;

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
					if (channel.target_path == "weights")
						animationType = Animation::AnimationType::WEIGHTS;
					Animation::SamplerType samplerType = sampler.interpolation == "LINEAR" ? Animation::SamplerType::LINEAR : sampler.interpolation == "STEP" ? Animation::SamplerType::STEP : Animation::SamplerType::CUBIC;
					auto& inputAccessor = model.accessors[sampler.input];
					assert(inputAccessor.type == 65);
					assert(inputAccessor.componentType == 5126);
					auto& inputBufferView = model.bufferViews[inputAccessor.bufferView];
					Vector<f32> inputVector;
					u32 startOfInputBuffer = inputAccessor.byteOffset + inputBufferView.byteOffset;
					u32 strideOfInputBuffer = inputBufferView.byteStride == 0 ? sizeof(f32) : inputBufferView.byteStride;
					f32 minInput = inputAccessor.minValues.size() > 0 ? inputAccessor.minValues[0] : 1000.f;
					f32 maxInput = inputAccessor.maxValues.size() > 0 ? inputAccessor.maxValues[0] : 0.f;
					
					for (int j = 0; j < inputAccessor.count; ++j)
					{
						u32 index = (startOfInputBuffer + strideOfInputBuffer * j);
						f32 val = static_cast<f32>(((f32*)model.buffers[inputBufferView.buffer].data.data())[index / sizeof(f32)]);
						if (val < minInput)
							minInput = val;
						if (val > maxInput)
							maxInput = val;
						inputVector.push_back(val);
					}

					Vector<f32> scalarOutput;
					Vector<vec3> vec3Output;
					Vector<vec4> vec4Output;

					auto& outputAccessor = model.accessors[sampler.output];
					auto& outputBufferView = model.bufferViews[outputAccessor.bufferView];
					assert(outputAccessor.componentType == 5126);
					if (outputAccessor.type == 65)
					{
						// scalar
						u32 startOfOutputBuffer = outputAccessor.byteOffset + outputBufferView.byteOffset;
						u32 strideOfOutputBuffer = outputBufferView.byteStride == 0 ? sizeof(f32) : outputBufferView.byteStride;
						for (int j = 0; j < outputAccessor.count; ++j)
						{
							u32 index = (startOfOutputBuffer + strideOfOutputBuffer * j);
							f32 val;
							val = static_cast<f32>(((f32*)model.buffers[outputBufferView.buffer].data.data())[index / sizeof(f32)]);
							scalarOutput.push_back(val);
						}
					}
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
					auto newAnimation = MakeShared<Animation>(animationType, samplerType, minInput, maxInput, inputVector, vec3Output, vec4Output, scalarOutput);
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
			
				if (nodeType == Node::MESH_NODE )
				{
					auto& gltfMesh = model.meshes[node.mesh];
					for (auto primitive : gltfMesh.primitives)
					{
						auto pipeline = forwardPipeline;
						if (primitive.material >= 0 && pbrMaterials[primitive.material]->material->alphaMode == Graphics::PBRMaterial::ALPHA_MODE::ALPHA_TRANSPARENT)
						{
							pipeline = forwardTransparentPipeline;
						}
						auto geometry = MakeShared<GLTFMesh>(pipeline, filename, primitive, model, primitive.material >= 0 ? pbrMaterials[primitive.material] : nullptr);
						geometry->node = newNode;
						
						newMeshes.push_back(geometry);

					}
				}
				else if (nodeType == Node::SKINNED_MESH_NODE)
				{
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
					auto& gltfMesh = model.meshes[node.mesh];
					for (auto primitive : gltfMesh.primitives)
					{
						auto pipeline = forwardPipeline;
						if (primitive.material >= 0 && pbrMaterials[primitive.material]->material->alphaMode == Graphics::PBRMaterial::ALPHA_MODE::ALPHA_TRANSPARENT)
						{
							pipeline = forwardTransparentPipeline;
						}
						auto geometry = MakeShared<GLTFMesh>(pipeline, filename, primitive, model, primitive.material >= 0 ? pbrMaterials[primitive.material] : nullptr, invBindMatrices);
						geometry->node = newNode;
						// setup compute vertex

						// TODO
						nodeToJoints.push_back(std::make_pair(geometry, model.skins[node.skin].joints));
						newMeshes.push_back(geometry);
					}
				}

				if (node.mesh > -1)
				{
					for (f32 weight : model.meshes[node.mesh].weights)
					{
						newNode->morphWeights.push_back(weight);
					}
				}

				for (auto& child : node.children)
				{
					nodesStack.push_back(std::make_pair<u32, u32>(child, newNode->nodeID.id));
				}

				newNode->animations = animationToNodes[nodeFront.first];
				for (auto& anim : newNode->animations)
				{
					if (anim->minInput > newNode->minAnimationTime)
						newNode->minAnimationTime = anim->minInput;
					if (anim->maxInput < newNode->maxAnimationTime)
						newNode->maxAnimationTime = anim->maxInput;
				}
				newNode->gltfID = nodeFront.first;
			}
		}

		for (auto res : nodeToJoints)
		{
			SharedPtr<GLTFMesh> node = res.first;
			Vector<int> jointIDs = res.second;
			Vector<SharedPtr<Node>> joints;

			for (int jointID : jointIDs)
			{
				for (auto n : nodeManager.nodes)
				{
					// if multiple skeletal mesh, start adding the joints relevant to the present one
					if (n && n->nodeID.id > curNumNodes && n->gltfID == jointID)
					{
						joints.push_back(n);
					}
				}
			}

			node->SetJoints(joints);
		}
		// reading first mesh only
		//auto& mesh = model.meshes[0];

		//LoadGLTFMesh(mesh, model, vertices, indices, mainTextureURI);
		

	}

}
