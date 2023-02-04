#include "Metadata.h"
#include "Physics\CharacterController.h"
#include "Physics\Collider.h"
#include "Physics\FixedJoint.h"
#include "Physics\ResolveConstraintsSystem.h"
#include "Physics\Rigidbody.h"
#include "Physics\RigidbodySystem.h"
#include "Physics\Transform.h"
#include "Physics\TransformSystem.h"
#include "Physics\TriggerCollider.h"
#include "Rendering\Camera.h"
#include "Rendering\CylinderLight.h"
#include "Rendering\DirectionalLight.h"
#include "Rendering\PointLight.h"
#include "Rendering\RenderSystem.h"
#include "Rendering\SkinnedMeshRenderer.h"
#include "Rendering\StaticMeshRenderer.h"
//File generated by Flavo Header Tool. DO NOT edit unless you are sure you know what to do
#include "Serializer.h"

reflection::Serializer::Serializer() {
	componentCreationMap_.insert({ "ft_engine::Metadata", &createComponentOfType<ft_engine::Metadata> });
	ft_engine::Metadata::type = 0;
	componentTypeMap_.insert({ "ft_engine::Metadata", 0 });
	typeComponentMap_.insert({ 0, "ft_engine::Metadata" });
	componentCreationMap_.insert({ "ft_engine::CharacterController", &createComponentOfType<ft_engine::CharacterController> });
	ft_engine::CharacterController::type = 1;
	componentTypeMap_.insert({ "ft_engine::CharacterController", 1 });
	typeComponentMap_.insert({ 1, "ft_engine::CharacterController" });
	componentCreationMap_.insert({ "ft_engine::Collider", &createComponentOfType<ft_engine::Collider> });
	ft_engine::Collider::type = 2;
	componentTypeMap_.insert({ "ft_engine::Collider", 2 });
	typeComponentMap_.insert({ 2, "ft_engine::Collider" });
	componentCreationMap_.insert({ "ft_engine::FixedJoint", &createComponentOfType<ft_engine::FixedJoint> });
	ft_engine::FixedJoint::type = 3;
	componentTypeMap_.insert({ "ft_engine::FixedJoint", 3 });
	typeComponentMap_.insert({ 3, "ft_engine::FixedJoint" });
	systemCreationMap_.insert({ "ft_engine::ResolveConstraintsSystem", &createObjectOfType<ft_engine::ResolveConstraintsSystem> });
	componentCreationMap_.insert({ "ft_engine::Rigidbody", &createComponentOfType<ft_engine::Rigidbody> });
	ft_engine::Rigidbody::type = 4;
	componentTypeMap_.insert({ "ft_engine::Rigidbody", 4 });
	typeComponentMap_.insert({ 4, "ft_engine::Rigidbody" });
	systemCreationMap_.insert({ "ft_engine::RigidbodySystem", &createObjectOfType<ft_engine::RigidbodySystem> });
	componentCreationMap_.insert({ "ft_engine::Transform", &createComponentOfType<ft_engine::Transform> });
	ft_engine::Transform::type = 5;
	componentTypeMap_.insert({ "ft_engine::Transform", 5 });
	typeComponentMap_.insert({ 5, "ft_engine::Transform" });
	systemCreationMap_.insert({ "ft_engine::TransformSystem", &createObjectOfType<ft_engine::TransformSystem> });
	componentCreationMap_.insert({ "ft_engine::TriggerCollider", &createComponentOfType<ft_engine::TriggerCollider> });
	ft_engine::TriggerCollider::type = 6;
	componentTypeMap_.insert({ "ft_engine::TriggerCollider", 6 });
	typeComponentMap_.insert({ 6, "ft_engine::TriggerCollider" });
	componentCreationMap_.insert({ "ft_render::Camera", &createComponentOfType<ft_render::Camera> });
	ft_render::Camera::type = 7;
	componentTypeMap_.insert({ "ft_render::Camera", 7 });
	typeComponentMap_.insert({ 7, "ft_render::Camera" });
	componentCreationMap_.insert({ "ft_render::CylinderLight", &createComponentOfType<ft_render::CylinderLight> });
	ft_render::CylinderLight::type = 8;
	componentTypeMap_.insert({ "ft_render::CylinderLight", 8 });
	typeComponentMap_.insert({ 8, "ft_render::CylinderLight" });
	componentCreationMap_.insert({ "ft_render::DirectionalLight", &createComponentOfType<ft_render::DirectionalLight> });
	ft_render::DirectionalLight::type = 9;
	componentTypeMap_.insert({ "ft_render::DirectionalLight", 9 });
	typeComponentMap_.insert({ 9, "ft_render::DirectionalLight" });
	componentCreationMap_.insert({ "ft_render::PointLight", &createComponentOfType<ft_render::PointLight> });
	ft_render::PointLight::type = 10;
	componentTypeMap_.insert({ "ft_render::PointLight", 10 });
	typeComponentMap_.insert({ 10, "ft_render::PointLight" });
	systemCreationMap_.insert({ "ft_render::RenderSystem", &createObjectOfType<ft_render::RenderSystem> });
	componentCreationMap_.insert({ "ft_render::SkinnedMeshRenderer", &createComponentOfType<ft_render::SkinnedMeshRenderer> });
	ft_render::SkinnedMeshRenderer::type = 11;
	componentTypeMap_.insert({ "ft_render::SkinnedMeshRenderer", 11 });
	typeComponentMap_.insert({ 11, "ft_render::SkinnedMeshRenderer" });
	componentCreationMap_.insert({ "ft_render::StaticMeshRenderer", &createComponentOfType<ft_render::StaticMeshRenderer> });
	ft_render::StaticMeshRenderer::type = 12;
	componentTypeMap_.insert({ "ft_render::StaticMeshRenderer", 12 });
	typeComponentMap_.insert({ 12, "ft_render::StaticMeshRenderer" });
} //Serializer()

