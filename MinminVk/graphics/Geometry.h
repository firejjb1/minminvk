#pragma once

#include <util/Type.h>
#include <graphics/Device.h>
#include <graphics/Buffer.h>
#include <graphics/Node.h>
#include <graphics/Animation.h>
#include <util/Math.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace Graphics
{
	struct RenderContext;

	struct GraphicsPipeline;

	struct VertexBinding
	{
		u32 stride = 0;
		u32 binding = 0;
		enum class InputRateType { VERTEX, INSTANCE };
		InputRateType inputRateType = InputRateType::VERTEX;
	};

	struct VertexAttribute
	{
		u32 binding = 0;
		u32 location = 0;
		u32 offset = 0;
		enum class VertexFormatType { FLOAT, VEC2, VEC3, VEC4, UVEC2, UVEC3, UVEC4 };
		VertexFormatType vertexFormatType;

	};

	struct VertexDesc
	{
		virtual VertexBinding GetVertexBinding() = 0;
		virtual Vector<VertexAttribute> GetVertexAttributes() = 0;
		virtual u8* GetVertices() = 0;
		virtual u32 GetVerticesCount() = 0;
	};

	struct BasicVertex : public VertexDesc
	{
		struct Vertex 
		{
			vec3 pos;
			vec3 color = vec3(1);
			vec2 texCoord;
			vec3 normal = vec3(0,0,1);

			bool operator==(const Vertex& other) const {
				return pos == other.pos && color == other.color && texCoord == other.texCoord;
			}

		};

		Vector<Vertex> vertices;

		BasicVertex() {};

		BasicVertex(Vector<Vertex> &&vertices) : vertices{ vertices } {}

		VertexBinding GetVertexBinding() override
		{
			VertexBinding binding;
			binding.stride = sizeof(Vertex);
			binding.binding = 0;
			return binding;
		}

		Vector<VertexAttribute> GetVertexAttributes() override
		{
			VertexAttribute posAttribute;
			posAttribute.binding = 0;
			posAttribute.location = 0;
			posAttribute.offset = offsetof(Vertex, pos);
			posAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC3;

			VertexAttribute colorAttribute;
			colorAttribute.binding = 0;
			colorAttribute.location = 1;
			colorAttribute.offset = offsetof(Vertex, color);
			colorAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC3;

			VertexAttribute uvAttribute;
			uvAttribute.binding = 0;
			uvAttribute.location = 2;
			uvAttribute.offset = offsetof(Vertex, texCoord);
			uvAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC2;
						
			VertexAttribute normalAttribute;
			normalAttribute.binding = 0;
			normalAttribute.location = 3;
			normalAttribute.offset = offsetof(Vertex, normal);
			normalAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC3;

			return Vector<VertexAttribute>{ posAttribute, colorAttribute, uvAttribute, normalAttribute };
		}

		u8* GetVertices() override
		{
			return ((u8*)vertices.data());
		}

		u32 GetVerticesCount() override
		{
			return vertices.size() * sizeof(Vertex) / sizeof(u8);
		}
	};

	struct SkinnedVertex : public VertexDesc
	{
		struct Vertex 
		{
			vec3 pos;
			vec2 texCoord;
			vec3 normal;
			vec4 weights;
			vec4u joints;

			bool operator==(const Vertex& other) const {
				return pos == other.pos && weights == other.weights && texCoord == other.texCoord && joints == other.joints && normal == other.normal;
			}

		};

		Vector<Vertex> vertices;

		SkinnedVertex() {};

		SkinnedVertex(Vector<Vertex> &&vertices) : vertices{ vertices } {}

		VertexBinding GetVertexBinding() override
		{
			VertexBinding binding;
			binding.stride = sizeof(Vertex);
			binding.binding = 0;
			return binding;
		}

		Vector<VertexAttribute> GetVertexAttributes() override
		{
			VertexAttribute posAttribute;
			posAttribute.binding = 0;
			posAttribute.location = 0;
			posAttribute.offset = offsetof(Vertex, pos);
			posAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC3;


			VertexAttribute uvAttribute;
			uvAttribute.binding = 0;
			uvAttribute.location = 1;
			uvAttribute.offset = offsetof(Vertex, texCoord);
			uvAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC2;

			VertexAttribute normalAttribute;
			normalAttribute.binding = 0;
			normalAttribute.location = 2;
			normalAttribute.offset = offsetof(Vertex, normal);
			normalAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC3;
			
			VertexAttribute weightsAttribute;
			weightsAttribute.binding = 0;
			weightsAttribute.location = 3;
			weightsAttribute.offset = offsetof(Vertex, weights);
			weightsAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC4;	
			
			VertexAttribute jointsAttribute;
			jointsAttribute.binding = 0;
			jointsAttribute.location = 4;
			jointsAttribute.offset = offsetof(Vertex, joints);
			jointsAttribute.vertexFormatType = VertexAttribute::VertexFormatType::UVEC4;

			return Vector<VertexAttribute>{ posAttribute, uvAttribute, normalAttribute, weightsAttribute, jointsAttribute };
		}

		u8* GetVertices() override
		{
			return ((u8*)vertices.data());
		}

		u32 GetVerticesCount() override
		{
			return vertices.size() * sizeof(Vertex) / sizeof(u8);
		}
	};

	struct ParticleVertex : public VertexDesc
	{
		struct Particle
		{
			vec4 position;
			//vec2 velocity;
			vec4 color;
		};
		Vector<Particle> particles;

		ParticleVertex() {}

		ParticleVertex(Vector<Particle> &particles) : particles{particles} {}

		VertexBinding GetVertexBinding() override
		{
			VertexBinding binding;
			binding.stride = sizeof(Particle);
			binding.binding = 0;
			return binding;
		}

		Vector<VertexAttribute> GetVertexAttributes() override
		{
			VertexAttribute posAttribute;
			posAttribute.binding = 0;
			posAttribute.location = 0;
			posAttribute.offset = offsetof(Particle, position);
			posAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC4;

			VertexAttribute colorAttribute;
			colorAttribute.binding = 0;
			colorAttribute.location = 1;
			colorAttribute.offset = offsetof(Particle, color);
			colorAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC4;

			return Vector<VertexAttribute>{ posAttribute, colorAttribute };
		}

		u8* GetVertices() override
		{
			return ((u8*)particles.data());
		}

		u32 GetVerticesCount() override
		{
			return particles.size() * sizeof(Particle) / sizeof(u8);
		}
	};

	struct GeometryID { u32 vertexBufferID = 0; u32 indexBufferID = 0; u32 setID; };

	struct Geometry
	{

		SharedPtr<Node> node;
		GeometryID geometryID;
		SharedPtr<BasicUniformBuffer> basicUniform;
		Texture mainTexture;

	protected:
		SharedPtr<VertexDesc> vertexDesc;
		Vector<u16> indices;
	public:
		virtual SharedPtr<VertexDesc> GetVertexData() { return vertexDesc; }
		virtual Vector<u16>& GetIndicesData() { return indices; }
		virtual void Draw(RenderContext&);
		virtual void Update(f32 deltaTime) {};

		Geometry(SharedPtr<BasicUniformBuffer> basicUniform, Texture mainTexture);

		Geometry() {
			vertexDesc = MakeShared<BasicVertex>();
		}

	};

	struct Quad : public Geometry
	{
	private:

		SharedPtr<VertexDesc> vertexDesc;

		Vector<u16> indices = {
			0, 1, 2, 2, 3, 0
		};
	public:


		Quad(SharedPtr<GraphicsPipeline>, SharedPtr<BasicUniformBuffer> uboTransform, Texture mainTexture);

		SharedPtr<VertexDesc> GetVertexData() override { return vertexDesc; }
		Vector<u16>& GetIndicesData() override { return indices; }

		void Update(f32 deltaTime) override
		{
			//modelMatrix = Math::Rotate(modelMatrix, deltaTime * Math::Radians(90), vec3(0, 0, 1));
		}

	};

	struct OBJMesh : public Geometry
	{
	public:
		OBJMesh(SharedPtr<GraphicsPipeline>, SharedPtr<BasicUniformBuffer> uboTransform, Texture mainTexture, String filename);
		OBJMesh(SharedPtr<GraphicsPipeline>, SharedPtr<BasicUniformBuffer> uboTransform, String filename);
		void Update(f32 deltaTime) override
		{
			//node->modelMatrix = Math::Rotate(node->modelMatrix, deltaTime * Math::Radians(90), vec3(0, -1, 0));
			//modelMatrix = Math::Translate(modelMatrix, vec3(0., 0.1, 0));
		}
	};

	struct GLTFMesh : public Geometry
	{
	public:
		GLTFMesh(SharedPtr<GraphicsPipeline>, SharedPtr<BasicUniformBuffer> basicUniform, String filename, tinygltf::Mesh& mesh, tinygltf::Model& model);
		void Update(f32 deltaTime) override
		{

		}
	};

	struct GLTFSkinnedMesh : Geometry
	{
	protected:
		Vector<SharedPtr<Node>> joints;
		Vector<mat4> inverseBindMatrices;

	public:
		GLTFSkinnedMesh(SharedPtr<GraphicsPipeline>, SharedPtr<BasicUniformBuffer> basicUniform, String filename, tinygltf::Mesh& mesh, tinygltf::Model& model);

		void SetInverseBindMatrices(Vector<mat4>& inverseBindMatrices)
		{
			this->inverseBindMatrices = inverseBindMatrices;
		}

		void SetJoints(Vector<SharedPtr<Node>> joints)
		{
			this->joints = joints;
		}

		void Update(f32 deltaTime) override
		{

			mat4 invWorld = Math::Inverse(node->worldMatrix);
			for (int i = 0; i < joints.size(); ++i)
			{
				auto joint = joints[i];
				basicUniform->transformUniform.jointMatrices[i] = (invWorld * joint->worldMatrix * inverseBindMatrices[i]);
			}
		}
	};
}

namespace std {
	template<> struct hash<Graphics::BasicVertex::Vertex> {
		size_t operator()(Graphics::BasicVertex::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}