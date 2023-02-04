#include "DxtkString.h"
#include "Assertion.h"
#include "Logger.h"
#include "Maths/Maths.h"

void DirectX::to_json(nlohmann::json& json, const XMFLOAT2& obj) {
	LOG_R("to_json with XMFLOAT2; It should never be visible");
}

void DirectX::from_json(const nlohmann::json& json, XMFLOAT2& obj) {
	LOG_R("from_json with XMFLOAT2; It should never be visible");
}

void DirectX::to_json(nlohmann::json& json, const XMFLOAT3& obj) {
	LOG_R("to_json with XMFLOAT3; It should never be visible");
}

void DirectX::from_json(const nlohmann::json& json, XMFLOAT3& obj) {
	LOG_R("from_json with XMFLOAT3; It should never be visible");
}

void DirectX::SimpleMath::to_json(nlohmann::json& json, const Vector2& obj) {
	json = nlohmann::json{
		{ "x", obj.x },
		{ "y", obj.y },
	};
}

void DirectX::SimpleMath::from_json(const nlohmann::json& json, Vector2& obj) {
	obj.x = json.at("x");
	obj.y = json.at("y");
}

void DirectX::SimpleMath::to_json(nlohmann::json& json, const Vector3& obj) {
	json = nlohmann::json{
		{ "x", obj.x },
		{ "y", obj.y },
		{ "z", obj.z }
	};
}

void DirectX::SimpleMath::from_json(const nlohmann::json& json, Vector3& obj) {
	obj.x = json.at("x");
	obj.y = json.at("y");
	obj.z = json.at("z");
}

void DirectX::SimpleMath::to_json(nlohmann::json& json, const Color& obj) {
	json = {
		{ "red", obj.R() },
		{ "green", obj.G() },
		{ "blue", obj.B() },
		{ "alpha", obj.A() }
	};
}

void DirectX::SimpleMath::from_json(const nlohmann::json& json, Color& obj) {
	obj.R(json.at("red"));
	obj.G(json.at("green"));
	obj.B(json.at("blue"));
	obj.A(json.at("alpha"));
}

void DirectX::SimpleMath::to_json(nlohmann::json& json, const Quaternion& obj) {
	const Vector3 euler = obj.Euler();
	json = nlohmann::json{
		{ "x", RAD2DEG(euler.x) },
		{ "y", RAD2DEG(euler.y) },
		{ "z", RAD2DEG(euler.z) }
	};
}

void DirectX::SimpleMath::from_json(const nlohmann::json& json, Quaternion& obj) {
	obj = Quaternion::CreateFromYawPitchRoll(
		DEG2RAD(static_cast<float>(json.at("y"))), 
		DEG2RAD(static_cast<float>(json.at("x"))),
		DEG2RAD(static_cast<float>(json.at("z"))));
}

void DirectX::SimpleMath::to_json(nlohmann::json& json, const Matrix& obj) {
	Vector3 pos;
	Vector3 scale;
	Quaternion rot;
	ASSERT_FAIL(obj.Decompose(scale, rot, pos), "Couldn't decompose matrix");
	json = nlohmann::json{
		{ "position", pos },
		{ "rotation", rot },
		{ "scale", scale }
	};
}

void DirectX::SimpleMath::from_json(const nlohmann::json& json, Matrix& obj) {
	obj = obj.Compose(json.at("position").get<Vector3>(), json.at("rotation").get<Quaternion>(), json.at("scale").get<Vector3>());
}

void DirectX::to_json(nlohmann::json& json, const BoundingBox& obj) {
	json = nlohmann::json{
		{ "center", Vector3(obj.Center) },
		{ "extent", Vector3(obj.Extents) }
	};
}

void DirectX::from_json(const nlohmann::json& json, BoundingBox& obj) {
	obj.Center = json.at("center").get<Vector3>();
	obj.Extents = json.at("extent").get<Vector3>();
}
