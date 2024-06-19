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

	void Import::LoadGLTF(const String& filename)
	{
		tinygltf::Model model;
		Util::IO::ReadGLTF(model, filename);
	}

}
