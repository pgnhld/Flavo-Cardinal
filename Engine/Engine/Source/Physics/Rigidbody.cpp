#include "Physics/Rigidbody.h"
#include "SceneManager.h"
#include "FTime.h"
#include "Logger.h"

ft_engine::Rigidbody::Rigidbody() {

}

void ft_engine::Rigidbody::addForce(const Vector3& value, EForceMode forceMode) {
	switch (forceMode) {
	case EForceMode::Acceleration:
		accumulatedForce += value * mass;
		break;
	case EForceMode::Force:
		accumulatedForce += value;
		break;
	case EForceMode::Impulse:
		accumulatedForce += value / framework::FTime::fixedDeltaTime;
		break;
	case EForceMode::VelocityChange:
		velocity += value;
		break;
	}
}

void ft_engine::Rigidbody::addForceAtPosition(Vector3 value, const Vector3& positionRelativeToPivot, EForceMode forceMode) {
	switch (forceMode) {
	case EForceMode::Acceleration:
		value *= mass;
		break;
	case EForceMode::Force:
		break;
	case EForceMode::Impulse:
		value /= framework::FTime::fixedDeltaTime;
		break;
	case EForceMode::VelocityChange:
		velocity += value;
		return;
	}
	accumulatedForce += value;
	accumulatedTorque += value.Cross(positionRelativeToPivot);
}

void ft_engine::Rigidbody::setInverseInertiaTensor(const Matrix3& inertiaTensor) {
	inverseInertiaTensor = std::move(inertiaTensor.Inverse());
}

nlohmann::json ft_engine::Rigidbody::serialize() {
	return {
		{ "mass", mass },
		{ "drag", drag },
		{ "angularDrag", angularDrag },
		{ "constraints", constraints}
	};
}

void ft_engine::Rigidbody::deserialize(const nlohmann::json& json) {
	mass = json.at("mass");
	drag = json.at("drag");
	angularDrag = json.at("angularDrag");
	constraints = json.at("constraints").get<RigidbodyConstraints>();
}

void ft_engine::to_json(nlohmann::json& json, const RigidbodyConstraints& obj) {
	json = nlohmann::json{
		{ "value", obj.constraints }
	};
}

void ft_engine::from_json(const nlohmann::json& json, RigidbodyConstraints& obj) {
	obj.constraints = json.at("value");
}


constexpr uint8_t ft_engine::RigidbodyConstraints::constraintToUint(ERigidbodyConstraints constraint) {
	return static_cast<uint8_t>(constraint);
}

/// return Vector(1,1,1) when object has 3 degrees of linear freedom, Vector3(0,0,0) when 0 etc.
Vector3 ft_engine::RigidbodyConstraints::linearMovementConstraints() const {
	return Vector3
	(
		(int)((1 << 0) & constraints) == 0,
		(int)((1 << 1) & constraints) == 0,
		(int)((1 << 2) & constraints) == 0
	);
}

/// return Vector(1,1,1) when object has 3 degrees of angular freedom, Vector3(0,0,0) when 0 etc.
Vector3 ft_engine::RigidbodyConstraints::angularMovementConstraints() const {
	return Vector3
	(
		(int)((1 << 3) & constraints) == 0,
		(int)((1 << 4) & constraints) == 0,
		(int)((1 << 5) & constraints) == 0
	);
}

bool ft_engine::RigidbodyConstraints::contains(ERigidbodyConstraints constraint) const {
	const uint8_t constraintUint = constraintToUint(constraint);
	return (constraintUint & constraints) == constraintUint;
}
