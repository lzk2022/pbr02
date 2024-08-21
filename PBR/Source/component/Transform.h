#pragma once
#include <GLM/glm.hpp>
#include "../component/Component.h"
#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ
#include <glm/gtx/transform.hpp>         // GLM��OpenGL��ѧ�� �任
namespace component {
    // ��ʾ�ռ��ö������
    enum class Space : char {
        Local = 1 << 0,   // ���ؿռ�
        World = 1 << 1    // ����ռ�
    };

    class Transform : public Component {
    public:
        Transform();
        // ƽ�Ʋ���  ƽ������
        void Translate(const glm::vec3& vector, Space space = Space::World);
        void Translate(float x, float y, float z, Space space = Space::World);

        // ������ת����
        void Rotate(const glm::vec3& axis, float angle, Space space);
        void Rotate(const glm::vec3& eulers, Space space);
        // ʹ��ŷ���ǽ�����ת����
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
        void ReCalBasis(void);  // ���¼��������
        void ReCalEuler(void);  // ���¼���ŷ����

    public:
        glm::vec3 mPosition;           // λ��
        glm::quat mRotation;           // ��ת������Ԫ����ʾ��
        glm::mat4 mTransform;          // �任������������洢

        float mEulerX, mEulerY, mEulerZ;  // ŷ���ǣ�ƫ������������ת������λΪ��
        float mScaleX, mScaleY, mScaleZ;  // ��������

        glm::vec3 mUp;                 // ������
        glm::vec3 mForward;            // ǰ����
        glm::vec3 mRight;              // ������

    };
}


