#pragma once

#include <util/Type.h>
#include <graphics/Device.h>
#include <graphics/Buffer.h>
#include <util/Math.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace Graphics
{
	struct RenderContext;

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
		enum class VertexFormatType { FLOAT, VEC2, VEC3, VEC4 };
		VertexFormatType vertexFormatType;

	};

	struct VertexDesc
	{
		virtual VertexBinding GetVertexBinding() = 0;
		virtual Vector<VertexAttribute> GetVertexAttributes() = 0;
	};

	struct BasicVertex : public VertexDesc
	{
		struct Vertex 
		{
			vec3 pos;
			vec3 color = vec3(1);
			vec2 texCoord;

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

			return Vector<VertexAttribute>{ posAttribute, colorAttribute, uvAttribute };
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
	};

	struct GeometryID { u32 vertexBufferID = 0; u32 indexBufferID = 0; };

	struct Geometry
	{
	GeometryID geometryID;
	SharedPtr<BasicUniformBuffer> basicUniform;
	mat4 modelMatrix = mat4(1);
	Texture mainTexture;

	protected:
		BasicVertex vertexDesc;
		Vector<u16> indices;
	public:
		virtual BasicVertex& GetVertexData() { return vertexDesc; }
		virtual Vector<u16>& GetIndicesData() { return indices; }
		virtual void Draw(RenderContext&);
		virtual void Update(f32 deltaTime) {};

		Geometry(SharedPtr<BasicUniformBuffer> basicUniform, Texture mainTexture);
	};

	struct Quad : public Geometry
	{
	private:
		BasicVertex vertexDesc{ std::move(Vector<BasicVertex::Vertex>{
				{ {-0.5f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
				{{0.5f, -0.5f, 0.f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
				{{0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
				{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			})
		};

		Vector<u16> indices = {
			0, 1, 2, 2, 3, 0
		};
	public:


		Quad(int descriptorPool, SharedPtr<BasicUniformBuffer> uboTransform, Texture mainTexture);

		BasicVertex& GetVertexData() override { return vertexDesc; }
		Vector<u16>& GetIndicesData() override { return indices; }

		void Update(f32 deltaTime) override
		{
			//modelMatrix = Math::Rotate(modelMatrix, deltaTime * Math::Radians(90), vec3(0, 0, 1));
		}

	};

	struct OBJMesh : public Geometry
	{
	public:
		OBJMesh(int descriptorPoolID, SharedPtr<BasicUniformBuffer> uboTransform, Texture mainTexture, String filename);
		OBJMesh(int descriptorPoolID, SharedPtr<BasicUniformBuffer> uboTransform, String filename);
		void Update(f32 deltaTime) override
		{
			modelMatrix = Math::Rotate(modelMatrix, deltaTime * Math::Radians(90), vec3(0, -1, 0));
			//modelMatrix = Math::Translate(modelMatrix, vec3(0., 0.1, 0));
		}
	};

	struct GLTFMesh : public Geometry
	{
	public:
		GLTFMesh(int descriptorPoolID, SharedPtr<BasicUniformBuffer> uboTransform, String filename, u32 meshIndex);
		void Update(f32 deltaTime) override
		{
			modelMatrix = Math::Rotate(modelMatrix, deltaTime * Math::Radians(90), vec3(0, -1, 0));
			//modelMatrix = Math::Translate(modelMatrix, vec3(0., 0.1, 0));
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