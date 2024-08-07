#pragma once

#include <util/Type.h>
#include <util/Math.h>

namespace Graphics
{
	struct Animation
	{
		const static u32 maxAnimationTime = 5;

		enum class AnimationType { ROTATION, TRANSLATION, SCALE };

		AnimationType animationType;

		enum class SamplerType { LINEAR, STEP, CUBIC };

		SamplerType samplerType = SamplerType::LINEAR;

		enum class OutputType { VEC3, VEC4 };

		OutputType outputType = OutputType::VEC3;

		Vector<f32> input;

		Vector<vec3> vec3Output;
		// vec4 for rotation only (quaternion)
		Vector<vec4> vec4Output;

		Animation(AnimationType animationType, SamplerType samplerTYpe, Vector<f32>& input, Vector<vec3>& vec3Output, Vector<vec4>& vec4Output) :
			animationType{ animationType }, samplerType{ samplerType }, input { input}, vec3Output{ vec3Output }, vec4Output{ vec4Output } 
		{
			assert(input.size() == vec3Output.size() || input.size() == vec4Output.size());
			assert(input.size() > 0);
			assert(animationType == AnimationType::ROTATION && vec4Output.size() > 0 || vec3Output.size() > 0);
			if (vec3Output.size() != 0)
				outputType = OutputType::VEC3;
			if (vec4Output.size() != 0)
				outputType = OutputType::VEC4;
		}

		inline vec4 gltfQuatToVec4(vec4 &gltfQuat)
		{
			return vec4(gltfQuat.w, gltfQuat.x, gltfQuat.y, gltfQuat.z);
		}

		vec4 Sample(f32 timer)
		{
			f32 samplingTime = timer;

			for (u32 i = 0; i < input.size(); ++i)
			{
				f32 inputVal = input[i];
				if (inputVal > samplingTime)
				{
					if (i == 0)
					{
						return outputType == OutputType::VEC3 ? vec4(vec3Output[i], 0) : gltfQuatToVec4(vec4Output[i]);
					}
					f32 lastInputVal = input[i - 1];
					f32 inputDiff = inputVal - lastInputVal;
					f32 a = (inputVal - samplingTime) / inputDiff;
					f32 b = (samplingTime - lastInputVal) / inputDiff;
					if (outputType == OutputType::VEC3)
						return vec4(glm::mix(vec3Output[i], vec3Output[i - 1], a), 0);
					else
					{
						// vec4 output
						vec4 quatA = gltfQuatToVec4(vec4Output[i]);
						vec4 quatB = gltfQuatToVec4(vec4Output[i - 1]);
						quat lerpedRot = glm::slerp(quat(quatB.x, quatB.y, quatB.z, quatB.w), quat(quatA.x, quatA.y, quatA.z, quatA.w), b);
						return vec4(lerpedRot.w, lerpedRot.x, lerpedRot.y, lerpedRot.z);
					}
				}
			}
			// reached the end (clamp to end)
			return outputType == OutputType::VEC3 ? vec4(vec3Output[vec3Output.size() - 1], 0) : gltfQuatToVec4(vec4Output[vec4Output.size() - 1]);
		}




	};
}