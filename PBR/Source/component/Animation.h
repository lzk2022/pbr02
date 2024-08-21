#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <glm/glm.hpp>
#include <assimp/anim.h>
#include <assimp/scene.h>
#include "../component/Model.h"
namespace component {
	class Channel {
	private:
        template<typename TKey, int Key>  // ǿ���Ͷ���
        struct Frame {
            Frame(TKey value, float time) : value(value), timestamp(time) {}
            TKey value;
            float timestamp;
        };

        using FT = Frame<glm::vec3, 1>;  // λ�ùؼ�֡
        using FR = Frame<glm::quat, 2>;  // ��ת�ؼ�֡
        using FS = Frame<glm::vec3, 3>;  // ���Źؼ�֡

    private:
        std::vector<FT> mPositions;  // λ�ùؼ�֡����
        std::vector<FR> mRotations;  // ��ת�ؼ�֡����
        std::vector<FS> mScales;     // ���Źؼ�֡����
        // ��ȡ�ؼ�֡������˽�з���
        template<typename TFrame>
        std::tuple<int, int> GetFrameIndex(const std::vector<TFrame>& frames, float time) const;
    public:
        std::string mName;   // ͨ������
        int mBoneId = -1;   // ����ID��Ĭ��Ϊ-1��ʾ��Ч

        // Ĭ�Ϲ��캯��
        Channel() : mBoneId(-1) {}

        // ���캯������Assimp�Ľڵ㶯����ʼ��ͨ��
        Channel(aiNodeAnim* ai_channel, const std::string& name, int id, float duration);

        // �ƶ����캯��
        Channel(Channel&& other) = default;

        // �ƶ���ֵ�����
        Channel& operator=(Channel&& other) = default;

        // ����ʱ���ֵ�õ��任����
        glm::mat4 Interpolate(float time) const;

	};
    class Model;
    /******************************************Animation****************************************************/
    class Animation {
    private:
        unsigned int mChannels;        // ͨ������
        std::vector<Channel> channels;  // ͨ�����ϣ�������ID����
        friend class Animator;          // Animator����Animation����Ԫ�࣬���Է���˽�г�Ա

    public:
        std::string name;    // ��������
        float duration;      // ��������ʱ��
        float speed;         // ���������ٶ�

        // ��Assimp������ģ�ͳ�ʼ������
        Animation(const aiScene* ai_scene, Model* model);
    };

    
    /******************************************Animator********************************************************/
    class Animator : public Component {
    public:
        float mCurrentTime;             // ��ǰ����ʱ��
        std::vector<glm::mat4> mBoneTransforms;  // �����任���󼯺�

        // ���캯������ʼ��Animator����
        Animator(Model* model);

        // ���¶���״̬
        // model: Ҫ���¶�����ģ��
        // deltatime: ��һ֡����ǰ֡��ʱ����
        void Update(Model& model, float deltatime);

        // ���ö���״̬
        // model: Ҫ���ö�����ģ��
        void Reset(Model* model);
    };
}

