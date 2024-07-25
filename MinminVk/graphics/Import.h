#pragma once

#include <util/Type.h>
#include <graphics/Node.h>

namespace tinygltf
{
	struct Mesh;
	struct Model;
}

namespace Graphics
{
	struct BasicVertex;
	struct Texture;
	struct BasicUniformBuffer;
	struct GLTFMesh;
	struct SkinnedVertex;
	struct GLTFSkinnedMesh;
	struct GraphicsPipeline;

	struct Import
	{
		static void LoadOBJ(Graphics::BasicVertex& vertices, Vector<u16>& indices, const String& filename);

		static void LoadHairStrands(Vector<f32>& vertices, const String& filename);

		static void LoadGLTFMesh(const String filename, tinygltf::Mesh& mesh, tinygltf::Model& model, Graphics::BasicVertex& vertices, Vector<u16>& indices, Texture& mainTexture);

		static void LoadGLTF(const String& filename, NodeManager& nodeManager, SharedPtr<GraphicsPipeline> forwardPipeline, SharedPtr<GraphicsPipeline> forwardSkinnedPipeline, Vector<SharedPtr<GLTFMesh>>& newMeshes, Vector<SharedPtr<GLTFSkinnedMesh>>& newSkinnedMeshes);

		static void LoadGLTFSkinnedMesh(const String filename, tinygltf::Mesh& mesh, tinygltf::Model& model, Graphics::SkinnedVertex& vertices, Vector<u16>& indices, Texture& mainTexture);
	};
}
