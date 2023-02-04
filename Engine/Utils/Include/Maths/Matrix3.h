#pragma once

#include <Global.h>
#include <d3d11.h>
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Matrix;
using DirectX::XMFLOAT3X3;
using DirectX::XMMATRIX;

struct Matrix3 : public XMFLOAT3X3
{
	Matrix3() : XMFLOAT3X3(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.f) {
	}
	Matrix3(float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22) : XMFLOAT3X3(m00, m01, m02,
			m10, m11, m12,
			m20, m21, m22) {
	}
	explicit Matrix3(const Vector3& r0, const Vector3& r1, const Vector3& r2) : XMFLOAT3X3(r0.x, r0.y, r0.z,
		r1.x, r1.y, r1.z,
		r2.x, r2.y, r2.z) {
	}
	Matrix3(const Matrix& m) : XMFLOAT3X3(m._11, m._12, m._13,
		m._21, m._22, m._23,
		m._31, m._32, m._33) {
	}

	Matrix3(const XMFLOAT3X3& M) { memcpy_s(this, sizeof(float) * 9, &M, sizeof(XMFLOAT3X3)); }

	Matrix3 AddDiagonal(float scalar) const;

	Matrix3 Inverse() const;
	Matrix3 Transpose() const;

	operator XMMATRIX() const { return XMLoadFloat3x3(this); }
	XMMATRIX toXMMATRIX() const { return DirectX::XMLoadFloat3x3(this); }

	Matrix3 operator*(const Matrix3& rhs) const;
	Matrix3 operator*(float scalar) const;
	Vector3 operator* (const Vector3& inVector) const;
};