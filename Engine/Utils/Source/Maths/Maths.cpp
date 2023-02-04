#include "Maths/Maths.h"

Vector3 MathsHelper::Hadamard(const Vector3 & lhs, const Vector3 & rhs) {
	return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

void MathsHelper::CreateOrthonormalBasis(Vector3& x, Vector3& y, Vector3& z) {
	x.Normalize();
	z = x.Cross(y);
	if (z.LengthSquared() == 0.0f) {
		return;
	}
	z.Normalize();
	y = z.Cross(x);
}

Matrix44 MathsHelper::MatrixLookAtLH(const Vector3& eye, const Vector3& at, const Vector3& up) {
	Matrix44 matResult;

	Vector3 zAxis = at - eye;
	zAxis.Normalize();

	Vector3 xAxis = up.Cross(zAxis);
	xAxis.Normalize();

	Vector3 yAxis = zAxis.Cross(xAxis);

	matResult._11 = xAxis.x;			matResult._12 = yAxis.x;			matResult._13 = zAxis.x;			matResult._14 = 0.0f;
	matResult._21 = xAxis.y;			matResult._22 = yAxis.y;			matResult._23 = zAxis.y;			matResult._24 = 0.0f;
	matResult._31 = xAxis.z;			matResult._32 = yAxis.z;			matResult._33 = zAxis.z;			matResult._34 = 0.0f;
	matResult._41 = -xAxis.Dot(eye);	matResult._42 = -yAxis.Dot(eye);	matResult._43 = -zAxis.Dot(eye);	matResult._44 = 1.0f;

	return matResult;
}

Matrix44 MathsHelper::MatrixPerspectiveFovLH(const float fovRadians, const float aspectRatio, const float zn, const float zf) {
	Matrix44 matResult;

	const float yScale = 1.f / tan((fovRadians / 2.f));
	const float xScale = yScale / aspectRatio;

	matResult._11 = xScale;			matResult._12 = 0.0f;		matResult._13 = 0.0f;				matResult._14 = 0.0f;
	matResult._21 = 0.0f;			matResult._22 = yScale;		matResult._23 = 0.0f;				matResult._24 = 0.0f;
	matResult._31 = 0.0f;			matResult._32 = 0.0f;		matResult._33 = zf / (zf - zn);			matResult._34 = 1.0f;
	matResult._41 = 0.0f;			matResult._42 = 0.0f;		matResult._43 = -zn * zf / (zf - zn);		matResult._44 = 0.0f;

	return matResult;
}