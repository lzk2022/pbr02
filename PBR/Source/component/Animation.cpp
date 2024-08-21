#include "Animation.h"
#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展
#include <GLM/gtc/type_ptr.hpp>
#include "../utils/Log.h"
#include "../utils/Math.h"
using namespace utils;
namespace component {
	static constexpr glm::mat4 gIidentityM = glm::mat4(1.0f);
	static constexpr glm::quat gIdentityQ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	// 将Assimp的aiMatrix4x4类型转换为GLM的mat4类型
	static inline glm::mat4 AssimpMat2GLM(const aiMatrix4x4& m) {
		return glm::transpose(glm::make_mat4(&m.a1));  // 因为aiMatrix4x4是行主序，所以需要转置
	}

    Channel::Channel(aiNodeAnim* aiChannel, const std::string& name, int id, float duration)
        : mName(name), mBoneId(id){
        LOG_TRACK;
        unsigned int numPosition = aiChannel->mNumPositionKeys;
        unsigned int numRotation = aiChannel->mNumRotationKeys;
        unsigned int numScales = aiChannel->mNumScalingKeys;

        // 检查动画通道的关键帧数量是否有效
        if (numPosition < 1 || numRotation < 1 || numScales < 1) {
            LOG_EXCEPTION(false, "无效的动画通道，每个关键帧至少需要一个帧。");
        }

        // 为位置、旋转和缩放关键帧数据分配空间，并预留一个额外的末尾帧以支持循环动画
        mPositions.resize(numPosition + 1, FT(glm::vec3(0.0f), -1.0f));
        mRotations.resize(numRotation + 1, FR(gIdentityQ, -1.0f));
        mScales.resize(numScales + 1, FS(glm::vec3(1.0f), -1.0f));

        // Assimp保证关键帧按时间戳升序返回且无重复，因此无需手动排序

        // 处理位置关键帧
        for (auto [i, preTime] = std::tuple(0U, 0.0f); i < numPosition; ++i) {
            auto& frame = aiChannel->mPositionKeys[i];
            auto& value = frame.mValue;
            auto timestamp = static_cast<float>(frame.mTime);

            LOG_ASSERT(timestamp >= preTime, "Assimp 未能按时间顺序返回帧！");
            mPositions[i].value = glm::vec3(value.x, value.y, value.z);
            mPositions[i].timestamp = timestamp;
            preTime = timestamp;

            // 如果是第一个帧，将其复制到最后一个帧以支持循环
            if (i == 0) {
                auto& lastFrame = mPositions.back();
                lastFrame.value = glm::vec3(value.x, value.y, value.z);
                lastFrame.timestamp = duration;
            }
        }

        // 处理旋转关键帧
        for (auto [i, prevTime] = std::tuple(0U, 0.0f); i < numRotation; ++i) {
            auto& frame = aiChannel->mRotationKeys[i];
            auto& value = frame.mValue;
            auto timestamp = static_cast<float>(frame.mTime);

            LOG_ASSERT(timestamp >= prevTime, "Assimp 未能按时间顺序返回帧！");
            mRotations[i].value = glm::quat(value.w, value.x, value.y, value.z);
            mRotations[i].timestamp = timestamp;
            prevTime = timestamp;

            // 如果是第一个帧，将其复制到最后一个帧以支持循环
            if (i == 0) {
                auto& last_frame = mRotations.back();
                last_frame.value = glm::quat(value.w, value.x, value.y, value.z);
                last_frame.timestamp = duration;
            }
        }

        // 处理缩放关键帧
        for (auto [i, preTime] = std::tuple(0U, 0.0f); i < numScales; ++i) {
            auto& frame = aiChannel->mScalingKeys[i];
            auto& value = frame.mValue;
            auto timestamp = static_cast<float>(frame.mTime);

            LOG_ASSERT(timestamp >= preTime, "Assimp 未能按时间顺序返回帧！");
            mScales[i].value = glm::vec3(value.x, value.y, value.z);
            mScales[i].timestamp = timestamp;
            preTime = timestamp;

            // 如果是第一个帧，将其复制到最后一个帧以支持循环
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

        return std::tuple(-1, -1);  // 如果返回-1，表示动画不循环
    }

    glm::mat4 Channel::Interpolate(float time) const {
        LOG_TRACK;
        // 获取位置、旋转和缩放关键帧的索引对
        auto [i1, i2] = GetFrameIndex<FT>(mPositions, time);
        auto& [prePosition, preTsT] = mPositions[i1];
        auto& [nextPosition, nextTsT] = mPositions[i2];

        auto [j1, j2] = GetFrameIndex<FR>(mRotations, time);
        auto& [preRotation, preTsR] = mRotations[j1];
        auto& [nextRotation, nextTsR] = mRotations[j2];

        auto [k1, k2] = GetFrameIndex<FS>(mScales, time);
        auto& [preScale, preTsS] = mScales[k1];
        auto& [nextScale, nextTsS] = mScales[k2];

        // 计算时间权重，用于线性插值
        float percentT = Math::LinearPercent(preTsT, nextTsT, time);
        float percentR = Math::LinearPercent(preTsR, nextTsR, time);
        float percentS = Math::LinearPercent(preTsS, nextTsS, time);

        // 使用glm命名空间
        using namespace glm;

        // 线性插值生成新的位置、旋转和缩放
        vec3 newPosition = Math::Lerp(prePosition, nextPosition, percentT);
        quat newRotation = Math::Slerp(preRotation, nextRotation, percentR);
        vec3 newScale = Math::Lerp(preScale, nextScale, percentS);

        // 构建变换矩阵并返回
        mat4 translation = glm::translate(gIidentityM, newPosition);
        mat4 rotation = glm::toMat4(newRotation);
        mat4 scale = glm::scale(gIidentityM, newScale);

        return translation * rotation * scale;
    }

    /*****************************************Animation**********************************************************/
    // 构造函数：从Assimp场景中加载动画数据到Animation对象
    Animation::Animation(const aiScene* pAiScene, Model* model) : mChannels(0) {
        LOG_TRACK;
        // 确保输入文件包含动画数据
        LOG_ASSERT(pAiScene->mNumAnimations > 0, "输入文件不包含动画！");
        aiAnimation* pAiAnimation = pAiScene->mAnimations[0];

        // 从Assimp动画对象中提取名称、时长和速度
        name = pAiAnimation->mName.C_Str();
        duration = static_cast<float>(pAiAnimation->mDuration);
        speed = static_cast<float>(pAiAnimation->mTicksPerSecond);

        // 将channels向量大小调整为模型骨骼数量，用于存储动画通道数据
        channels.resize(model->mBoneNum);  // 与骨骼匹配通道，使用resize而不是reserve
        auto& nodes = model->mNodes;

        // 遍历Assimp动画中的每个通道
        for (unsigned int i = 0; i < pAiAnimation->mNumChannels; ++i) {
            aiNodeAnim* pAiChannel = pAiAnimation->mChannels[i];
            std::string boneName = pAiChannel->mNodeName.C_Str();

            // 在模型节点列表中查找与通道骨骼名称匹配的节点
            auto node = ranges::find_if(nodes, [&boneName](const Node& node) {
                return node.mName == boneName;
            });

            // 如果没有找到匹配的节点，跳过该通道
            if (node == nodes.end()) {
                continue;  // 如果层次结构中没有匹配的节点，放弃此通道
            }

            // 如果Assimp认为它不是骨骼，跳过该通道
            if (!node->IsBone()) {
                continue;  // 如果Assimp不认为它是骨骼，放弃此通道
            }

            // 获取通道对应的骨骼索引，创建新的Channel对象并移动到channels中
            Channel& channel = channels[node->mBId];
            LOG_ASSERT(channel.mBoneId < 0, "该通道已经被填充，存在重复的骨骼！");

            channel = std::move(Channel(pAiChannel, boneName, node->mBId, duration));
            nodes[node->mNId].mAlive = true;
            mChannels++;
        }

        // 检查通道数量是否与骨骼数量匹配
        unsigned int cnt = ranges::count_if(channels, [](const Channel& c) { return c.mBoneId >= 0; });
        LOG_ASSERT(mChannels == cnt, "通道数量不正确，必须与骨骼一一对应！");
        LOG_ASSERT(mChannels <= model->mBoneNum, "无效的通道未被删除，请清理！");
    }

    /*******************************************Animator**************************************************/
    void Animator::Reset(Model* model) {
        LOG_TRACK;
        // 确保模型具有动画数据
        LOG_ASSERT(model->mAnimation, "模型没有动画！");

        // 调整骨骼变换矩阵向量大小为骨骼数量，并初始化为单位矩阵
        mBoneTransforms.resize(model->mBoneNum, gIidentityM);
        mCurrentTime = 0.0f;  // 重置当前时间为0
    }
    
    Animator::Animator(Model* model) { Reset(model); }

    void Animator::Update(Model& model, float deltatime) {
        LOG_TRACK;
        // 获取模型的动画数据和速度
        const auto& animation = model.mAnimation;
        mCurrentTime += animation->speed * deltatime;  // 根据动画速度和时间步长更新当前时间
        mCurrentTime = fmod(mCurrentTime, animation->duration);  // 循环播放动画片段

        // 获取动画的通道和模型的节点
        const auto& channels = animation->channels;
        auto& nodes = model.mNodes;

        // 遍历模型节点列表，按层次顺序更新骨骼变换矩阵
        for (auto& node : nodes) {
            int& boneId = node.mBId;       // 当前节点对应的骨骼索引
            int& parentId = node.mPId;     // 当前节点的父节点索引

            // 如果节点需要动画更新，则使用通道进行插值计算节点到父节点的变换矩阵，否则保持原有变换
            glm::mat4 n2p = node.Animated() ? channels[boneId].Interpolate(mCurrentTime) : node.mN2P;

            // 计算从父节点到模型空间的变换矩阵
            glm::mat4 p2m = parentId < 0 ? glm::inverse(nodes[0].mN2P) : nodes[parentId].mN2M;

            // 计算当前节点的新的节点到模型空间的变换矩阵
            node.mN2M = p2m * n2p;

            // 如果当前节点是骨骼节点，更新骨骼变换矩阵
            if (node.IsBone()) {
                mBoneTransforms[boneId] = node.mN2M * node.mM2N;
            }
        }
    }
}
