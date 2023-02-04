#pragma once
#include <d3d11.h>
#include "../../ThirdParty/DirectXTK/Inc/SimpleMath.h"
#include "Json.h"

namespace DirectX
{
	/* Base DX structs serialization; It should never get called but is required by json serializer */
	void to_json(nlohmann::json& json, const XMFLOAT2& obj);
	void from_json(const nlohmann::json& json, XMFLOAT2& obj);
	void to_json(nlohmann::json& json, const XMFLOAT3& obj);
	void from_json(const nlohmann::json& json, XMFLOAT3& obj);

	namespace SimpleMath
	{
		void to_json(nlohmann::json& json, const Vector2& obj);
		void from_json(const nlohmann::json& json, Vector2& obj);

		void to_json(nlohmann::json& json, const Vector3& obj);
		void from_json(const nlohmann::json& json, Vector3& obj);

		void to_json(nlohmann::json& json, const Color& obj);
		void from_json(const nlohmann::json& json, Color& obj);

		void to_json(nlohmann::json& json, const Quaternion& obj);
		void from_json(const nlohmann::json& json, Quaternion& obj);

		void to_json(nlohmann::json& json, const Matrix& obj);
		void from_json(const nlohmann::json& json, Matrix& obj);
	}

	void to_json(nlohmann::json& json, const BoundingBox& obj);
	void from_json(const nlohmann::json& json, BoundingBox& obj);
}

using namespace DirectX::SimpleMath;

inline std::ostream& operator<<(std::ostream& out, Matrix& matrix) {
	nlohmann::json j;
	to_json(j, matrix);
	out << j;
	return out;
}

inline std::ostream& operator<<(std::ostream& out, const Vector3& vector) {
	nlohmann::json j;
	to_json(j, vector);
	out << j;
	return out;
}

inline std::ostream& operator<<(std::ostream& out, const Quaternion& quaternion) {
	nlohmann::json j;
	to_json(j, quaternion);
	out << j;
	return out;
}

inline std::ostream& operator<<(std::ostream& out, const DirectX::BoundingBox& boundingBox) {
	nlohmann::json j;
	to_json(j, boundingBox);
	out << j;
	return out;
}