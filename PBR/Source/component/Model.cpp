#include "Model.h"
#include "../utils/Log.h"
#include <GLM/gtc/type_ptr.hpp>
#include "../utils/Global.h"

namespace component {
	// 将Assimp中的aiMatrix4x4类型转换为glm::mat4类型的函数
   // 参数 m: Assimp的aiMatrix4x4类型矩阵
	static inline glm::mat4 AssimpMat2GLM(const aiMatrix4x4& m) {
		return glm::transpose(glm::make_mat4(&m.a1));  // aiMatrix4x4是行主序的，因此需要转置为列主序
	}
    constexpr unsigned int gMaxVtxBones = 4;  // 最大顶点骨骼数为4
	/********************************************************************************************************/
	Node::Node(int nid, int pid, const std::string& name)
		: mNId(nid), mPId(pid), mBId(-1), mName(name), mAlive(0){
		// 确保父节点在其子节点之前被处理
		LOG_ASSERT(nid >= 0 && pid < nid, "父节点没有在其子节点之前被处理！");  
	}
	bool Node::IsBone() const {
		return mBId >= 0;
	}
	bool Node::Animated() const {
		return (mBId >= 0) && mAlive;
	}
	
	/**********************************************Model******************************************************/

	void Model::ProcessTree(aiNode* pAiNode, int parent){
        LOG_TRACK;
		aiString& aiName = pAiNode->mName;
		auto& node = mNodes.emplace_back(mNodeNum++, parent, aiName.length == 0 ? "unnamed" : aiName.C_Str());
		node.mN2P = AssimpMat2GLM(pAiNode->mTransformation);
		int next_parent = mNodeNum - 1;  // 每次递归调用都有一个独立的本地副本

		// 递归处理当前节点的所有子节点
		for (unsigned int i = 0; i < pAiNode->mNumChildren; i++) {
			aiNode* child_node = pAiNode->mChildren[i];
			ProcessTree(child_node, next_parent);
		}
	}

	void Model::ProcessNode(aiNode* pAiNode) {
        LOG_TRACK;
		// 提前为当前节点中的每个网格分配存储空间
		mMeshes.reserve(mMeshes.size() + pAiNode->mNumMeshes);

		// 迭代处理当前节点中的每个网格
		for (unsigned int i = 0; i < pAiNode->mNumMeshes; i++) {
			unsigned int mesh_id = pAiNode->mMeshes[i];
			aiMesh* ai_mesh = mAiRoot->mMeshes[mesh_id];
			ProcessMesh(ai_mesh);
		}

		// 递归处理当前节点的所有子节点
		for (unsigned int i = 0; i < pAiNode->mNumChildren; i++) {
			aiNode* child_node = pAiNode->mChildren[i];
			ProcessNode(child_node);
		}
	}

