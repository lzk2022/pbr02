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
        template<typename TKey, int Key>  // 强类型定义
        struct Frame {
            Frame(TKey value, float time) : value(value), timestamp(time) {}
            TKey value;
            float timestamp;
        };

        using FT = Frame<glm::vec3, 1>;  // 位置关键帧
        using FR = Frame<glm::quat, 2>;  // 旋转关键帧
        using FS = Frame<glm::vec3, 3>;  // 缩放关键帧

    private:
        std::vector<FT> mPositions;  // 位置关键帧集合
        std::vector<FR> mRotations;  // 旋转关键帧集合
        std::vector<FS> mScales;     // 缩放关键帧集合
        // 获取关键帧索引的私有方法
        template<typename TFrame>
        std::tuple<int, int> GetFrameIndex(const std::vector<TFrame>& frames, float time) const;
    public:
        std::string mName;   // 通道名称
        int mBoneId = -1;   // 骨骼ID，默认为-1表示无效

        // 默认构造函数
        Channel() : mBoneId(-1) {}

        // 构造函数，从Assimp的节点动画初始化通道
        Channel(aiNodeAnim* ai_channel, const std::string& name, int id, float duration);

        // 移动构造函数
        Channel(Channel&& other) = default;

        // 移动赋值运算符
        Channel& operator=(Channel&& other) = default;

        // 根据时间插值得到变换矩阵
        glm::mat4 Interpolate(float time) const;

	};
    class Model;
    /******************************************Animation****************************************************/
    class Animation {
    private:
        unsigned int mChannels;        // 通道数量
        std::vector<Channel> channels;  // 通道集合，按骨骼ID索引
        friend class Animator;          // Animator类是Animation的友元类，可以访问私有成员

    public:
        std::string name;    // 动画名称
        float duration;      // 动画持续时间
        float speed;         // 动画播放速度

        // 从Assimp场景和模型初始化动画
        Animation(const aiScene* ai_scene, Model* model);
    };

    
    /******************************************Animator********************************************************/
    class Animator : public Component {
    public:
        float mCurrentTime;             // 当前动画时间
        std::vector<glm::mat4> mBoneTransforms;  // 骨骼变换矩阵集合

        // 构造函数，初始化Animator对象
        Animator(Model* model);

        // 更新动画状态
        // model: 要更新动画的模型
        // deltatime: 上一帧到当前帧的时间间隔
        void Update(Model& model, float deltatime);

        // 重置动画状态
        // model: 要重置动画的模型
        void Reset(Model* model);
    };
}

