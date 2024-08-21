#include "../component/Camera.h"
#include <type_traits>
#include "../utils/Log.h"
#include "../utils/Ext.h"
#include "../component/Model.h"
namespace scene {
    template<typename T, typename... Args>
    inline T& Entity::AddComponent(Args&&... args) {
        LOG_ASSERT(!mRegistry->all_of<T>(mId), "{0} 已经有组件 {1}！", mName, utils::type_name<T>());
        using namespace component;

        // 如果是Camera组件，需要获取Transform组件并传递给Camera构造函数
        if constexpr (std::is_same_v<T, Camera>) {
            auto& transform = mRegistry->get<Transform>(mId);
            return mRegistry->emplace<T>(mId, &transform, std::forward<Args>(args)...);
        }
        // 如果是Animator组件，需要获取Model组件并传递给Animator构造函数
        else if constexpr (std::is_same_v<T, Animator>) {
            auto& model = mRegistry->get<Model>(mId);
            return mRegistry->emplace<T>(mId, &model, std::forward<Args>(args)...);
        }
        // 其他类型的组件直接添加
        else {
            return mRegistry->emplace<T>(mId, std::forward<Args>(args)...);
        }
    }


    template<typename T>
    inline T& Entity::GetComponent() {
        LOG_ASSERT(mRegistry->all_of<T>(mId), "在 {0} 中未找到组件 {1}！", mName, utils::type_name<T>());
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



