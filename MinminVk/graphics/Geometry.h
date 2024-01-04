#pragma once

#include <util/Type.h>
#include <graphics/Device.h>

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
			vec3 color;
		};

		Vector<Vertex> vertices;


		BasicVertex() {};

		BasicVertex(Vector<Vertex> &vertices) : vertices{ vertices } {}

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
			posAttribute.binding = 0;
			colorAttribute.location = 1;
			colorAttribute.offset = offsetof(Vertex, color);
			colorAttribute.vertexFormatType = VertexAttribute::VertexFormatType::VEC3;
			return Vector<VertexAttribute>{ posAttribute, colorAttribute };
		}
	};

	struct GeometryID { u32 vertexBufferID = 0; u32 indexBufferID = 0; };

	struct Quad
	{
		BasicVertex vertexDesc{ Vector<BasicVertex::Vertex>{
				//{{-0.5f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
				//{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
				//{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}},

				//{{-0.5f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
				//{{0.5f, -0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
				//{{0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}},

				{{-0.5f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f, -0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
				{{0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}},
				{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}},
			}
		};

		Vector<u16> indices = {
			0, 1, 2, 2, 3, 0
		};

		GeometryID geometryID;

		Quad();

		void Draw(RenderContext&);

	};

}