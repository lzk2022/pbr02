#pragma once
#include <ecs/entt.hpp>
namespace scene {
	class Entity {
	public:
		Entity() = default;
		Entity(const std::string& name, entt::entity id, entt::registry* reg) : mName(name), mId(id), mRegistry(reg) {}
		~Entity() {};
		Entity(const Entity&) = default;
		Entity& operator=(const Entity&) = default;

		// 显式转换为bool类型，判断实体是否有效
		explicit operator bool() const { return mId != entt::null; }

		// 添加组件的模板函数，返回添加的组件的引用
		 // Args为可变模板参数，用于传递给组件构造函数的参数
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		// 获取组件的模板函数，返回指定类型的组件引用
		template<typename T>
		T& GetComponent();

		// 设置组件的模板函数，返回设置后的组件引用
		// Args为可变模板参数，用于传递给组件构造函数的参数
		template<typename T, typename... Args>
		T& SetComponent(Args&&... args);

		// 移除组件的模板函数，根据类型移除实体上的组件
		template<typename T>
		void RemoveComponent();
	private:
		entt::registry* mRegistry = nullptr; // 实体所属的注册表指针

	public:
		entt::entity mId = entt::null; // 实体的ID
		std::string mName; // 实体的名称

	};
}

#include "Entity.hpp"
