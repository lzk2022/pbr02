#include "Animation.h"
#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ
#include <GLM/gtc/type_ptr.hpp>
#include "../utils/Log.h"
#include "../utils/Math.h"
using namespace utils;
namespace component {
	static constexpr glm::mat4 gIidentityM = glm::mat4(1.0f);
	static constexpr glm::quat gIdentityQ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	// ��Assimp��aiMatrix4x4����ת��ΪGLM��mat4����
	static inline glm::mat4 AssimpMat2GLM(const aiMatrix4x4& m) {
		return glm::transpose(glm::make_mat4(&m.a1));  // ��ΪaiMatrix4x4��������������Ҫת��
	}

    Channel::Channel(aiNodeAnim* aiChannel, const std::string& name, int id, float duration)
        : mName(name), mBoneId(id){
        LOG_TRACK;
        unsigned int numPosition = aiChannel->mNumPositionKeys;
        unsigned int numRotation = aiChannel->mNumRotationKeys;
        unsigned int numScales = aiChannel->mNumScalingKeys;

        // ��鶯��ͨ���Ĺؼ�֡�����Ƿ���Ч
        if (numPosition < 1 || numRotation < 1 || numScales < 1) {
            LOG_EXCEPTION(false, "��Ч�Ķ���ͨ����ÿ���ؼ�֡������Ҫһ��֡��");
        }

        // Ϊλ�á���ת�����Źؼ�֡���ݷ���ռ䣬��Ԥ��һ�������ĩβ֡��֧��ѭ������
        mPositions.resize(numPosition + 1, FT(glm::vec3(0.0f), -1.0f));
        mRotations.resize(numRotation + 1, FR(gIdentityQ, -1.0f));
        mScales.resize(numScales + 1, FS(glm::vec3(1.0f), -1.0f));

        // Assimp��֤�ؼ�֡��ʱ������򷵻������ظ�����������ֶ�����

        // ����λ�ùؼ�֡
        for (auto [i, preTime] = std::tuple(0U, 0.0f); i < numPosition; ++i) {
            auto& frame = aiChannel->mPositionKeys[i];
            auto& value = frame.mValue;
            auto timestamp = static_cast<float>(frame.mTime);

            LOG_ASSERT(timestamp >= preTime, "Assimp δ�ܰ�ʱ��˳�򷵻�֡��");
            mPositions[i].value = glm::vec3(value.x, value.y, value.z);
            mPositions[i].timestamp = timestamp;
            preTime = timestamp;

            // ����ǵ�һ��֡�����临�Ƶ����һ��֡��֧��ѭ��
            if (i == 0) {
                auto& lastFrame = mPositions.back();
                lastFrame.value = glm::vec3(value.x, value.y, value.z);
                lastFrame.timestamp = duration;
            }
        }

        // ������ת�ؼ�֡
        for (auto [i, prevTime] = std::tuple(0U, 0.0f); i < numRotation; ++i) {
            auto& frame = aiChannel->mRotationKeys[i];
            auto& value = frame.mValue;
            auto timestamp = static_cast<float>(frame.mTime);

            LOG_ASSERT(timestamp >= prevTime, "Assimp δ�ܰ�ʱ��˳�򷵻�֡��");
            mRotations[i].value = glm::quat(value.w, value.x, value.y, value.z);
            mRotations[i].timestamp = timestamp;
            prevTime = timestamp;

            // ����ǵ�һ��֡�����临�Ƶ����һ��֡��֧��ѭ��
            if (i == 0) {
                auto& last_frame = mRotations.back();
                last_frame.value = glm::quat(value.w, value.x, value.y, value.z);
                last_frame.timestamp = duration;
            }
        }

        // �������Źؼ�֡
        for (auto [i, preTime] = std::tuple(0U, 0.0f); i < numScales; ++i) {
            auto& frame = aiChannel->mScalingKeys[i];
            auto& value = frame.mValue;
            auto timestamp = static_cast<float>(frame.mTime);

            LOG_ASSERT(timestamp >= preTime, "Assimp δ�ܰ�ʱ��˳�򷵻�֡��");
            mScales[i].value = glm::vec3(value.x, value.y, value.z);
            mScales[i].timestamp = timestamp;
            preTime = timestamp;

            // ����ǵ�һ��֡�����临�Ƶ����һ��֡��֧��ѭ��
            if (i == 0) {
                auto& lastFrame = mScales.back();
                lastFrame.value = glm::vec3(value.x, value.y, value.z);
                lastFrame.timestamp = duration;
            }
        }
    }

