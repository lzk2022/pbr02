#include "Transform.h"
#include <GLM/gtx/euler_angles.hpp>
#include "../utils/Log.h"
namespace component {
    static constexpr glm::vec3 gOrigin{ 0.0f };           // 原点
    static constexpr glm::mat4 gIdentity{ 1.0f };           // 单位矩阵
    static constexpr glm::quat gEye{ 1.0f, 0.0f, 0.0f, 0.0f };  // 单位四元数（wxyz顺序）
    static constexpr glm::vec3 gWorldRight{ 1.0f, 0.0f, 0.0f };  // 世界坐标系的+x轴方向
    static constexpr glm::vec3 gWorldUp{ 0.0f, 1.0f, 0.0f };  // 世界坐标系的+y轴方向
    static constexpr glm::vec3 gWorldForward{ 0.0f, 0.0f,-1.0f };  // 世界坐标系的-z轴方向

    Transform::Transform() :Component(){
        LOG_TRACK;
        mTransform = gIdentity;
        mPosition = gOrigin;
        mRotation = gEye;
        mRight = gWorldRight;
        mUp = gWorldUp;
        mForward = gWorldForward;
        mEulerX = 0.0f;
        mEulerY = 0.0f;
        mEulerZ = 0.0f;
        mScaleX = 1.0f;
        mScaleY = 1.0f;
        mScaleZ = 1.0f;
    }

    void Transform::Translate(const glm::vec3& vector, Space space){
        LOG_TRACK;
        // 按局部空间进行平移：期望向量是局部坐标系中的坐标
        if (space == Space::Local) {
            glm::vec3 world = LocalToWorld(vector);             // 将局部坐标系转换为世界坐标系
            this->mPosition += world;                           // 更新位置
            this->mTransform[3] = glm::vec4(mPosition, 1.0f);   // 更新变换矩阵的平移部分
        }
        // 按世界空间进行平移：直接根据向量更新位置
        else {
            this->mPosition += vector;                              // 更新位置
            this->mTransform[3] = glm::vec4(mPosition, 1.0f);       // 更新变换矩阵的平移部分
        }
	}
    void Transform::Translate(float x, float y, float z, Space space){
        Translate(glm::vec3(x, y, z), space);                       // 调用重载的glm::vec3向量版本的Translate函数
    }
    void Transform::Rotate(const glm::vec3& axis, float angle, Space space){
        LOG_TRACK;
        float radians = glm::radians(angle);        // 将角度转换为弧度
        glm::vec3 v = glm::normalize(axis);         // 规范化旋转轴向

        glm::mat4 R = glm::rotate(radians, v);      // 旋转矩阵4x4
        glm::quat Q = glm::angleAxis(radians, v);   // 旋转四元数

        // 按局部空间进行旋转：期望轴向是局部坐标系中的轴向，例如 right = glm::vec3(1, 0, 0)
        if (space == Space::Local) {
            this->mTransform = this->mTransform * R;                // 更新变换矩阵
            this->mRotation = glm::normalize(this->mRotation * Q);  // 更新旋转四元数
        }
        // 按世界空间进行旋转：期望轴向是世界坐标系中的轴向，可能会引入平移
        else {
            this->mTransform = R * this->mTransform;                // 更新变换矩阵
            this->mRotation = glm::normalize(Q * this->mRotation);  // 更新旋转四元数
            this->mPosition = glm::vec3(this->mTransform[3]);       // 更新位置
        }

        ReCalEuler();   // 重新计算欧拉角
        ReCalBasis();   // 重新计算基向量
    }