    void Model::ProcessMesh(aiMesh* pAiMesh) {
        LOG_TRACK;
        std::vector<Mesh::Vertex> vertices;
        std::vector<GLuint> indices;
        std::bitset<6> localFormat;

        vertices.reserve(pAiMesh->mNumVertices);  // 预先分配顶点存储空间
        indices.reserve(pAiMesh->mNumFaces * 3);  // 每个面都是三角形

        // 确定当前网格的顶点格式
        localFormat.set(0, pAiMesh->HasPositions());                  // 是否包含顶点位置信息
        localFormat.set(1, pAiMesh->HasNormals());                    // 是否包含法线信息
        localFormat.set(2, pAiMesh->HasTextureCoords(0));             // 是否包含第1组纹理坐标信息
        localFormat.set(3, pAiMesh->HasTextureCoords(1) && pAiMesh->GetNumUVChannels() > 1);  // 是否包含第2组纹理坐标信息
        localFormat.set(4, pAiMesh->HasTangentsAndBitangents());      // 是否包含切线和副切线信息
        localFormat.set(5, pAiMesh->HasTangentsAndBitangents());      // 是否包含切线和副切线信息

        if (mVertNum == 0) {
            mVtxFormat = localFormat;  // 初始化顶点格式，若是第一个网格则直接使用其本地格式
        }

        static bool warned = false;
        if (mVtxFormat != localFormat && !warned) {
            LOG_WARN("不一致的顶点格式！某些网格缺少属性");
            warned = true;
        }

        mVtxFormat |= localFormat;  // 位或运算，合并每一对位

        // 构造网格顶点，不包含骨骼数据
        for (unsigned int i = 0; i < pAiMesh->mNumVertices; i++) {
            Mesh::Vertex vertex{};
            vertex.boneId = ivec4(-1);  // 将骨骼ID初始化为-1，而不是0

            if (localFormat.test(0)) {
                aiVector3D& aiPosition = pAiMesh->mVertices[i];
                vertex.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
            }

            if (localFormat.test(1)) {
                aiVector3D& aiNormal = pAiMesh->mNormals[i];
                vertex.normal = glm::vec3(aiNormal.x, aiNormal.y, aiNormal.z);
            }

            if (localFormat.test(2)) {
                aiVector3D& aiUV = pAiMesh->mTextureCoords[0][i];  // 第1组纹理坐标
                vertex.uv = glm::vec2(aiUV.x, aiUV.y);
            }

            if (localFormat.test(3)) {
                aiVector3D& aiUV2 = pAiMesh->mTextureCoords[1][i];  // 第2组纹理坐标
                vertex.uv2 = glm::vec2(aiUV2.x, aiUV2.y);
            }

            // 切线和副切线总是成对出现，如果存在一个，则另一个也存在
            if (localFormat.test(4) && localFormat.test(5)) {
                aiVector3D& aiTangent = pAiMesh->mTangents[i];
                aiVector3D& aiBinormal = pAiMesh->mBitangents[i];
                vertex.tangent = glm::vec3(aiTangent.x, aiTangent.y, aiTangent.z);
                vertex.binormal = glm::vec3(aiBinormal.x, aiBinormal.y, aiBinormal.z);
            }

            vertices.push_back(vertex);
            mVertNum++;
        }

        // 构造网格索引
        for (unsigned int i = 0; i < pAiMesh->mNumFaces; i++) {
            aiFace& triangle = pAiMesh->mFaces[i];
            LOG_ASSERT(triangle.mNumIndices == 3, "该多边形不是三角形！");

            // Assimp的默认绕序与OpenGL一致（逆时针）
            indices.push_back(triangle.mIndices[0]);
            indices.push_back(triangle.mIndices[1]);
            indices.push_back(triangle.mIndices[2]);
            mTriNum++;
        }

        // 填充顶点中缺失的骨骼数据
        if (mIsAnimated) {
            for (unsigned int i = 0; i < pAiMesh->mNumBones; i++) {
                aiBone* ai_bone = pAiMesh->mBones[i];
                std::string name = ai_bone->mName.C_Str();

                auto it = ranges::find_if(mNodes, [&name](const Node& node) { return name == node.mName; });
                LOG_ASSERT(it != mNodes.end(), "无效的骨骼，无法在节点层级中找到匹配！");

                // 获取非常量引用以更新节点（不能通过const迭代器`it`更新）
                Node& node = mNodes[it->mNId];  // 向量`nodes`通过节点ID进行索引

                // 第一次看到骨骼时，为其分配骨骼ID并更新m2n矩阵，否则bid >= 0表示已经在其他网格中更新，
                // 因此只需处理此新网格的骨骼权重。如果是这种情况，说明该骨骼将影响多个网格中的顶点，通常是树中的顶级节点

                if (node.mBId < 0) {  // 新骨骼
                    node.mM2N = AssimpMat2GLM(ai_bone->mOffsetMatrix);
                    node.mBId = mBoneNum++;
                }

                for (int j = 0; j < ai_bone->mNumWeights; j++) {
                    unsigned int vtxId = ai_bone->mWeights[j].mVertexId;
                    const float  weight = ai_bone->mWeights[j].mWeight;
                    LOG_ASSERT(vtxId < vertices.size(), "顶点ID超出范围！");

                    auto& vertex = vertices[vtxId];
                    bool full = glm::all(glm::greaterThanEqual(vertex.boneId, ivec4(0)));
                    LOG_ASSERT(!full, "发现超过每个顶点4个骨骼，请检查导入设置！");

                    for (int k = 0; k < gMaxVtxBones; k++) {
                        if (vertex.boneId[k] < 0) {
                            vertex.boneId[k] = node.mBId;
                            vertex.boneWt[k] = weight;
                            break;  // 每个骨骼只设置一个槽位
                        }
                    }
                }
            }
        }

        auto& mesh = mMeshes.emplace_back(vertices, indices);  // 在原地移动构造网格对象
        mMesheNum++;

        // 将网格与材质建立关联
        aiMaterial* ai_material = mAiRoot->mMaterials[pAiMesh->mMaterialIndex];
        ProcessMaterial(ai_material, mesh);
    }

