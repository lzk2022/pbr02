#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展
#include "../component/Component.h"
#include "Transform.h"
namespace component {
    enum class View : char {
        Orthgraphic = 1 << 0,  // 正交视图的标记
        Perspective = 1 << 1   // 透视视图的标记
    };

    class Camera : public Component {
    public:
        float mFov;           // 视场角
        float mNearClip;     // 近裁剪面
        float mFarClip;      // 远裁剪面
        float mMoveSpeed;    // 移动速度
        float mZoomSpeed;    // 缩放速度
        float mRotateSpeed;  // 旋转速度
        float mOrbitSpeed;   // 轨道速度

        glm::vec3 mInitPosition;   // 初始位置
        glm::quat mInitRotation;   // 初始旋转四元数

        Transform* mpTransform;  // 指向 Transform 类的指针，部分由 ECS 组管理，所以该指针可靠
        View mView;     // 视图类型

    public:
        // 构造函数，初始化相机组件
        // 参数：
        //   T: Transform 指针，相机的位置和方向
        //   view: 视图类型，默认为透视视图
        Camera(Transform* T, View view = View::Perspective);

        // 获取视图矩阵
        glm::mat4 GetViewMatrix() const;

        // 获取投影矩阵
        glm::mat4 GetProjectionMatrix() const;

        // 更新相机状态
        void Update();
    };
}
