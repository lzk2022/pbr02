#include "Transform.h"
#include <GLM/gtx/euler_angles.hpp>
#include "../utils/Log.h"
namespace component {
    static constexpr glm::vec3 gOrigin{ 0.0f };           // ԭ��
    static constexpr glm::mat4 gIdentity{ 1.0f };           // ��λ����
    static constexpr glm::quat gEye{ 1.0f, 0.0f, 0.0f, 0.0f };  // ��λ��Ԫ����wxyz˳��
    static constexpr glm::vec3 gWorldRight{ 1.0f, 0.0f, 0.0f };  // ��������ϵ��+x�᷽��
    static constexpr glm::vec3 gWorldUp{ 0.0f, 1.0f, 0.0f };  // ��������ϵ��+y�᷽��
    static constexpr glm::vec3 gWorldForward{ 0.0f, 0.0f,-1.0f };  // ��������ϵ��-z�᷽��

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
        // ���ֲ��ռ����ƽ�ƣ����������Ǿֲ�����ϵ�е�����
        if (space == Space::Local) {
            glm::vec3 world = LocalToWorld(vector);             // ���ֲ�����ϵת��Ϊ��������ϵ
            this->mPosition += world;                           // ����λ��
            this->mTransform[3] = glm::vec4(mPosition, 1.0f);   // ���±任�����ƽ�Ʋ���
        }
        // ������ռ����ƽ�ƣ�ֱ�Ӹ�����������λ��
        else {
            this->mPosition += vector;                              // ����λ��
            this->mTransform[3] = glm::vec4(mPosition, 1.0f);       // ���±任�����ƽ�Ʋ���
        }
	}
    void Transform::Translate(float x, float y, float z, Space space){
        Translate(glm::vec3(x, y, z), space);                       // �������ص�glm::vec3�����汾��Translate����
    }
    void Transform::Rotate(const glm::vec3& axis, float angle, Space space){
        LOG_TRACK;
        float radians = glm::radians(angle);        // ���Ƕ�ת��Ϊ����
        glm::vec3 v = glm::normalize(axis);         // �淶����ת����

        glm::mat4 R = glm::rotate(radians, v);      // ��ת����4x4
        glm::quat Q = glm::angleAxis(radians, v);   // ��ת��Ԫ��

        // ���ֲ��ռ������ת�����������Ǿֲ�����ϵ�е��������� right = glm::vec3(1, 0, 0)
        if (space == Space::Local) {
            this->mTransform = this->mTransform * R;                // ���±任����
            this->mRotation = glm::normalize(this->mRotation * Q);  // ������ת��Ԫ��
        }
        // ������ռ������ת��������������������ϵ�е����򣬿��ܻ�����ƽ��
        else {
            this->mTransform = R * this->mTransform;                // ���±任����
            this->mRotation = glm::normalize(Q * this->mRotation);  // ������ת��Ԫ��
            this->mPosition = glm::vec3(this->mTransform[3]);       // ����λ��
        }

        ReCalEuler();   // ���¼���ŷ����
        ReCalBasis();   // ���¼��������
    }

    void Transform::Rotate(const glm::vec3& eulers, Space space){
        LOG_TRACK;
        glm::vec3 radians = glm::radians(eulers);  // ��ŷ����ת��Ϊ����

        // ��ת����4x4
        glm::mat4 RX = glm::rotate(radians.x, gWorldRight);
        glm::mat4 RY = glm::rotate(radians.y, gWorldUp);
        glm::mat4 RZ = glm::rotate(radians.z, gWorldForward);
        glm::mat4 R = RZ * RX * RY;  // Y->X->Z

        // ��ת��Ԫ��
        glm::quat QX = glm::angleAxis(radians.x, gWorldRight);
        glm::quat QY = glm::angleAxis(radians.y, gWorldUp);
        glm::quat QZ = glm::angleAxis(radians.z, gWorldForward);
        glm::quat Q = QZ * QX * QY;  // Y->X->Z

        // ���ֲ��ռ������ת��ŷ��������ھֲ�������������ת
        if (space == Space::Local) {
            this->mTransform = this->mTransform * R;              // ���±任����
            this->mRotation = glm::normalize(this->mRotation * Q);  // ������ת��Ԫ��
        }
        // ������ռ������ת��ŷ������������������ X��Y �� -Z ������ת
        else {
            this->mTransform = R * this->mTransform;              // ���±任����
            this->mRotation = glm::normalize(Q * this->mRotation);  // ������ת��Ԫ��
            this->mPosition = glm::vec3(this->mTransform[3]);     // ����λ��
        }

        ReCalEuler();   // ���¼���ŷ����
        ReCalBasis();   // ���¼��������
    }

    void Transform::Rotate(float eulerX, float eulerY, float eulerZ, Space space)
    {
        Rotate(glm::vec3(eulerX, eulerY, eulerZ), space);  // ���ð�ŷ���ǽ�����ת�ĺ���
    }

    void Transform::Scale(float scale) {
        LOG_TRACK;
        mTransform = glm::scale(mTransform, glm::vec3(scale));  // ���±任����
        mScaleX *= scale;   // ���� X ����������
        mScaleY *= scale;   // ���� Y ����������
        mScaleZ *= scale;   // ���� Z ����������
    }
    void Transform::Scale(const glm::vec3& scale) {
        LOG_TRACK;
        mTransform = glm::scale(mTransform, scale);  // ���±任����
        mScaleX *= scale.x;   // ���� X ����������
        mScaleY *= scale.y;   // ���� Y ����������
        mScaleZ *= scale.z;   // ���� Z ����������
    }
    void Transform::Scale(float scaleX, float scaleY, float scaleZ) {
        LOG_TRACK;
        Scale(glm::vec3(scaleX, scaleY, scaleZ));  // ���ð����������������ŵĺ���
    }
    void Transform::SetPosition(const glm::vec3& position) {
        LOG_TRACK;
        mPosition = position;                      // ����λ��
        mTransform[3] = glm::vec4(position, 1.0f);  // ���±任�����е�λ�÷���
    }

    void Transform::SetRotation(const glm::quat& rotation) {
        LOG_TRACK;
        /* 4x4 �任������������洢��������ʾ������
        ƽ�ƺ����ŷ����� X��Y �� Z ���Ϸֱ��� T �� S ��ʾ��
        ��ת������ R��U �� F ��ʾ���ֱ�������������ǰ��

        [ SX * RX,  SY * UX,  SZ * FX,  TX ]
        [ SX * RY,  SY * UY,  SZ * FY,  TY ]
        [ SX * RZ,  SY * UZ,  SZ * FZ,  TZ ]
        [ 0      ,  0      ,  0      ,  1  ]

        Ҫֱ��������ת�����Ա任����������Ҫ��ͷ��ʼ��������
        ��������һ����λ����Ȼ��Ӧ�� T��S ���µ� R ������
        ע��˳�����Ҫ�����������ţ�Ȼ����ת�����ƽ�ơ�
        */

        glm::vec4 T = mTransform[3];  // ���浱ǰ��ƽ�Ʒ���

        // �����ڵ�λ�����Ͻ�������
        mTransform = glm::mat4(1.0f);
        mTransform[0][0] = mScaleX;
        mTransform[1][1] = mScaleY;
        mTransform[2][2] = mScaleZ;
        mTransform[3][3] = 1;

        // [ SX, 00, 00, 00 ] -> ���ھ���������ʾ
        // [ 00, SY, 00, 00 ]
        // [ 00, 00, SZ, 00 ]
        // [ 00, 00, 00, 01 ]

        // ����Ԫ��ת��Ϊ�µ���ת����Ӧ�û���
        mRotation = glm::normalize(rotation);
        mTransform = glm::mat4_cast(mRotation) * mTransform;  // ����ռ�

        // [ SX * RX,  SY * UX,  SZ * FX,  0 ] -> ���ھ���������ʾ
        // [ SX * RY,  SY * UY,  SZ * FY,  0 ]
        // [ SX * RZ,  SY * UZ,  SZ * FZ,  0 ]
        // [ 0      ,  0      ,  0      ,  1 ]

        mTransform[3] = T;  // ���ƽ�Ʒ���Ӧ�û������滻���һ�У�

        ReCalEuler();  // ���¼���ŷ����
        ReCalBasis();  // ���¼��������
    }
    void Transform::SetTransform(const glm::mat4& transform) {
        LOG_TRACK;
        mScaleX = glm::length(transform[0]);  // ������������
        mScaleY = glm::length(transform[1]);  // ������������
        mScaleZ = glm::length(transform[2]);  // ������������

        // ��ȡ����ת����
        glm::mat3 pureRotationMatrix = glm::mat3(
            transform[0] / mScaleX,  // ��һ��
            transform[1] / mScaleY,  // ��һ��
            transform[2] / mScaleZ   // ��һ��
        );

        mTransform = transform;                           // ���±任����
        mPosition = glm::vec3(transform[3]);              // ����λ��
        mRotation = glm::normalize(glm::quat_cast(pureRotationMatrix));  // ������ת��Ԫ��

        ReCalEuler();  // ���¼���ŷ����
        ReCalBasis();  // ���¼��������
    }

    void Transform::ReCalBasis() {
        LOG_TRACK;
        // ���ǿ���ֱ�Ӵӱ任�����ǰ���л�ȡ��������
        // �������ǶԻ���������⣨��ο�ͷ�ļ��е��ĵ�����
        // �� 0��1 �� 2 �ж�Ӧ��������������������ռ��У���

        mRight = glm::normalize(glm::vec3(mTransform[0]));
        mUp = glm::normalize(glm::vec3(mTransform[1]));
        mForward = glm::normalize(glm::vec3(mTransform[2])) * (-1.0f);

        // ��һ�ּ򵥶�ǿ��Ľ�������������������Ӧ����Ԫ��
        if constexpr (false) {
            mRight = mRotation * gWorldRight;
            mUp = mRotation * gWorldUp;
            mForward = mRotation * gWorldForward;
        }

        // ��Ҫʹ��ŷ���ǻ���������ܻᵼ������
    }
    void Transform::ReCalEuler() {
        LOG_TRACK;
        // �Ӿ�������ȡŷ���ǣ����� Y->X->Z ��˳��ƫ��->����->��ת��
        glm::extractEulerAngleYXZ(mTransform, mEulerY, mEulerX, mEulerZ);
        mEulerX = glm::degrees(mEulerX);
        mEulerY = glm::degrees(mEulerY);
        mEulerZ = glm::degrees(mEulerZ);

        // ���ߣ����ǿ��Դ���Ԫ������ȡŷ���ǣ�������ʾ��Ȼ��Ч��
        // ����ע�⣬��������ķ��������ȼۣ���Ϊŷ���ǵ�˳�����Ҫ��
        // �ر�أ�`glm::eulerAngles` �ٶ�������˳���� X->Y->Z�����������ǵ� Y->X->Z Լ����
        // ��˷��ص�ֵ�ǲ�ͬ�ġ���ʵ�ϣ�������ֵ������ȷ�ģ�����ֻ�Ǳ�ʾͬһ����Ĳ�ͬ��ʽ��
        // ����ϣ�����Ȼ�������ĸ�����ת��ֻҪ����һ�¾Ϳ��ԣ�����ʽ˵��û�л�����

        if constexpr (false) {
            glm::vec3 eulers = glm::eulerAngles(mRotation);
            mEulerX = glm::degrees(eulers.x);
            mEulerY = glm::degrees(eulers.y);
            mEulerZ = glm::degrees(eulers.z);
        }
    }
    glm::vec3 Transform::LocalToWorld(const glm::vec3& v) const {
        if constexpr (false) {
            return glm::mat3(mTransform) * v;  // ���ǵȼ۵�
        }

        return mRotation * v;
    }

    glm::vec3 Transform::WorldToLocal(const glm::vec3& v) const {
        // ���ǵȼ۵ģ��������������ǰ����
        if constexpr (false) {
            return glm::inverse(glm::mat3(mTransform)) * v;
        }

        // ���ǵȼ۵ģ���������������ģ�û�зǾ������ţ�
        if constexpr (false) {
            return glm::transpose(glm::mat3(mTransform)) * v;
        }

        // ��Ԫ������ֻ��Ҫ `glm::dot(vec4, vec4)` ���Ժܱ���
        return glm::inverse(mRotation) * v;
    }
    glm::mat4 Transform::GetLocalTransform() const {
        return glm::lookAt(mPosition, mPosition + mForward, mUp);
    }
    glm::mat4 Transform::GetLocalTransform(const glm::vec3& forward, const glm::vec3& up) const {
        return glm::lookAt(mPosition, mPosition + forward, up);
    }
}
