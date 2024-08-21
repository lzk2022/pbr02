#include "../component/Camera.h"
#include <type_traits>
#include "../utils/Log.h"
#include "../utils/Ext.h"
#include "../component/Model.h"
namespace scene {
    template<typename T, typename... Args>
    inline T& Entity::AddComponent(Args&&... args) {
        LOG_ASSERT(!mRegistry->all_of<T>(mId), "{0} �Ѿ������ {1}��", mName, utils::type_name<T>());
        using namespace component;

        // �����Camera�������Ҫ��ȡTransform��������ݸ�Camera���캯��
        if constexpr (std::is_same_v<T, Camera>) {
            auto& transform = mRegistry->get<Transform>(mId);
            return mRegistry->emplace<T>(mId, &transform, std::forward<Args>(args)...);
        }
        // �����Animator�������Ҫ��ȡModel��������ݸ�Animator���캯��
        else if constexpr (std::is_same_v<T, Animator>) {
            auto& model = mRegistry->get<Model>(mId);
            return mRegistry->emplace<T>(mId, &model, std::forward<Args>(args)...);
        }
        // �������͵����ֱ�����
        else {
            return mRegistry->emplace<T>(mId, std::forward<Args>(args)...);
        }
    }


    template<typename T>
    inline T& Entity::GetComponent() {
        LOG_ASSERT(mRegistry->all_of<T>(mId), "�� {0} ��δ�ҵ���� {1}��", mName, utils::type_name<T>());
        return mRegistry->get<T>(mId);
    }


    template<typename T, typename... Args>
    inline T& Entity::SetComponent(Args&&... args) {
        return mRegistry->emplace_or_replace<T>(mId, std::forward<Args>(args)...);
    }

    template<typename T>
    inline void Entity::RemoveComponent() {
        mRegistry->remove<T>(mId);
    }
}



