#include "Maths/Matrix3.h"

Matrix3 Matrix3::AddDiagonal(float scalar) const {
	return Matrix3(
		_11 + scalar,
		_12,
		_13,
		_21,
		_22 + scalar,
		_23,
		_31,
		_32,
		_33 + scalar
	);
}

Matrix3 Matrix3::Inverse() const {
	Matrix result;
	result = DirectX::XMMatrixInverse(nullptr, toXMMATRIX());
	return Matrix3(result);
}

Matrix3 Matrix3::Transpose() const {
	return Matrix3(
		_11,
		_21,
		_31,
		_12,
		_22,
		_32,
		_13,
		_23,
		_33
	);
}

Matrix3 Matrix3::operator*(const Matrix3 & rhs) const {
	float t[9];

	t[0] = _11 * rhs._11 + _21 * rhs._12 + _31 * rhs._13;
	t[1] = _11 * rhs._21 + _21 * rhs._22 + _31 * rhs._23;
	t[2] = _11 * rhs._31 + _21 * rhs._32 + _31 * rhs._33;

	t[3] = _12 * rhs._11 + _22 * rhs._12 + _32 * rhs._13;
	t[4] = _12 * rhs._21 + _22 * rhs._22 + _32 * rhs._23;
	t[5] = _12 * rhs._31 + _22 * rhs._32 + _32 * rhs._33;

	t[6] = _13 * rhs._11 + _23 * rhs._12 + _33 * rhs._13;
	t[7] = _13 * rhs._21 + _23 * rhs._22 + _33 * rhs._23;
	t[8] = _13 * rhs._31 + _23 * rhs._32 + _33 * rhs._33;

	return Matrix3(
		t[0],
		t[3],
		t[6],
		t[1],
		t[4],
		t[7],
		t[2],
		t[5],
		t[8]
	);
}

Matrix3 Matrix3::operator*(float scalar) const {
	return Matrix3(
		_11 * scalar,
		_12 * scalar,
		_13 * scalar,
		_21 * scalar,
		_22 * scalar,
		_23 * scalar,
		_31 * scalar,
		_32 * scalar,
		_33 * scalar
	);
}

Vector3 Matrix3::operator*(const Vector3& inVector) const {
	return DirectX::XMVector3Transform(inVector, toXMMATRIX());
}