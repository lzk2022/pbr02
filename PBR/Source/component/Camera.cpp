#include "Camera.h"
#include <glad/glad.h>      // GLAD��OpenGL������
#include <glm/glm.hpp>                   // GLM��OpenGL��ѧ��
#include <glm/ext.hpp>                   // GLM��OpenGL��ѧ�� ��չ
#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ
#include <glm/gtx/transform.hpp>         // GLM��OpenGL��ѧ�� �任
#include <glm/gtc/type_ptr.hpp>          // GLM��OpenGL��ѧ�� ָ��
#include <glm/gtx/string_cast.hpp>       // GLM��OpenGL��ѧ�� �ַ���ת��
#include <glm/gtc/matrix_transform.hpp>  // GLM��OpenGL��ѧ�� ����任
#include <glm/gtx/perpendicular.hpp>     // GLM��OpenGL��ѧ�� ��ֱ����
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
    // ��ȡ��ͼ����
    glm::mat4 Camera::GetViewMatrix() const {
        LOG_TRACK;
        // �����Ѿ�����ȷ�ı任����ȡ����������ֱ�ӵõ���ͼ����
        // Ȼ�����������԰����������ѡ��ʹ��`glm::lookAt()`������
        // ��ֻ����������Ĳ�˺͵�����㡣
        if constexpr (true) {
            return glm::inverse(mpTransform->mTransform);
        }
        else {
            return glm::lookAt(mpTransform->mPosition, mpTransform->mPosition + mpTransform->mForward, mpTransform->mUp);
        }
    }

    // ��ȡͶӰ����
    glm::mat4 Camera::GetProjectionMatrix() const {
        LOG_TRACK;
        return (mView == View::Orthgraphic)
            ? glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, mNearClip, mFarClip)  // ����ͶӰ
            : glm::perspective(glm::radians(mFov), Window::mAspectRatio, mNearClip, mFarClip);  // ͸��ͶӰ
    }

    void Camera::Update()
    {
        float deltatime = Clock::mDeltaTime;
        static bool recovering = false;  // �Ƿ��ڻָ�����ʼλ�ú���ת��

        // ƽ���ؽ�����ָ�����ʼλ�úͷ���
        if (recovering) {
            float t = Math::EaseFactor(10.0f, deltatime);
            mpTransform->SetPosition(Math::Lerp(mpTransform->mPosition, mInitPosition, t));         // ƽ���ƶ�����ʼλ��
            mpTransform->SetRotation(Math::SlerpRaw(mpTransform->mRotation, mInitRotation, t));     // ƽ����ת����ʼ����

            if (Math::Equals(mpTransform->mPosition, mInitPosition) && Math::Equals(mpTransform->mRotation, mInitRotation)) {
                recovering = false;  // ��λ�úͷ��򶼻ָ�ʱֹͣ�ָ�
            }

            return;  // �ڻָ������ڼ���������û�����
        }

        // ���������ڰ�ס�����������ƶ������Χ�������ת�������������
        if (Input::GetMouseDown(MouseButton::Left)) {
            static constexpr auto world_up = glm::vec3(0.0f, 1.0f, 0.0f);

            // �������������ģʽ��ˮƽ��������������ᣨ+Y������ֱ������ƾֲ���������ת��
            // ������ת����������ռ�����ɵġ�
            // ��ֱ���Ӧ�ñ�������һ����Χ�ڣ�ʹ�������ŷ����x��Զ���ᳬ��������Χ(-90, 90)��
            // ��������ᱻ���á�

            float orbitY = -Input::GetCursorOffset(MouseAxis::Horizontal) * mOrbitSpeed;
            float orbitX = -Input::GetCursorOffset(MouseAxis::Vertical) * mOrbitSpeed;

            // �� `T->eulerX + orbitX` ������ (-89, 89) ��Χ��
            orbitX = glm::clamp(orbitX, -mpTransform->mEulerX - 89.0f, -mpTransform->mEulerX + 89.0f);

            mpTransform->Rotate(world_up, orbitY, Space::World);     // ��������������ת
            mpTransform->Rotate(mpTransform->mRight, orbitX, Space::World);     // �ƾֲ���������ת
            return;  // �����������ģʽ�º��������¼���������
        }

        // ������ţ��ڰ�ס�Ҽ�������»������
        if (Input::GetMouseDown(MouseButton::Right)) {
            mFov -= Input::GetCursorOffset(MouseAxis::Horizontal) * mZoomSpeed;
            mFov = glm::clamp(mFov, 30.0f, 120.0f);  // �����ӽǷ�Χ��30��120��֮��
            return;  // ��ƽ�������ڼ���������¼�����������ת��
        }

        // һ���ͷ���갴ť��ƽ���ؽ��ӽ�fov�ָ���90�ȣ�Ĭ��ֵ��
        mFov = Math::Lerp(mFov, 90.0f, Math::EaseFactor(20.0f, deltatime));

        // ֻ�е�û����갴ť�¼��������жϵ��¼���ʱ���Ŵ������¼�
        if (Input::GetKeyDown('r')) {
            recovering = true;  // �ָ�����ʼλ�úͷ���
        }

        // ��ת������X��Y�ᣨֻ�и�����ƫ����û�з�����
        float eulerY = mpTransform->mEulerY - Input::GetCursorOffset(MouseAxis::Horizontal) * mRotateSpeed;
        float eulerX = mpTransform->mEulerX - Input::GetCursorOffset(MouseAxis::Vertical) * mRotateSpeed;

        eulerY = glm::radians(eulerY);  // ���Ƕ�ת��Ϊ����
        eulerX = glm::radians(glm::clamp(eulerX, -89.0f, 89.0f));  // ���ƴ�ֱ��ת
        glm::quat rotation = glm::quat_cast(glm::eulerAngleYXZ(eulerY, eulerX, 0.0f));

        mpTransform->SetRotation(rotation);

        // ƽ�ƣ�δ��һ�����ƶ��ڶԽ��߷����ϸ��죩
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