reflection::Serializer::~Serializer() { }

const std::unordered_map<std::string, CreateObjectOfTypeFunc>& reflection::Serializer::getSystemCreationMap() const {
	return systemCreationMap_;
}

const std::unordered_map<std::string, CreateComponentOfTypeFunc>& reflection::Serializer::getComponentCreationMap() const {
	return componentCreationMap_;
}

const std::unordered_map<std::string, uint32>& reflection::Serializer::getComponentTypeMap() const {
	return componentTypeMap_;
}

const std::unordered_map<uint32, std::string>& reflection::Serializer::getTypeComponentMap() const {
	return typeComponentMap_;
}

void reflection::Serializer::initRootComponent(eecs::Entity entity) const {
	entity.addComponent<ft_engine::Metadata>();
	entity.addComponent<ft_engine::CharacterController>();
	entity.addComponent<ft_engine::Collider>();
	entity.addComponent<ft_engine::FixedJoint>();
	entity.addComponent<ft_engine::Rigidbody>();
	entity.addComponent<ft_engine::Transform>();
	entity.addComponent<ft_engine::TriggerCollider>();
	entity.addComponent<ft_render::Camera>();
	entity.addComponent<ft_render::CylinderLight>();
	entity.addComponent<ft_render::DirectionalLight>();
	entity.addComponent<ft_render::PointLight>();
	entity.addComponent<ft_render::SkinnedMeshRenderer>();
	entity.addComponent<ft_render::StaticMeshRenderer>();
}

template<> eecs::ComponentBase* createComponentOfType<ft_engine::Metadata>() { return new ft_engine::Metadata(); }
template<> eecs::ComponentBase* createComponentOfType<ft_engine::CharacterController>() { return new ft_engine::CharacterController(); }
template<> eecs::ComponentBase* createComponentOfType<ft_engine::Collider>() { return new ft_engine::Collider(); }
template<> eecs::ComponentBase* createComponentOfType<ft_engine::FixedJoint>() { return new ft_engine::FixedJoint(); }
template<> eecs::SystemBase* createObjectOfType<ft_engine::ResolveConstraintsSystem>() { return new ft_engine::ResolveConstraintsSystem(); }
template<> eecs::ComponentBase* createComponentOfType<ft_engine::Rigidbody>() { return new ft_engine::Rigidbody(); }
template<> eecs::SystemBase* createObjectOfType<ft_engine::RigidbodySystem>() { return new ft_engine::RigidbodySystem(); }
template<> eecs::ComponentBase* createComponentOfType<ft_engine::Transform>() { return new ft_engine::Transform(); }
template<> eecs::SystemBase* createObjectOfType<ft_engine::TransformSystem>() { return new ft_engine::TransformSystem(); }
template<> eecs::ComponentBase* createComponentOfType<ft_engine::TriggerCollider>() { return new ft_engine::TriggerCollider(); }
template<> eecs::ComponentBase* createComponentOfType<ft_render::Camera>() { return new ft_render::Camera(); }
template<> eecs::ComponentBase* createComponentOfType<ft_render::CylinderLight>() { return new ft_render::CylinderLight(); }
template<> eecs::ComponentBase* createComponentOfType<ft_render::DirectionalLight>() { return new ft_render::DirectionalLight(); }
template<> eecs::ComponentBase* createComponentOfType<ft_render::PointLight>() { return new ft_render::PointLight(); }
template<> eecs::SystemBase* createObjectOfType<ft_render::RenderSystem>() { return new ft_render::RenderSystem(); }
template<> eecs::ComponentBase* createComponentOfType<ft_render::SkinnedMeshRenderer>() { return new ft_render::SkinnedMeshRenderer(); }
template<> eecs::ComponentBase* createComponentOfType<ft_render::StaticMeshRenderer>() { return new ft_render::StaticMeshRenderer(); }
