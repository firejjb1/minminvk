#pragma once

#include <util/Type.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Math
{
	inline static const f32 PI = 3.1415926f;

	inline const f32 Radians(f32 deg)
	{
		return glm::radians(deg);
	}

	inline const mat4 Rotate(const mat4 &matrix, const f32 radian, const vec3 axis)
	{
		return glm::rotate(matrix, radian, axis);
	}

	inline const mat4 RotateQuat(const quat q)
	{
		return glm::toMat4(q);
	}

	inline mat4 Translate(const mat4 &matrix, const vec3 translation)
	{
		return glm::translate(matrix, translation);
	}

	inline mat4 Scale(const mat4 &matrix, const vec3 scale)
	{
		return glm::scale(matrix, scale);
	}

	inline mat4 LookAt(const vec3 eye, const vec3 center, const vec3 up)
	{
		return glm::lookAtLH(eye, center, up);
	}

	inline mat4 Perspective(f32 fovY, i32 width, i32 height, f32 zNear, f32 zFar)
	{
		return glm::perspectiveLH(fovY, width / (f32)height, zNear, zFar);
	}

	inline mat4 Inverse(mat4 mat)
	{
		return glm::inverse(mat);
	}
}