    void Transform::Rotate(const glm::vec3& eulers, Space space){
        LOG_TRACK;
        glm::vec3 radians = glm::radians(eulers);  // 将欧拉角转换为弧度

        // 旋转矩阵4x4
        glm::mat4 RX = glm::rotate(radians.x, gWorldRight);
        glm::mat4 RY = glm::rotate(radians.y, gWorldUp);
        glm::mat4 RZ = glm::rotate(radians.z, gWorldForward);
        glm::mat4 R = RZ * RX * RY;  // Y->X->Z

        // 旋转四元数
        glm::quat QX = glm::angleAxis(radians.x, gWorldRight);
        glm::quat QY = glm::angleAxis(radians.y, gWorldUp);
        glm::quat QZ = glm::angleAxis(radians.z, gWorldForward);
        glm::quat Q = QZ * QX * QY;  // Y->X->Z

        // 按局部空间进行旋转：欧拉角相对于局部基向量进行旋转
        if (space == Space::Local) {
            this->mTransform = this->mTransform * R;              // 更新变换矩阵
            this->mRotation = glm::normalize(this->mRotation * Q);  // 更新旋转四元数
        }
        // 按世界空间进行旋转：欧拉角相对于世界基向量 X、Y 和 -Z 进行旋转
        else {
            this->mTransform = R * this->mTransform;              // 更新变换矩阵
            this->mRotation = glm::normalize(Q * this->mRotation);  // 更新旋转四元数
            this->mPosition = glm::vec3(this->mTransform[3]);     // 更新位置
        }

        ReCalEuler();   // 重新计算欧拉角
        ReCalBasis();   // 重新计算基向量
    }

    void Transform::Rotate(float eulerX, float eulerY, float eulerZ, Space space)
    {
        Rotate(glm::vec3(eulerX, eulerY, eulerZ), space);  // 调用按欧拉角进行旋转的函数
    }

    void Transform::Scale(float scale) {
        LOG_TRACK;
        mTransform = glm::scale(mTransform, glm::vec3(scale));  // 更新变换矩阵
        mScaleX *= scale;   // 更新 X 轴缩放因子
        mScaleY *= scale;   // 更新 Y 轴缩放因子
        mScaleZ *= scale;   // 更新 Z 轴缩放因子
    }
    void Transform::Scale(const glm::vec3& scale) {
        LOG_TRACK;
        mTransform = glm::scale(mTransform, scale);  // 更新变换矩阵
        mScaleX *= scale.x;   // 更新 X 轴缩放因子
        mScaleY *= scale.y;   // 更新 Y 轴缩放因子
        mScaleZ *= scale.z;   // 更新 Z 轴缩放因子
    }
    void Transform::Scale(float scaleX, float scaleY, float scaleZ) {
        LOG_TRACK;
        Scale(glm::vec3(scaleX, scaleY, scaleZ));  // 调用按各轴向量进行缩放的函数
    }
    void Transform::SetPosition(const glm::vec3& position) {
        LOG_TRACK;
        mPosition = position;                      // 更新位置
        mTransform[3] = glm::vec4(position, 1.0f);  // 更新变换矩阵中的位置分量
    }

    void Transform::SetRotation(const glm::quat& rotation) {
        LOG_TRACK;
        /* 4x4 变换矩阵以列主序存储，如下所示，其中
        平移和缩放分量在 X、Y 和 Z 轴上分别用 T 和 S 表示，
        旋转分量由 R、U 和 F 表示，分别代表右向、上向和前向

        [ SX * RX,  SY * UX,  SZ * FX,  TX ]
        [ SX * RY,  SY * UY,  SZ * FY,  TY ]
        [ SX * RZ,  SY * UZ,  SZ * FZ,  TZ ]
        [ 0      ,  0      ,  0      ,  1  ]

        要直接设置旋转（绝对变换），我们需要从头开始构建矩阵：
        首先制作一个单位矩阵，然后应用 T、S 和新的 R 分量。
        注意顺序很重要：必须先缩放，然后旋转，最后平移。
        */

        glm::vec4 T = mTransform[3];  // 缓存当前的平移分量

        // 首先在单位矩阵上进行缩放
        mTransform = glm::mat4(1.0f);
        mTransform[0][0] = mScaleX;
        mTransform[1][1] = mScaleY;
        mTransform[2][2] = mScaleZ;
        mTransform[3][3] = 1;

        // [ SX, 00, 00, 00 ] -> 现在矩阵如下所示
        // [ 00, SY, 00, 00 ]
        // [ 00, 00, SZ, 00 ]
        // [ 00, 00, 00, 01 ]

        // 将四元数转换为新的旋转矩阵并应用回来
        mRotation = glm::normalize(rotation);
        mTransform = glm::mat4_cast(mRotation) * mTransform;  // 世界空间

        // [ SX * RX,  SY * UX,  SZ * FX,  0 ] -> 现在矩阵如下所示
        // [ SX * RY,  SY * UY,  SZ * FY,  0 ]
        // [ SX * RZ,  SY * UZ,  SZ * FZ,  0 ]
        // [ 0      ,  0      ,  0      ,  1 ]

        mTransform[3] = T;  // 最后将平移分量应用回来（替换最后一列）

        ReCalEuler();  // 重新计算欧拉角
        ReCalBasis();  // 重新计算基向量
    }
    void Transform::SetTransform(const glm::mat4& transform) {
        LOG_TRACK;
        mScaleX = glm::length(transform[0]);  // 计算缩放因子
        mScaleY = glm::length(transform[1]);  // 计算缩放因子
        mScaleZ = glm::length(transform[2]);  // 计算缩放因子

        // 提取纯旋转矩阵
        glm::mat3 pureRotationMatrix = glm::mat3(
            transform[0] / mScaleX,  // 归一化
            transform[1] / mScaleY,  // 归一化
            transform[2] / mScaleZ   // 归一化
        );

        mTransform = transform;                           // 更新变换矩阵
        mPosition = glm::vec3(transform[3]);              // 更新位置
        mRotation = glm::normalize(glm::quat_cast(pureRotationMatrix));  // 更新旋转四元数

        ReCalEuler();  // 重新计算欧拉角
        ReCalBasis();  // 重新计算基向量
    }

