#pragma once

#include "Global.h"
#include "Id.h"
#include "Json.h"

namespace eecs
{
	class EntityManager;

	const static uint32 MAX_COMPONENTS = 128;

	/**
	* \brief Base component class only used to specify unique Id for every Component (Id becomes later an index in various vectors)
	*/
	class ComponentBase
	{
	public:
		ComponentBase();
		virtual ~ComponentBase();

		virtual nlohmann::json serialize() = 0;
		virtual void deserialize(const nlohmann::json& json) = 0;

		Id assignedTo_;
	};

	/**
	 * \brief Core component class. User-defined components should inherit from this.
	 * \remarks Default parameterless constructor has to be definied.
	 * \remarks Transform : public Component<Transform> { Transform(); };
	 */
	template<typename T>
	class Component : public ComponentBase
	{
	public:
		virtual ~Component() { }

		//Should be assigned by serializing class
		static uint32 type;

		nlohmann::json serialize() override = 0;
		void deserialize(const nlohmann::json& json) override = 0;
	};

	/**
	 * \brief Wrapper class for Component-type classes. Provides safe access to Components, even after their deletion.
	 * \remarks ComponentHandle<Transform> handle;
	 * \remarks Transform* trans = handle.get(); //can be nullptr but will never cause memory errors
	 * \remarks handle->doSth() //direct access to pointer
	 */
	template<typename T>
	class ComponentHandle
	{
	public:
		ComponentHandle() : manager_(nullptr), id_(99999999, 0) {}

		T* get() const;
		bool isValid() const;

		T* operator->();
		bool operator()() const;
		bool operator==(const ComponentHandle& another) const;
		bool operator<(const ComponentHandle& another) const;
		bool operator!=(const ComponentHandle& another) const;

	private:
		ComponentHandle(EntityManager* manager, Id id) : manager_(manager), id_(id) {}

	private:
		friend class EntityManager;
		EntityManager* manager_;
		Id id_;
	};

	template <typename T>
	uint32 Component<T>::type = 0;

	template<typename T>
	T* ComponentHandle<T>::get() const {
		if (!manager_) return nullptr;
		return manager_->getComponentPtr<T>(id_);
	}

	template<typename T>
	bool ComponentHandle<T>::isValid() const {
		if (manager_ == nullptr) return false;
		return manager_->isValid(id_) && manager_->hasComponent<T>(id_);
	}

	template<typename T>
	T* ComponentHandle<T>::operator->() {
		return get();
	}

	template<typename T>
	bool ComponentHandle<T>::operator()() const {
		return isValid();
	}

	template<typename T>
	bool ComponentHandle<T>::operator==(const ComponentHandle& another) const {
		return id_ == another.id_;
	}

	template<typename T>
	bool ComponentHandle<T>::operator<(const ComponentHandle& another) const {
		return id_ < another.id_;
	}

	template<typename T>
	bool ComponentHandle<T>::operator!=(const ComponentHandle& another) const {
		return !(id_ == another.id_);
	}
}
