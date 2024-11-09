#pragma once

#include <util/Type.h>
#include <graphics/Node.h>

namespace tinygltf
{
	struct Mesh;
	struct Model;
	struct Primitive;
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
	struct PBRMaterial;
	struct Sampler;

	struct Import
	{
		static Vector<SharedPtr<Sampler>> textureSamplers;

		static void LoadOBJ(Graphics::BasicVertex& vertices, Vector<u16>& indices, const String& filename);

		static void LoadHairStrands(Vector<f32>& vertices, const String& filename);

		static void LoadTextures(const String filename, tinygltf::Primitive& mesh, tinygltf::Model& model, Texture& mainTexture, Texture& metallic, Texture& normal, Texture& occlusion, Texture& emissive);

		static void LoadGLTFMesh(const String filename, tinygltf::Primitive& mesh, tinygltf::Model& model, Graphics::BasicVertex& vertices, Vector<u16>& indices, Texture& mainTexture, Texture& metallic, Texture& normal, Texture& occlusion, Texture& emissive);

		static SharedPtr<Node> LoadGLTF(const String& filename, NodeManager& nodeManager, SharedPtr<GraphicsPipeline> forwardPipeline, SharedPtr<GraphicsPipeline> forwardTransparentPipeline, Vector<SharedPtr<GLTFMesh>>& newMeshes);

	};
}
