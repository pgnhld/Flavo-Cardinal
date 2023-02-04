#pragma once

#include <Global.h>
#include <d3d11.h>
#include <SimpleMath.h>

using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Matrix;
using DirectX::XMFLOAT4X4;
using DirectX::XMMATRIX;

struct Matrix44 : public XMFLOAT4X4
{
	Matrix44() : XMFLOAT4X4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f) {
	}
	Matrix44(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) : XMFLOAT4X4(m00, m01, m02, m03,
			m10, m11, m12, m22,
			m20, m21, m22, m23,
			m30, m31, m32, m33) {
	}
	explicit Matrix44(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3) : XMFLOAT4X4(r0.x, r0.y, r0.z, r0.w,
		r1.x, r1.y, r1.z, r1.w,
		r2.x, r2.y, r2.z, r2.w,
		r3.x, r3.y, r3.z, r3.w) {
	}
	Matrix44(const Matrix& m) : XMFLOAT4X4(m._11, m._12, m._13, m._14,
		m._21, m._22, m._23, m._24,
		m._31, m._32, m._33, m._34,
		m._41, m._42, m._43, m._44) {
	}

	Matrix44(const XMFLOAT4X4& M) { memcpy_s(this, sizeof(float) * 16, &M, sizeof(XMFLOAT4X4)); }

	Matrix44 Inverse() const;

	operator XMMATRIX() const { return XMLoadFloat4x4(this); }
	XMMATRIX toXMMATRIX() const { return DirectX::XMLoadFloat4x4(this); }

	// Unary operator
	Vector4 operator* (const Vector4& inVector) const;
};