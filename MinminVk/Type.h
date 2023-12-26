#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>
#include <set>
#include <optional>

typedef uint32_t u32;
typedef float f32;
typedef int32_t i32;

template <class T>
using Optional = std::optional<T>;

typedef glm::mat4 mat4;
typedef glm::mat3 mat3;


typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

template <class T>
using SharedPtr = std::shared_ptr<T>;
template <class T, class... Args>
auto MakeShared(Args&&... args) -> decltype(std::make_shared<T>(std::forward<Args>(args)...))
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<class T>
using Vector = std::vector<T>; 

template<class T>
using Set = std::set<T>;

constexpr auto DebugPrint = printf;

namespace Test
{
	void TestGLM()
	{
		glm::mat4 matrix;
		glm::vec4 vec;
		auto test = matrix * vec;
	}
}