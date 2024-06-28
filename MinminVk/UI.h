#pragma once

#include "util/Type.h"

namespace UI
{
	// Hair parameters
	f32 windStrength = 3;
	vec3 windDirection = vec3(-1.f, -0.f, 0.f);
	f32 shockStrength = 50;
	u32 elcIteration = 10;
	f32 stiffnessLocal = 0.5f;
	f32 stiffnessGlobal = 0.1f;
	f32 effectiveRangeGlobal = 0.1f;
	f32 capsuleRadius = 0.12f;
	bool rotateHead = false;
	bool resetHeadPos = false;
	vec3 cameraPosition = vec3(0.01f, -1.0f, 11.0f);
	vec3 cameraLookDirection = vec3(0.0f, 0.f, -1.0f);
}