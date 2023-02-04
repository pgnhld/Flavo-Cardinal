#include "Maths/Matrix4.h"

Matrix44 Matrix44::Inverse() const {
	Matrix result;
	result = DirectX::XMMatrixInverse(nullptr, toXMMATRIX());
	return Matrix44(result);
}

Vector4 Matrix44::operator*(const Vector4& inVector) const {
	return DirectX::XMVector4Transform(inVector, toXMMATRIX());
}