#pragma once
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../component/Component.h"
#include "Mesh.h"
#include "Material.h"
#include "Animation.h"
namespace component {
	class Animation;
	class Mesh;
	class Material;

	/*********************************************Node*******************************************************/
	class Node {
	public:
		Node(int nid, int pid, const std::string& name);

		// 判断当前节点是否为骨骼节点
		bool IsBone() const;
		// 判断当前节点是否是动画节点（既是骨骼节点并且受动画影响）
		bool Animated() const;

	public:
		int mNId = -1;    // 节点ID，必须大于等于0
		int mPId = -1;    // 父节点ID，对于根节点为-1
		int mBId = -1;    // 骨骼ID，如果节点不是骨骼节点则为-1
		bool mAlive = 0;  // 是否是骨骼节点且受通道影响

		std::string mName;   // 节点名称
		glm::mat4 mN2P;      // 节点空间到父节点空间的变换矩阵（相对于父节点的局部变换）
		glm::mat4 mM2N;      // 模型空间（绑定姿势）到节点空间（骨骼空间）的变换矩阵，仅适用于骨骼节点
		glm::mat4 mN2M;      // 节点空间到模型空间的变换矩阵，运行时更新，如果节点不活动则不适用
	};
	enum class Quality : uint32_t {  // 导入质量预设
		Auto = 0x0,
		Low = aiProcessPreset_TargetRealtime_Fast,
		Medium = aiProcessPreset_TargetRealtime_Quality,
		High = aiProcessPreset_TargetRealtime_MaxQuality
	};

	/*************************************************Model****************************************************/
	class Model : public Component {
	public:
		// 递归处理节点树结构，深度优先遍历，存储每个节点的层次信息
		void ProcessTree(aiNode* aiNode, int parent);
		// 递归处理节点，处理当前节点下的所有网格
		void ProcessNode(aiNode* aiNode);
		// 处理网格数据，构造顶点和索引，填充Mesh对象
		void ProcessMesh(aiMesh* aiMesh);
		// 处理材质数据，将材质关键字与ID进行映射，并关联材质与网格
		void ProcessMaterial(aiMaterial* aiMaterial, const Mesh& mesh);
		// 构造函数，加载模型文件并初始化模型对象
		Model(const std::string& filepath, Quality quality, bool animate = false);
		// 附加动画，加载并关联模型的动画数据
		void AttachMotion(const std::string& filepath);
		Material& SetMaterial(const std::string& matkey, std::shared_ptr<Material>&& material);

	public:
		unsigned int mNodeNum = 0;     // 节点总数
		unsigned int mBoneNum = 0;     // 骨骼总数
		unsigned int mMesheNum = 0;    // 网格总数
		unsigned int mVertNum = 0;     // 顶点总数
		unsigned int mTriNum = 0;      // 三角形总数
		bool mIsAnimated = false;        // 是否包含动画

		std::vector<Node> mNodes;             // 节点列表
		std::vector<Mesh> mMeshes;            // 网格列表
		std::unordered_map<GLuint, Material> mMaterials;  // 材质映射，关联材质ID和材质对象
		std::unique_ptr<Animation> mAnimation;  // 动画对象的唯一指针

	private:
		const aiScene* mAiRoot = nullptr;  // Assimp场景根节点指针
		std::bitset<6> mVtxFormat;         // 顶点格式位集，用于描述模型顶点的属性
		std::unordered_map<std::string, GLuint> mMaterialsCache;  // 材质缓存，用于存储材质关键字和ID的映射
		std::string mFilePath;

	};

}
