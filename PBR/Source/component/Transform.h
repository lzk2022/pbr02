#pragma once
#include <GLM/glm.hpp>
#include "../component/Component.h"
#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展
#include <glm/gtx/transform.hpp>         // GLM：OpenGL数学库 变换
namespace component {
    // 表示空间的枚举类型
    enum class Space : char {
        Local = 1 << 0,   // 本地空间
        World = 1 << 1    // 世界空间
    };

    class Transform : public Component {
    public:
        Transform();
        // 平移操作  平移向量
        void Translate(const glm::vec3& vector, Space space = Space::World);
        void Translate(float x, float y, float z, Space space = Space::World);

        // 绕轴旋转操作
        void Rotate(const glm::vec3& axis, float angle, Space space);
        void Rotate(const glm::vec3& eulers, Space space);
        // 使用欧拉角进行旋转操作
        void Rotate(float eulerX, float eulerY, float eulerZ, Space space);

        void Scale(float scale);
        void Scale(const glm::vec3& scale);
        void Scale(float scaleX, float scaleY, float scaleZ);
        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::quat& rotation);
        void SetTransform(const glm::mat4& transform);
        glm::vec3 LocalToWorld(const glm::vec3& v) const;
        glm::vec3 WorldToLocal(const glm::vec3& v) const;
        glm::mat4 GetLocalTransform() const;
        glm::mat4 GetLocalTransform(const glm::vec3& forward, const glm::vec3& up) const;

    private:
        void ReCalBasis(void);  // 重新计算基向量
        void ReCalEuler(void);  // 重新计算欧拉角

    public:
        glm::vec3 mPosition;           // 位置
        glm::quat mRotation;           // 旋转（以四元数表示）
        glm::mat4 mTransform;          // 变换矩阵，以列主序存储

        float mEulerX, mEulerY, mEulerZ;  // 欧拉角（偏航、俯仰、滚转），单位为度
        float mScaleX, mScaleY, mScaleZ;  // 缩放因子

        glm::vec3 mUp;                 // 上向量
        glm::vec3 mForward;            // 前向量
        glm::vec3 mRight;              // 右向量

    };
}