    void Model::ProcessMaterial(aiMaterial* pAiMaterial, const Mesh& mesh) {
        LOG_TRACK;
        LOG_ASSERT(pAiMaterial != nullptr, "损坏的Assimp数据：材质为空指针！");

        aiString name;
        if (pAiMaterial->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
            LOG_ERROR("无法加载网格的材质（VAO = {0}）...", mesh.mVAO->getId());
            return;
        }

        std::string matkey{ name.C_Str() };

        // 检查材质键是否已存在于本地缓存中
        if (mMaterialsCache.find(matkey) != mMaterialsCache.end()) {
            GLuint matid = mMaterialsCache[matkey];
            mesh.SetMaterialID(matid);  // 重用先前的材质ID，因为材质是共享的
            return;
        }

        // 新材质，将材质键存储到本地缓存中
        GLuint matid = mesh.mMaterialId;
        mMaterialsCache[matkey] = matid;
    }

    Model::Model(const std::string& filepath, Quality quality, bool animate) : Component(), mIsAnimated(animate){
        LOG_TRACK;
        mFilePath = gResourcePath + filepath;
        mVtxFormat.reset();  // 重置顶点格式
        mMeshes.clear();      // 清空网格数据
        mMaterials.clear();   // 清空材质数据

        // 设置导入选项，包括质量选项和预处理选项
        // 参数 quality: 模型加载的质量选项
        unsigned int import_options = static_cast<unsigned int>(quality)
            | aiProcess_FlipUVs                      // 翻转UV坐标
            | aiProcess_Triangulate                  // 将所有图元转换为三角形
            | aiProcess_GenSmoothNormals             // 生成平滑法线
            | aiProcess_FindInvalidData              // 查找无效数据
            | aiProcess_ValidateDataStructure        // 验证数据结构
            | aiProcess_CalcTangentSpace             // 计算切线空间
            | aiProcess_LimitBoneWeights;            // 限制骨骼权重


        // 对于静态模型，让Assimp预先转换所有顶点（会失去层次结构）
        if (!mIsAnimated) {
            import_options |= aiProcess_PreTransformVertices;
        }

        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);  // 设置“每顶点4骨骼”规则
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, false);  // 禁止读取动画数据

        LOG_INFO("开始加载模型: {0}", mFilePath);
        auto startTime = std::chrono::high_resolution_clock::now();

        this->mAiRoot = importer.ReadFile(mFilePath, import_options);  // 使用Assimp读取模型文件

        if (!mAiRoot || mAiRoot->mRootNode == nullptr || mAiRoot->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            LOG_ERROR("无法导入模型: {0}", mFilePath);
            LOG_ERROR("Assimp错误: {0}", importer.GetErrorString());
            return;
        }

        ProcessTree(mAiRoot->mRootNode, -1);  // 递归处理并存储节点层次结构信息
        ProcessNode(mAiRoot->mRootNode);      // 递归处理每个节点

        if (mIsAnimated) {
            unsigned int cnt = ranges::count_if(mNodes, [](const Node& node) { return node.mBId >= 0; });
            LOG_ASSERT(mBoneNum == cnt, "数据损坏: 存在重复或缺失的骨骼!");
            LOG_ASSERT(mBoneNum <= 150, "动画仅支持最多100个骨骼!");
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> loadingTime = endTime - startTime;
        LOG_INFO("模型导入完成! 总加载时间: {0:.2f} 毫秒", loadingTime.count());
        mAiRoot = nullptr;

        // Assimp导入器会在函数返回时自动释放aiScene，因此无法手动释放根资源（第二个delete可能导致未定义行为）
        if constexpr (false) {
            delete mAiRoot;
            importer.FreeScene();
        }

        LOG_INFO("生成模型加载报告......（仅供参考）");
        LOG_INFO("-----------------------------------------------------");

        LOG_INFO("总网格数量:     {0}", mMesheNum);
        LOG_INFO("总顶点数量:     {0:.2f}k ({1})", mVertNum * 0.001f, mVertNum);
        LOG_INFO("总三角形数量:   {0:.2f}k ({1})", mTriNum * 0.001f, mTriNum);
        LOG_INFO("-----------------------------------------------------");

        LOG_INFO("顶点是否具有位置信息 ? [{0}]", mVtxFormat.test(0) ? "Y" : "N");
        LOG_INFO("顶点是否具有法线信息 ? [{0}]", mVtxFormat.test(1) ? "Y" : "N");
        LOG_INFO("顶点是否具有UV集1信息 ? [{0}]", mVtxFormat.test(2) ? "Y" : "N");
        LOG_INFO("顶点是否具有UV集2信息 ? [{0}]", mVtxFormat.test(3) ? "Y" : "N");
        LOG_INFO("顶点是否具有切线/副切线信息 ? [{0}]", mVtxFormat.test(4) ? "Y" : "N");
        LOG_INFO("-----------------------------------------------------");

        std::string allMtls = "不可用";

        if (!mMaterialsCache.empty()) {
            auto it = mMaterialsCache.begin();
            allMtls = it->first + " (id = " + std::to_string(it->second) + ")";
            std::advance(it, 1);
            while (it != mMaterialsCache.end()) {
                allMtls += (", " + it->first + " (id = " + std::to_string(it->second) + ")");
                std::advance(it, 1);
            }
        }

        LOG_INFO("内部材质: {0}", allMtls);
        LOG_INFO("-----------------------------------------------------");
    }

    Material& Model::SetMaterial(const std::string& matkey, std::shared_ptr<Material>&& material) {
        LOG_TRACK;
        LOG_ASSERT(mMaterialsCache.count(matkey) > 0, "无效的材质键: {0}", matkey);

        // 注意，我们期望material参数是资产管理器中原始asset_ref的临时右值副本，
        // 因此我们可以直接移动它到指定位置

        GLuint matid = mMaterialsCache[matkey];
        mMaterials.insert_or_assign(matid, std::move(material));
        return mMaterials.at(matid);
    }

    void Model::AttachMotion(const std::string& filepath) {
        LOG_TRACK;
        if (!mIsAnimated) {
            LOG_ERROR("无法将动画附加到模型，模型必须是动画的");
        }

        // 导入选项，用于控制Assimp导入模型时的处理步骤。
        // 这些选项用于处理模型的UV翻转、三角化、生成平滑法线、查找无效数据、验证数据结构、计算切线空间和限制骨骼权重。
        // 注意：加载动画时，必须禁用aiProcess_PreTransformVertices标志。
        const unsigned int import_options = 0
            | aiProcess_FlipUVs
            | aiProcess_Triangulate
            | aiProcess_GenSmoothNormals
            | aiProcess_FindInvalidData
            | aiProcess_ValidateDataStructure
            | aiProcess_CalcTangentSpace
            | aiProcess_LimitBoneWeights
            // | aiProcess_PreTransformVertices  // 必须禁用此标志以加载动画
            ;

        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, true);

        LOG_INFO("开始加载动画: {0}", filepath);
        const aiScene* scene = importer.ReadFile(filepath, import_options);

        // 注意，这里我们不需要检查`scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE`
        // 因为文件可以仅包含动画（不包含顶点、网格），此时场景可能是不完整的，但动画仍然有效

        if (!scene || scene->mRootNode == nullptr) {
            LOG_ERROR("加载动画失败: {0}", filepath);
            LOG_ERROR("Assimp错误: {0}", importer.GetErrorString());
            return;
        }

        mAnimation = std::make_unique<Animation>(scene, this);
    }
}
