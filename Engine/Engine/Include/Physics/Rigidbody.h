#pragma once

#include <Global.h>
#include "Maths/Maths.h"
#include "EECS.h"

FLAVO_COMPONENT(ft_engine, Rigidbody)
namespace ft_engine
{
	using namespace utils;

	enum class EForceMode
	{
		Acceleration = 0,
		Force = 1,
		Impulse = 2,
		VelocityChange = 3
	};

	enum class ERigidbodyConstraints
	{
		Free = 0,
		FreezePositionX = 1,
		FreezePositionY = 2,
		FreezePositionZ = 4,
		FreezePosition = 7,
		FreezeRotationX = 8,
		FreezeRotationY = 16,
		FreezeRotationZ = 32,
		FreezeRotation = 56,
		Freeze = 63,
		FreezeGravity = 64,
		FreezeAll = 127,
		FixedJoint = 128
	};

	struct RigidbodyConstraints
	{
		static constexpr uint8_t constraintToUint(ERigidbodyConstraints constraint);

		Vector3 linearMovementConstraints() const;
		Vector3 angularMovementConstraints() const;
		bool contains(ERigidbodyConstraints constraint) const;

		uint8_t constraints;
	};

	void to_json(nlohmann::json& json, const RigidbodyConstraints& obj);
	void from_json(const nlohmann::json& json, RigidbodyConstraints& obj);

	class Rigidbody : public eecs::Component<Rigidbody>, public eecs::IInvoker
	{
	public:
		Rigidbody();

		void addForce(const Vector3& value, EForceMode forceMode = EForceMode::Force);
		void addForceAtPosition(Vector3 value, const Vector3& positionRelativeToPivot, EForceMode forceMode = EForceMode::Force);
		void setInverseInertiaTensor(const Matrix3& inertiaTensor);

		Vector3 velocity;
		Vector3 angularMomentum;

		Vector3 accumulatedForce;
		Vector3 accumulatedTorque;

		Vector3 lastAccumulatedForce;

		Matrix3 inverseInertiaTensor = Matrix3(0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.2f);
		float mass;

		float drag;
		float angularDrag;

		RigidbodyConstraints constraints;

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;
	};
}