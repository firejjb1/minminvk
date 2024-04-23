#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <util/Type.h>
#include <util/IO.h>

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

		// TODO parse glsl file and compile them
		Shader(String shaderPath, ShaderType shaderType, String entryPoint) 
			: shaderPath {shaderPath}, shaderType{shaderType}, entryPoint{entryPoint}
		{
			if (entryPoint != "main")
			{
				DebugPrint("ERROR: glsl only supports entry point called main for now.\n");
				assert(false);
			}
			shaderCode = Util::IO::ReadFile(shaderPath);
		}

		//void Init();
	};
}

#endif