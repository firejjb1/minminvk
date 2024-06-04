#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include <set>
#include <optional>
#include <string>
#include <limits>
#include <algorithm>
#include <map>

#define concat_str(first, second) first second

typedef uint32_t u32;
typedef uint16_t u16;

typedef float f32;
typedef int32_t i32;

template <class T>
using Optional = std::optional<T>;

typedef glm::mat4 mat4;
typedef glm::mat3 mat3;


typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::uvec2 vec2u;
typedef glm::uvec3 vec3u;
typedef glm::uvec4 vec4u;

typedef std::string String;

template <class T>
using SharedPtr = std::shared_ptr<T>;
template <class T, class... Args>
auto MakeShared(Args&&... args) -> decltype(std::make_shared<T>(std::forward<Args>(args)...))
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<class T>
using Vector = std::vector<T>; 

template<class K, class V >
using Map = std::map<K, V>;

template<class T>
using Set = std::set<T>;

template<class T>
auto Min(const T& a, const T& b) -> const T&
{
	return std::min<T>(a, b);
}

template<class T>
auto Max(const T& a, const T& b) -> const T&
{
	return std::max<T>(a, b);
}

constexpr auto DebugPrint = printf;

template<class T>
T EnumBitwiseOr(T a, T b)
{
	return static_cast<T>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

template<class T>
bool EnumBitwiseAnd(T a, T b)
{
	return static_cast<bool > (static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
}