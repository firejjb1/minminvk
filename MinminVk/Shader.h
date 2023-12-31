#pragma once
#include "Type.h"
#include "IO.h"

namespace Graphics 
{
	struct Shader
	{
		String shaderPath;
		Vector<char> shaderCode;
		String entryPoint;

		enum class ShaderType
		{
			SHADER_VERTEX,
			SHADER_FRAGMENT,
			SHADER_COMPUTE,
			SHADER_RAYTRACE,
			COUNT
		};
		ShaderType shaderType;

		Shader(String shaderPath, ShaderType shaderType, String entryPoint) 
			: shaderPath {shaderPath}, shaderType{shaderType}, entryPoint{entryPoint}
		{
			shaderCode = Common::ReadFile(shaderPath);
		}

		//void Init();
	};
}