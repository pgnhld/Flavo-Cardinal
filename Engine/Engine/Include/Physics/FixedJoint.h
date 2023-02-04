#pragma once

#include "Global.h"
#include "EECS.h"
#include "Maths/Maths.h"
#include "CoroutineManager.h"

FLAVO_COMPONENT(ft_engine, FixedJoint)
namespace ft_engine
{
	class FixedJoint : public eecs::Component<FixedJoint>
	{
	public:
		bool bEnabled = true;

		bool bCarried = false;
		Entity carryingEntity;

		/* Fixed joint params */
		//Set by RigidbodySystem if joint should be broken
		bool bBroken = false;
		//Usually it should be camera looking direction or at least carrying Entity looking direction
		Matrix lookWorldMatrix;
		//Whether object has been pulled to target offset via Coroutine
		bool bHandledToDestination = false;
		//Has Look Matrix been assigned
		bool bLookMatrixAssigned = false;
		//Range [-1.0f, 1.0f]
		const float minimumRelativeOffsetY = -0.2f;
		//Left/Up/Forward
		Vector3 localTargetOffset = Vector3(0.0f, 0.5f, 1.5f);
		//Maximum allowed object offset from target position after initial pull
		float maximumJointRange = 0.2f;
		//Pulling time
		double pullTime = 0.8;
		//Defines which world Vectors are used to create box's world rotation
		int worldVectorIndices[2];
		//Defines signs of world Vectors
		float worldVectorSigns[2];
		//Pulling Coroutine
		Coroutine pullCoroutine;

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;
	};
}
