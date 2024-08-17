#pragma once

#include <util/Type.h>
#include <util/Math.h>

namespace Graphics
{
	struct Animation
	{
		const static u32 maxAnimationTime = 5;

		enum class AnimationType { ROTATION, TRANSLATION, SCALE, WEIGHTS };

		AnimationType animationType;

		enum class SamplerType { LINEAR, STEP, CUBIC };

		SamplerType samplerType = SamplerType::LINEAR;

		enum class OutputType { SCALAR, VEC3, VEC4 };

		OutputType outputType = OutputType::VEC3;

		Vector<f32> input;

		Vector<f32> scalarOutput;
		Vector<vec3> vec3Output;
		// vec4 for rotation only (quaternion)
		Vector<vec4> vec4Output;

		u32 numWeightsMorphTarget = 1;


		Animation(AnimationType animationType, SamplerType samplerTYpe, Vector<f32>& input, Vector<vec3>& vec3Output, Vector<vec4>& vec4Output, Vector<f32>& scalarOutput) :
			animationType{ animationType }, samplerType{ samplerType }, input{ input }, vec3Output{ vec3Output }, vec4Output{ vec4Output }, scalarOutput{ scalarOutput }
		{
			assert(input.size() == vec3Output.size() || input.size() == vec4Output.size() || scalarOutput.size() >= input.size());
			assert(input.size() > 0);
			assert(animationType == AnimationType::ROTATION && vec4Output.size() > 0 || vec3Output.size() > 0 || scalarOutput.size() > 0);
			if (vec3Output.size() != 0)
				outputType = OutputType::VEC3;
			if (vec4Output.size() != 0)
				outputType = OutputType::VEC4;
			if (scalarOutput.size() != 0)
				outputType = OutputType::SCALAR;
			if (animationType == AnimationType::WEIGHTS)
				numWeightsMorphTarget = scalarOutput.size() / input.size();
		}

		inline vec4 gltfQuatToVec4(vec4 &gltfQuat)
		{
			return vec4(gltfQuat.w, gltfQuat.x, gltfQuat.y, gltfQuat.z);
		}

		inline vec4 ReadMorphWeights(u32 i)
		{
			vec4 res;
			for (int morphI = 0; morphI < Min((u32)4, numWeightsMorphTarget); ++morphI)
			{
				res[morphI] = scalarOutput[i * numWeightsMorphTarget + morphI];
			}
			return res;
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
						if (outputType == OutputType::SCALAR)
						{
							// read morph weights 4 at a time
							return ReadMorphWeights(i);
						}
						return outputType == OutputType::VEC3 ? vec4(vec3Output[i], 0) : gltfQuatToVec4(vec4Output[i]);
					}
					f32 lastInputVal = input[i - 1];
					f32 inputDiff = inputVal - lastInputVal;
					f32 a = (inputVal - samplingTime) / inputDiff;
					f32 b = (samplingTime - lastInputVal) / inputDiff;
					if (outputType == OutputType::SCALAR)
					{
						vec4 w1 = ReadMorphWeights(i);
						vec4 w2 = ReadMorphWeights(i - 1);
						return glm::mix(w1, w2, a);
					}
					else if (outputType == OutputType::VEC3)
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
			if (outputType == OutputType::SCALAR)
				return ReadMorphWeights(input.size() - 1);
			return outputType == OutputType::VEC3 ? vec4(vec3Output[vec3Output.size() - 1], 0) : gltfQuatToVec4(vec4Output[vec4Output.size() - 1]);
		}
	};
}