    template<typename TFrame>
    std::tuple<int, int> Channel::GetFrameIndex(const std::vector<TFrame>& frames, float time) const {
        LOG_TRACK;
        if (frames.size() == 1) {
            return std::tuple(0, 0);
        }

        for (int i = 1; i < frames.size(); ++i) {
            auto& [_, timestamp] = frames.at(i);
            if (time < timestamp) {
                return std::tuple(i - 1, i);
            }
        }

        return std::tuple(-1, -1);  // �������-1����ʾ������ѭ��
    }

    glm::mat4 Channel::Interpolate(float time) const {
        LOG_TRACK;
        // ��ȡλ�á���ת�����Źؼ�֡��������
        auto [i1, i2] = GetFrameIndex<FT>(mPositions, time);
        auto& [prePosition, preTsT] = mPositions[i1];
        auto& [nextPosition, nextTsT] = mPositions[i2];

        auto [j1, j2] = GetFrameIndex<FR>(mRotations, time);
        auto& [preRotation, preTsR] = mRotations[j1];
        auto& [nextRotation, nextTsR] = mRotations[j2];

        auto [k1, k2] = GetFrameIndex<FS>(mScales, time);
        auto& [preScale, preTsS] = mScales[k1];
        auto& [nextScale, nextTsS] = mScales[k2];

        // ����ʱ��Ȩ�أ��������Բ�ֵ
        float percentT = Math::LinearPercent(preTsT, nextTsT, time);
        float percentR = Math::LinearPercent(preTsR, nextTsR, time);
        float percentS = Math::LinearPercent(preTsS, nextTsS, time);

        // ʹ��glm�����ռ�
        using namespace glm;

        // ���Բ�ֵ�����µ�λ�á���ת������
        vec3 newPosition = Math::Lerp(prePosition, nextPosition, percentT);
        quat newRotation = Math::Slerp(preRotation, nextRotation, percentR);
        vec3 newScale = Math::Lerp(preScale, nextScale, percentS);

        // �����任���󲢷���
        mat4 translation = glm::translate(gIidentityM, newPosition);
        mat4 rotation = glm::toMat4(newRotation);
        mat4 scale = glm::scale(gIidentityM, newScale);

        return translation * rotation * scale;
    }

