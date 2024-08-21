#include "Camera.h"
#include <glad/glad.h>      // GLAD：OpenGL加载器
#include <glm/glm.hpp>                   // GLM：OpenGL数学库
#include <glm/ext.hpp>                   // GLM：OpenGL数学库 扩展
#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展
#include <glm/gtx/transform.hpp>         // GLM：OpenGL数学库 变换
#include <glm/gtc/type_ptr.hpp>          // GLM：OpenGL数学库 指针
#include <glm/gtx/string_cast.hpp>       // GLM：OpenGL数学库 字符串转换
#include <glm/gtc/matrix_transform.hpp>  // GLM：OpenGL数学库 矩阵变换
#include <glm/gtx/perpendicular.hpp>     // GLM：OpenGL数学库 垂直向量
#include "../core/Window.h"
#include "../utils/Log.h"
#include "../core/Input.h"
#include "../core/Clock.h"

using namespace glm;
using namespace core;
namespace component {
    Camera::Camera(Transform* T, View view) :Component() {
        LOG_TRACK;
        mFov = 90.f;
        mNearClip = 0.1f;
        mFarClip = 100.0f;
        mMoveSpeed = 5.0f;
        mZoomSpeed = 0.04f;
        mRotateSpeed = 0.3f;
        mOrbitSpeed = 0.05f;
        mInitPosition = T->mPosition;
        mInitRotation = T->mRotation;
        mpTransform = T;
        mView = view;
    }
    // 获取视图矩阵
    glm::mat4 Camera::GetViewMatrix() const {
        LOG_TRACK;
        // 由于已经有正确的变换矩阵，取其逆矩阵可以直接得到视图矩阵，
        // 然而，这样稍显昂贵，因此我们选择使用`glm::lookAt()`函数，
        // 它只需进行少量的叉乘和点乘运算。
        if constexpr (true) {
            return glm::inverse(mpTransform->mTransform);
        }
        else {
            return glm::lookAt(mpTransform->mPosition, mpTransform->mPosition + mpTransform->mForward, mpTransform->mUp);
        }
    }

    // 获取投影矩阵
    glm::mat4 Camera::GetProjectionMatrix() const {
        LOG_TRACK;
        return (mView == View::Orthgraphic)
            ? glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, mNearClip, mFarClip)  // 正交投影
            : glm::perspective(glm::radians(mFov), Window::mAspectRatio, mNearClip, mFarClip);  // 透视投影
    }

    void Camera::Update()
    {
        float deltatime = Clock::mDeltaTime;
        static bool recovering = false;  // 是否在恢复到初始位置和旋转中

        // 平滑地将相机恢复到初始位置和方向
        if (recovering) {
            float t = Math::EaseFactor(10.0f, deltatime);
            mpTransform->SetPosition(Math::Lerp(mpTransform->mPosition, mInitPosition, t));         // 平滑移动到初始位置
            mpTransform->SetRotation(Math::SlerpRaw(mpTransform->mRotation, mInitRotation, t));     // 平滑旋转到初始方向

            if (Math::Equals(mpTransform->mPosition, mInitPosition) && Math::Equals(mpTransform->mRotation, mInitRotation)) {
                recovering = false;  // 当位置和方向都恢复时停止恢复
            }

            return;  // 在恢复过渡期间忽略所有用户输入
        }

        // 相机轨道：在按住左键的情况下移动鼠标以围绕相机旋转（球形摄像机）
        if (Input::GetMouseDown(MouseButton::Left)) {
            static constexpr auto world_up = glm::vec3(0.0f, 1.0f, 0.0f);

            // 对于球形摄像机模式，水平轨道是绕世界上轴（+Y），垂直轨道是绕局部右向量旋转，
            // 两种旋转都是在世界空间中完成的。
            // 垂直轨道应该被限制在一个范围内，使得相机的欧拉角x永远不会超出度数范围(-90, 90)，
            // 否则世界会被倒置。

            float orbitY = -Input::GetCursorOffset(MouseAxis::Horizontal) * mOrbitSpeed;
            float orbitX = -Input::GetCursorOffset(MouseAxis::Vertical) * mOrbitSpeed;

            // 将 `T->eulerX + orbitX` 限制在 (-89, 89) 范围内
            orbitX = glm::clamp(orbitX, -mpTransform->mEulerX - 89.0f, -mpTransform->mEulerX + 89.0f);

            mpTransform->Rotate(world_up, orbitY, Space::World);     // 绕世界上向量旋转
            mpTransform->Rotate(mpTransform->mRight, orbitX, Space::World);     // 绕局部右向量旋转
            return;  // 在球形摄像机模式下忽略其他事件（按键）
        }

        // 相机缩放：在按住右键的情况下滑动鼠标
        if (Input::GetMouseDown(MouseButton::Right)) {
            mFov -= Input::GetCursorOffset(MouseAxis::Horizontal) * mZoomSpeed;
            mFov = glm::clamp(mFov, 30.0f, 120.0f);  // 限制视角范围在30到120度之间
            return;  // 在平滑缩放期间忽略其他事件（按键和旋转）
        }

        // 一旦释放鼠标按钮，平滑地将视角fov恢复到90度（默认值）
        mFov = Math::Lerp(mFov, 90.0f, Math::EaseFactor(20.0f, deltatime));

        // 只有当没有鼠标按钮事件（不能中断的事件）时，才处理按键事件
        if (Input::GetKeyDown('r')) {
            recovering = true;  // 恢复到初始位置和方向
        }

        // 旋转限制在X和Y轴（只有俯仰和偏航，没有翻滚）
        float eulerY = mpTransform->mEulerY - Input::GetCursorOffset(MouseAxis::Horizontal) * mRotateSpeed;
        float eulerX = mpTransform->mEulerX - Input::GetCursorOffset(MouseAxis::Vertical) * mRotateSpeed;

        eulerY = glm::radians(eulerY);  // 将角度转换为弧度
        eulerX = glm::radians(glm::clamp(eulerX, -89.0f, 89.0f));  // 限制垂直旋转
        glm::quat rotation = glm::quat_cast(glm::eulerAngleYXZ(eulerY, eulerX, 0.0f));

        mpTransform->SetRotation(rotation);

        // 平移（未归一化，移动在对角线方向上更快）
        if (Input::GetKeyDown('w')) {
            mpTransform->Translate(mpTransform->mForward * (mMoveSpeed * deltatime));
        }

        if (Input::GetKeyDown('s')) {
            mpTransform->Translate(-mpTransform->mForward * (mMoveSpeed * deltatime));
        }

        if (Input::GetKeyDown('a')) {
            mpTransform->Translate(-mpTransform->mRight * (mMoveSpeed * deltatime));
        }

        if (Input::GetKeyDown('d')) {
            mpTransform->Translate(mpTransform->mRight * (mMoveSpeed * deltatime));
        }

        if (Input::GetKeyDown('z')) {
            mpTransform->Translate(-mpTransform->mUp * (mMoveSpeed * deltatime));
        }

        if (Input::GetKeyDown(0x20)) {  // VK_SPACE
            mpTransform->Translate(mpTransform->mUp * (mMoveSpeed * deltatime));
        }
    }

}