    void Transform::ReCalBasis() {
        LOG_TRACK;
        // 我们可以直接从变换矩阵的前三列获取基向量。
        // 根据我们对基向量的理解（请参考头文件中的文档），
        // 第 0、1 和 2 列对应于右向、上向和向后（在世界空间中）。

        mRight = glm::normalize(glm::vec3(mTransform[0]));
        mUp = glm::normalize(glm::vec3(mTransform[1]));
        mForward = glm::normalize(glm::vec3(mTransform[2])) * (-1.0f);

        // 另一种简单而强大的解决方案是在世界基础上应用四元数
        if constexpr (false) {
            mRight = mRotation * gWorldRight;
            mUp = mRotation * gWorldUp;
            mForward = mRotation * gWorldForward;
        }

        // 不要使用欧拉角或叉积，这可能会导致歧义
    }
    void Transform::ReCalEuler() {
        LOG_TRACK;
        // 从矩阵中提取欧拉角，按照 Y->X->Z 的顺序（偏航->俯仰->滚转）
        glm::extractEulerAngleYXZ(mTransform, mEulerY, mEulerX, mEulerZ);
        mEulerX = glm::degrees(mEulerX);
        mEulerY = glm::degrees(mEulerY);
        mEulerZ = glm::degrees(mEulerZ);

        // 或者，我们可以从四元数中提取欧拉角，如下所示仍然有效。
        // 但请注意，这与上面的方法并不等价，因为欧拉角的顺序很重要。
        // 特别地，`glm::eulerAngles` 假定隐含的顺序是 X->Y->Z，而不是我们的 Y->X->Z 约定，
        // 因此返回的值是不同的。事实上，这两个值都是正确的，它们只是表示同一方向的不同方式。
        // 我们希望首先或最后绕哪个轴旋转，只要保持一致就可以，但显式说明没有坏处。

        if constexpr (false) {
            glm::vec3 eulers = glm::eulerAngles(mRotation);
            mEulerX = glm::degrees(eulers.x);
            mEulerY = glm::degrees(eulers.y);
            mEulerZ = glm::degrees(eulers.z);
        }
    }
    glm::vec3 Transform::LocalToWorld(const glm::vec3& v) const {
        if constexpr (false) {
            return glm::mat3(mTransform) * v;  // 这是等价的
        }

        return mRotation * v;
    }

    glm::vec3 Transform::WorldToLocal(const glm::vec3& v) const {
        // 这是等价的，但计算矩阵的逆是昂贵的
        if constexpr (false) {
            return glm::inverse(glm::mat3(mTransform)) * v;
        }

        // 这是等价的，如果矩阵是正交的（没有非均匀缩放）
        if constexpr (false) {
            return glm::transpose(glm::mat3(mTransform)) * v;
        }

        // 四元数的逆只需要 `glm::dot(vec4, vec4)` 所以很便宜
        return glm::inverse(mRotation) * v;
    }
    glm::mat4 Transform::GetLocalTransform() const {
        return glm::lookAt(mPosition, mPosition + mForward, mUp);
    }
    glm::mat4 Transform::GetLocalTransform(const glm::vec3& forward, const glm::vec3& up) const {
        return glm::lookAt(mPosition, mPosition + forward, up);
    }
}