    /*****************************************Animation**********************************************************/
    // ���캯������Assimp�����м��ض������ݵ�Animation����
    Animation::Animation(const aiScene* pAiScene, Model* model) : mChannels(0) {
        LOG_TRACK;
        // ȷ�������ļ�������������
        LOG_ASSERT(pAiScene->mNumAnimations > 0, "�����ļ�������������");
        aiAnimation* pAiAnimation = pAiScene->mAnimations[0];

        // ��Assimp������������ȡ���ơ�ʱ�����ٶ�
        name = pAiAnimation->mName.C_Str();
        duration = static_cast<float>(pAiAnimation->mDuration);
        speed = static_cast<float>(pAiAnimation->mTicksPerSecond);

        // ��channels������С����Ϊģ�͹������������ڴ洢����ͨ������
        channels.resize(model->mBoneNum);  // �����ƥ��ͨ����ʹ��resize������reserve
        auto& nodes = model->mNodes;

        // ����Assimp�����е�ÿ��ͨ��
        for (unsigned int i = 0; i < pAiAnimation->mNumChannels; ++i) {
            aiNodeAnim* pAiChannel = pAiAnimation->mChannels[i];
            std::string boneName = pAiChannel->mNodeName.C_Str();

            // ��ģ�ͽڵ��б��в�����ͨ����������ƥ��Ľڵ�
            auto node = ranges::find_if(nodes, [&boneName](const Node& node) {
                return node.mName == boneName;
            });

            // ���û���ҵ�ƥ��Ľڵ㣬������ͨ��
            if (node == nodes.end()) {
                continue;  // �����νṹ��û��ƥ��Ľڵ㣬������ͨ��
            }

            // ���Assimp��Ϊ�����ǹ�����������ͨ��
            if (!node->IsBone()) {
                continue;  // ���Assimp����Ϊ���ǹ�����������ͨ��
            }

            // ��ȡͨ����Ӧ�Ĺ��������������µ�Channel�����ƶ���channels��
            Channel& channel = channels[node->mBId];
            LOG_ASSERT(channel.mBoneId < 0, "��ͨ���Ѿ�����䣬�����ظ��Ĺ�����");

            channel = std::move(Channel(pAiChannel, boneName, node->mBId, duration));
            nodes[node->mNId].mAlive = true;
            mChannels++;
        }

        // ���ͨ�������Ƿ����������ƥ��
        unsigned int cnt = ranges::count_if(channels, [](const Channel& c) { return c.mBoneId >= 0; });
        LOG_ASSERT(mChannels == cnt, "ͨ����������ȷ�����������һһ��Ӧ��");
        LOG_ASSERT(mChannels <= model->mBoneNum, "��Ч��ͨ��δ��ɾ����������");
    }

    /*******************************************Animator**************************************************/
    void Animator::Reset(Model* model) {
        LOG_TRACK;
        // ȷ��ģ�;��ж�������
        LOG_ASSERT(model->mAnimation, "ģ��û�ж�����");

        // ���������任����������СΪ��������������ʼ��Ϊ��λ����
        mBoneTransforms.resize(model->mBoneNum, gIidentityM);
        mCurrentTime = 0.0f;  // ���õ�ǰʱ��Ϊ0
    }
    
    Animator::Animator(Model* model) { Reset(model); }

    void Animator::Update(Model& model, float deltatime) {
        LOG_TRACK;
        // ��ȡģ�͵Ķ������ݺ��ٶ�
        const auto& animation = model.mAnimation;
        mCurrentTime += animation->speed * deltatime;  // ���ݶ����ٶȺ�ʱ�䲽�����µ�ǰʱ��
        mCurrentTime = fmod(mCurrentTime, animation->duration);  // ѭ�����Ŷ���Ƭ��

        // ��ȡ������ͨ����ģ�͵Ľڵ�
        const auto& channels = animation->channels;
        auto& nodes = model.mNodes;

        // ����ģ�ͽڵ��б������˳����¹����任����
        for (auto& node : nodes) {
            int& boneId = node.mBId;       // ��ǰ�ڵ��Ӧ�Ĺ�������
            int& parentId = node.mPId;     // ��ǰ�ڵ�ĸ��ڵ�����

            // ����ڵ���Ҫ�������£���ʹ��ͨ�����в�ֵ����ڵ㵽���ڵ�ı任���󣬷��򱣳�ԭ�б任
            glm::mat4 n2p = node.Animated() ? channels[boneId].Interpolate(mCurrentTime) : node.mN2P;

            // ����Ӹ��ڵ㵽ģ�Ϳռ�ı任����
            glm::mat4 p2m = parentId < 0 ? glm::inverse(nodes[0].mN2P) : nodes[parentId].mN2M;

            // ���㵱ǰ�ڵ���µĽڵ㵽ģ�Ϳռ�ı任����
            node.mN2M = p2m * n2p;

            // �����ǰ�ڵ��ǹ����ڵ㣬���¹����任����
            if (node.IsBone()) {
                mBoneTransforms[boneId] = node.mN2M * node.mM2N;
            }
        }
    }
}
