#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ
#include "../component/Component.h"
#include "Transform.h"
namespace component {
    enum class View : char {
        Orthgraphic = 1 << 0,  // ������ͼ�ı��
        Perspective = 1 << 1   // ͸����ͼ�ı��
    };

    class Camera : public Component {
    public:
        float mFov;           // �ӳ���
        float mNearClip;     // ���ü���
        float mFarClip;      // Զ�ü���
        float mMoveSpeed;    // �ƶ��ٶ�
        float mZoomSpeed;    // �����ٶ�
        float mRotateSpeed;  // ��ת�ٶ�
        float mOrbitSpeed;   // ����ٶ�

        glm::vec3 mInitPosition;   // ��ʼλ��
        glm::quat mInitRotation;   // ��ʼ��ת��Ԫ��

        Transform* mpTransform;  // ָ�� Transform ���ָ�룬������ ECS ��������Ը�ָ��ɿ�
        View mView;     // ��ͼ����

    public:
        // ���캯������ʼ��������
        // ������
        //   T: Transform ָ�룬�����λ�úͷ���
        //   view: ��ͼ���ͣ�Ĭ��Ϊ͸����ͼ
        Camera(Transform* T, View view = View::Perspective);

        // ��ȡ��ͼ����
        glm::mat4 GetViewMatrix() const;

        // ��ȡͶӰ����
        glm::mat4 GetProjectionMatrix() const;

        // �������״̬
        void Update();
    };
}
