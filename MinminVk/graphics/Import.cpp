#include <graphics/Import.h>
#include <graphics/Geometry.h>
#include <util/IO.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>

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

	void Import::LoadGLTF(const String& filename, Graphics::BasicVertex& vertices, Vector<u16>& indices, String& mainTextureURI)
	{
		tinygltf::Model model;
		Util::IO::ReadGLTF(model, filename);

		// reading first mesh only
		auto& mesh = model.meshes[0];
		tinygltf::Accessor positionAccessor;
		tinygltf::Accessor normalAccessor;
		tinygltf::Accessor uvAccessor;
		auto indicesAccessor = model.accessors[mesh.primitives[0].indices];

		// reading first primitive only
		mainTextureURI = model.images[model.textures[model.materials[mesh.primitives[0].material].pbrMetallicRoughness.baseColorTexture.index].source].uri;
		for (auto& attrib : mesh.primitives[0].attributes)
		{
			if (attrib.first.compare("POSITION") == 0)
			{
				positionAccessor = model.accessors[attrib.second];
			}

			if (attrib.first.compare("NORMAL") == 0)
			{
				normalAccessor = model.accessors[attrib.second];
			}
			if (attrib.first.compare("TEXCOORD_0") == 0)
			{
				uvAccessor = model.accessors[attrib.second];
			}
		}
		// only support f32 types for now
		assert(positionAccessor.componentType == 5126);
		assert(normalAccessor.componentType == 5126);
		assert(uvAccessor.componentType == 5126);

		assert(indicesAccessor.componentType == 5123);

		vertices.vertices.resize(positionAccessor.count);
		
		auto positionBufferView = model.bufferViews[positionAccessor.bufferView];
		DebugPrint("Positions\n");
		for (int i = positionBufferView.byteOffset / sizeof(f32); i < positionBufferView.byteOffset / sizeof(f32) + positionBufferView.byteLength / sizeof(f32); ++i)
		{
			f32 val = static_cast<f32>(((f32*)model.buffers[positionBufferView.buffer].data.data())[i]);
			DebugPrint("%f ", val);
			if ((i + 1) % 3 == 0)
				DebugPrint("\n");

			vertices.vertices[(i - positionBufferView.byteOffset / sizeof(f32)) / 3].pos[i % 3] = val;
		}

		//auto normalBufferView = model.bufferViews[normalAccessor.bufferView];
		//DebugPrint("Normals\n");
		//for (int i = normalBufferView.byteOffset / sizeof(f32); i < normalBufferView.byteOffset / sizeof(f32) + normalBufferView.byteLength / sizeof(f32); ++i)
		//{
		//	f32 val = static_cast<f32>(((f32*)model.buffers[normalBufferView.buffer].data.data())[i]);
		//	DebugPrint("%f ", val);
		//	if ((i + 1) % 3 == 0)
		//		DebugPrint("\n");
		//}

		auto uvBufferView = model.bufferViews[uvAccessor.bufferView];
		DebugPrint("UV\n");
		for (int i = uvBufferView.byteOffset / sizeof(f32); i < uvBufferView.byteOffset / sizeof(f32) + uvBufferView.byteLength / sizeof(f32); ++i)
		{
			f32 val = static_cast<f32>(((f32*)model.buffers[uvBufferView.buffer].data.data())[i]);
			val = 0.5f * val + 0.5f;
			DebugPrint("%f ", val);
			if ((i + 1) % 2 == 0)
				DebugPrint("\n");

			vertices.vertices[(i - uvBufferView.byteOffset / sizeof(f32)) / 2].texCoord[i % 2] = val;

		}

		DebugPrint("Indices\n");
		auto indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
		for (int i = indicesBufferView.byteOffset / sizeof(u16); i < indicesAccessor.byteOffset / sizeof(u16) + indicesBufferView.byteLength / sizeof(u16); ++i)
		{
			u16 val = static_cast<u16>(((u16*)model.buffers[indicesBufferView.buffer].data.data())[i]);
			DebugPrint("%d\n", val);
			indices.push_back(val);
		}

	}

}
