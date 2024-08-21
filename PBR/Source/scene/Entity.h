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

		// ��ʽת��Ϊbool���ͣ��ж�ʵ���Ƿ���Ч
		explicit operator bool() const { return mId != entt::null; }

		// ��������ģ�庯����������ӵ����������
		 // ArgsΪ�ɱ�ģ����������ڴ��ݸ�������캯���Ĳ���
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		// ��ȡ�����ģ�庯��������ָ�����͵��������
		template<typename T>
		T& GetComponent();

		// ���������ģ�庯�����������ú���������
		// ArgsΪ�ɱ�ģ����������ڴ��ݸ�������캯���Ĳ���
		template<typename T, typename... Args>
		T& SetComponent(Args&&... args);

		// �Ƴ������ģ�庯�������������Ƴ�ʵ���ϵ����
		template<typename T>
		void RemoveComponent();
	private:
		entt::registry* mRegistry = nullptr; // ʵ��������ע���ָ��

	public:
		entt::entity mId = entt::null; // ʵ���ID
		std::string mName; // ʵ�������

	};
}

#include "Entity.hpp"
