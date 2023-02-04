#pragma once

#include <d3d11.h>
#include "SimpleMath.h"
#include <algorithm>

#include "Matrix3.h"
#include "Matrix4.h"

// Maybe move to some different header.
constexpr float gf_PI = float(3.14159265358979323846264338327950288419716939937510);
#define DEG2RAD(a) ((a) * (gf_PI / 180.0f))
#define RAD2DEG(a) ((a) * (180.0f / gf_PI))

template<typename T> inline T clamp(T val, T lo, T hi) { return std::min(std::max(val, lo), hi); }

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;

class MathsHelper
{
public:
	static Vector3 Hadamard(const Vector3& lhs, const Vector3& rhs);
	static void CreateOrthonormalBasis(Vector3& x, Vector3& y, Vector3& z);

	static Matrix44 MatrixLookAtLH(const Vector3& eye, const Vector3& at, const Vector3& up);
	static Matrix44 MatrixPerspectiveFovLH(const float fovRadians, const float aspectRatio, const float zn, const float zf);
	template <typename T, typename L, typename R> static T Clamp(const T& value, const L& lhs, const R& rhs);
};

template<typename T, typename L, typename R>
inline T MathsHelper::Clamp(const T & value, const L & lhs, const R & rhs) {
	return value < lhs ? lhs : (value > rhs ? rhs : value);
}
