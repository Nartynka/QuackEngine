#pragma once

#include <entt.hpp>

#include "Components.h"
#include "Assert.h"

namespace Quack
{
	class Scene;

	class Entity
	{
	public:
		Entity(entt::entity handle, Scene* scene) 
			: handle(handle), scene(scene) {}
		~Entity() = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			QUACK_ASSERT(!HasComponent<T>(), "Entity already has this component!");
			return scene->GetRegistry().emplace<T>(handle, std::forward<Args>(args)...);
		}

		template<typename T>
		bool HasComponent()
		{
			return scene->GetRegistry().any_of<T>(handle);
		}

		template<typename T>
		T& GetComponent()
		{
			QUACK_ASSERT(HasComponent<T>(), "Entity does not have this component!");
			return scene->GetRegistry().get<T>(handle);
		}

		// @TODO: remove component
	private:

		entt::entity handle;
		Scene* scene; // @TODO: think about if there is a better way
	};

}
