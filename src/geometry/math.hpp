#ifndef SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_
#define SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_

#include <glm/glm.hpp>
#include <limits>

namespace skity {

using Matrix = glm::mat4;

#define Float1 1.0f
#define FloatHalf 0.5f
#define FloatNaN std::numeric_limits<float>::quiet_NaN()
#define FloatInfinity (+std::numeric_limits<float>::infinity())
#define NearlyZero (Float1 / (1 << 12))
#define FloatRoot2Over2 0.707106781f

static inline bool FloatNearlyZero(float x, float tolerance = NearlyZero) {
  return glm::abs(x) <= tolerance;
}

static inline float CubeRoot(float x) { return glm::pow(x, 0.3333333f); }

static inline bool FloatIsNan(float x) { return x != x; }

[[clang::no_sanitize("float-divide-by-zero")]] static inline float
SkityIEEEFloatDivided(float number, float denom) {
  return number / denom;
}

#define FloatInvert(x) SkityIEEEFloatDivided(Float1, (x))

static inline bool FloatIsFinite(float x) { return !glm::isinf(x); }

static inline float CrossProduct(glm::vec4 const& a, glm::vec4 const& b) {
  return a.x * b.y - a.y * b.x;
}

static inline float DotProduct(glm::vec4 const& a, glm::vec4 const& b) {
  return a.x * b.x + a.y * b.y;
}

static inline glm::vec2 Times2(glm::vec2 const& value) { return value + value; }

template <class T>
T Interp(T const& v0, T const& v1, T const& t) {
  return v0 + (v1 - v0) * t;
}

}  // namespace skity

#endif  // SKITY_INCLUDE_SKITY_GEOMETRY_MATH_HPP_