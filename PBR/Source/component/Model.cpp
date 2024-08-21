#include "Model.h"
#include "../utils/Log.h"
#include <GLM/gtc/type_ptr.hpp>
#include "../utils/Global.h"

namespace component {
	// ��Assimp�е�aiMatrix4x4����ת��Ϊglm::mat4���͵ĺ���
   // ���� m: Assimp��aiMatrix4x4���;���
	static inline glm::mat4 AssimpMat2GLM(const aiMatrix4x4& m) {
		return glm::transpose(glm::make_mat4(&m.a1));  // aiMatrix4x4��������ģ������Ҫת��Ϊ������
	}
    constexpr unsigned int gMaxVtxBones = 4;  // ��󶥵������Ϊ4
	/********************************************************************************************************/
	Node::Node(int nid, int pid, const std::string& name)
		: mNId(nid), mPId(pid), mBId(-1), mName(name), mAlive(0){
		// ȷ�����ڵ������ӽڵ�֮ǰ������
		LOG_ASSERT(nid >= 0 && pid < nid, "���ڵ�û�������ӽڵ�֮ǰ������");  
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
		int next_parent = mNodeNum - 1;  // ÿ�εݹ���ö���һ�������ı��ظ���

		// �ݹ鴦��ǰ�ڵ�������ӽڵ�
		for (unsigned int i = 0; i < pAiNode->mNumChildren; i++) {
			aiNode* child_node = pAiNode->mChildren[i];
			ProcessTree(child_node, next_parent);
		}
	}

	void Model::ProcessNode(aiNode* pAiNode) {
        LOG_TRACK;
		// ��ǰΪ��ǰ�ڵ��е�ÿ���������洢�ռ�
		mMeshes.reserve(mMeshes.size() + pAiNode->mNumMeshes);

		// ��������ǰ�ڵ��е�ÿ������
		for (unsigned int i = 0; i < pAiNode->mNumMeshes; i++) {
			unsigned int mesh_id = pAiNode->mMeshes[i];
			aiMesh* ai_mesh = mAiRoot->mMeshes[mesh_id];
			ProcessMesh(ai_mesh);
		}

		// �ݹ鴦��ǰ�ڵ�������ӽڵ�
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

        vertices.reserve(pAiMesh->mNumVertices);  // Ԥ�ȷ��䶥��洢�ռ�
        indices.reserve(pAiMesh->mNumFaces * 3);  // ÿ���涼��������

        // ȷ����ǰ����Ķ����ʽ
        localFormat.set(0, pAiMesh->HasPositions());                  // �Ƿ��������λ����Ϣ
        localFormat.set(1, pAiMesh->HasNormals());                    // �Ƿ����������Ϣ
        localFormat.set(2, pAiMesh->HasTextureCoords(0));             // �Ƿ������1������������Ϣ
        localFormat.set(3, pAiMesh->HasTextureCoords(1) && pAiMesh->GetNumUVChannels() > 1);  // �Ƿ������2������������Ϣ
        localFormat.set(4, pAiMesh->HasTangentsAndBitangents());      // �Ƿ�������ߺ͸�������Ϣ
        localFormat.set(5, pAiMesh->HasTangentsAndBitangents());      // �Ƿ�������ߺ͸�������Ϣ

        if (mVertNum == 0) {
            mVtxFormat = localFormat;  // ��ʼ�������ʽ�����ǵ�һ��������ֱ��ʹ���䱾�ظ�ʽ
        }

        static bool warned = false;
        if (mVtxFormat != localFormat && !warned) {
            LOG_WARN("��һ�µĶ����ʽ��ĳЩ����ȱ������");
            warned = true;
        }

        mVtxFormat |= localFormat;  // λ�����㣬�ϲ�ÿһ��λ

        // �������񶥵㣬��������������
        for (unsigned int i = 0; i < pAiMesh->mNumVertices; i++) {
            Mesh::Vertex vertex{};
            vertex.boneId = ivec4(-1);  // ������ID��ʼ��Ϊ-1��������0

            if (localFormat.test(0)) {
                aiVector3D& aiPosition = pAiMesh->mVertices[i];
                vertex.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
            }

            if (localFormat.test(1)) {
                aiVector3D& aiNormal = pAiMesh->mNormals[i];
                vertex.normal = glm::vec3(aiNormal.x, aiNormal.y, aiNormal.z);
            }

            if (localFormat.test(2)) {
                aiVector3D& aiUV = pAiMesh->mTextureCoords[0][i];  // ��1����������
                vertex.uv = glm::vec2(aiUV.x, aiUV.y);
            }

            if (localFormat.test(3)) {
                aiVector3D& aiUV2 = pAiMesh->mTextureCoords[1][i];  // ��2����������
                vertex.uv2 = glm::vec2(aiUV2.x, aiUV2.y);
            }

            // ���ߺ͸��������ǳɶԳ��֣��������һ��������һ��Ҳ����
            if (localFormat.test(4) && localFormat.test(5)) {
                aiVector3D& aiTangent = pAiMesh->mTangents[i];
                aiVector3D& aiBinormal = pAiMesh->mBitangents[i];
                vertex.tangent = glm::vec3(aiTangent.x, aiTangent.y, aiTangent.z);
                vertex.binormal = glm::vec3(aiBinormal.x, aiBinormal.y, aiBinormal.z);
            }

            vertices.push_back(vertex);
            mVertNum++;
        }

        // ������������
        for (unsigned int i = 0; i < pAiMesh->mNumFaces; i++) {
            aiFace& triangle = pAiMesh->mFaces[i];
            LOG_ASSERT(triangle.mNumIndices == 3, "�ö���β��������Σ�");

            // Assimp��Ĭ��������OpenGLһ�£���ʱ�룩
            indices.push_back(triangle.mIndices[0]);
            indices.push_back(triangle.mIndices[1]);
            indices.push_back(triangle.mIndices[2]);
            mTriNum++;
        }

        // ��䶥����ȱʧ�Ĺ�������
        if (mIsAnimated) {
            for (unsigned int i = 0; i < pAiMesh->mNumBones; i++) {
                aiBone* ai_bone = pAiMesh->mBones[i];
                std::string name = ai_bone->mName.C_Str();

                auto it = ranges::find_if(mNodes, [&name](const Node& node) { return name == node.mName; });
                LOG_ASSERT(it != mNodes.end(), "��Ч�Ĺ������޷��ڽڵ�㼶���ҵ�ƥ�䣡");

                // ��ȡ�ǳ��������Ը��½ڵ㣨����ͨ��const������`it`���£�
                Node& node = mNodes[it->mNId];  // ����`nodes`ͨ���ڵ�ID��������

                // ��һ�ο�������ʱ��Ϊ��������ID������m2n���󣬷���bid >= 0��ʾ�Ѿ������������и��£�
                // ���ֻ�账���������Ĺ���Ȩ�ء���������������˵���ù�����Ӱ���������еĶ��㣬ͨ�������еĶ����ڵ�

                if (node.mBId < 0) {  // �¹���
                    node.mM2N = AssimpMat2GLM(ai_bone->mOffsetMatrix);
                    node.mBId = mBoneNum++;
                }

                for (int j = 0; j < ai_bone->mNumWeights; j++) {
                    unsigned int vtxId = ai_bone->mWeights[j].mVertexId;
                    const float  weight = ai_bone->mWeights[j].mWeight;
                    LOG_ASSERT(vtxId < vertices.size(), "����ID������Χ��");

                    auto& vertex = vertices[vtxId];
                    bool full = glm::all(glm::greaterThanEqual(vertex.boneId, ivec4(0)));
                    LOG_ASSERT(!full, "���ֳ���ÿ������4�����������鵼�����ã�");

                    for (int k = 0; k < gMaxVtxBones; k++) {
                        if (vertex.boneId[k] < 0) {
                            vertex.boneId[k] = node.mBId;
                            vertex.boneWt[k] = weight;
                            break;  // ÿ������ֻ����һ����λ
                        }
                    }
                }
            }
        }

        auto& mesh = mMeshes.emplace_back(vertices, indices);  // ��ԭ���ƶ������������
        mMesheNum++;

        // ����������ʽ�������
        aiMaterial* ai_material = mAiRoot->mMaterials[pAiMesh->mMaterialIndex];
        ProcessMaterial(ai_material, mesh);
    }

    void Model::ProcessMaterial(aiMaterial* pAiMaterial, const Mesh& mesh) {
        LOG_TRACK;
        LOG_ASSERT(pAiMaterial != nullptr, "�𻵵�Assimp���ݣ�����Ϊ��ָ�룡");

        aiString name;
        if (pAiMaterial->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
            LOG_ERROR("�޷���������Ĳ��ʣ�VAO = {0}��...", mesh.mVAO->getId());
            return;
        }

        std::string matkey{ name.C_Str() };

        // �����ʼ��Ƿ��Ѵ����ڱ��ػ�����
        if (mMaterialsCache.find(matkey) != mMaterialsCache.end()) {
            GLuint matid = mMaterialsCache[matkey];
            mesh.SetMaterialID(matid);  // ������ǰ�Ĳ���ID����Ϊ�����ǹ����
            return;
        }

        // �²��ʣ������ʼ��洢�����ػ�����
        GLuint matid = mesh.mMaterialId;
        mMaterialsCache[matkey] = matid;
    }

    Model::Model(const std::string& filepath, Quality quality, bool animate) : Component(), mIsAnimated(animate){
        LOG_TRACK;
        mFilePath = gResourcePath + filepath;
        mVtxFormat.reset();  // ���ö����ʽ
        mMeshes.clear();      // �����������
        mMaterials.clear();   // ��ղ�������

        // ���õ���ѡ���������ѡ���Ԥ����ѡ��
        // ���� quality: ģ�ͼ��ص�����ѡ��
        unsigned int import_options = static_cast<unsigned int>(quality)
            | aiProcess_FlipUVs                      // ��תUV����
            | aiProcess_Triangulate                  // ������ͼԪת��Ϊ������
            | aiProcess_GenSmoothNormals             // ����ƽ������
            | aiProcess_FindInvalidData              // ������Ч����
            | aiProcess_ValidateDataStructure        // ��֤���ݽṹ
            | aiProcess_CalcTangentSpace             // �������߿ռ�
            | aiProcess_LimitBoneWeights;            // ���ƹ���Ȩ��


        // ���ھ�̬ģ�ͣ���AssimpԤ��ת�����ж��㣨��ʧȥ��νṹ��
        if (!mIsAnimated) {
            import_options |= aiProcess_PreTransformVertices;
        }

        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);  // ���á�ÿ����4����������
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, false);  // ��ֹ��ȡ��������

        LOG_INFO("��ʼ����ģ��: {0}", mFilePath);
        auto startTime = std::chrono::high_resolution_clock::now();

        this->mAiRoot = importer.ReadFile(mFilePath, import_options);  // ʹ��Assimp��ȡģ���ļ�

        if (!mAiRoot || mAiRoot->mRootNode == nullptr || mAiRoot->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            LOG_ERROR("�޷�����ģ��: {0}", mFilePath);
            LOG_ERROR("Assimp����: {0}", importer.GetErrorString());
            return;
        }

        ProcessTree(mAiRoot->mRootNode, -1);  // �ݹ鴦���洢�ڵ��νṹ��Ϣ
        ProcessNode(mAiRoot->mRootNode);      // �ݹ鴦��ÿ���ڵ�

        if (mIsAnimated) {
            unsigned int cnt = ranges::count_if(mNodes, [](const Node& node) { return node.mBId >= 0; });
            LOG_ASSERT(mBoneNum == cnt, "������: �����ظ���ȱʧ�Ĺ���!");
            LOG_ASSERT(mBoneNum <= 150, "������֧�����100������!");
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> loadingTime = endTime - startTime;
        LOG_INFO("ģ�͵������! �ܼ���ʱ��: {0:.2f} ����", loadingTime.count());
        mAiRoot = nullptr;

        // Assimp���������ں�������ʱ�Զ��ͷ�aiScene������޷��ֶ��ͷŸ���Դ���ڶ���delete���ܵ���δ������Ϊ��
        if constexpr (false) {
            delete mAiRoot;
            importer.FreeScene();
        }

        LOG_INFO("����ģ�ͼ��ر���......�������ο���");
        LOG_INFO("-----------------------------------------------------");

        LOG_INFO("����������:     {0}", mMesheNum);
        LOG_INFO("�ܶ�������:     {0:.2f}k ({1})", mVertNum * 0.001f, mVertNum);
        LOG_INFO("������������:   {0:.2f}k ({1})", mTriNum * 0.001f, mTriNum);
        LOG_INFO("-----------------------------------------------------");

        LOG_INFO("�����Ƿ����λ����Ϣ ? [{0}]", mVtxFormat.test(0) ? "Y" : "N");
        LOG_INFO("�����Ƿ���з�����Ϣ ? [{0}]", mVtxFormat.test(1) ? "Y" : "N");
        LOG_INFO("�����Ƿ����UV��1��Ϣ ? [{0}]", mVtxFormat.test(2) ? "Y" : "N");
        LOG_INFO("�����Ƿ����UV��2��Ϣ ? [{0}]", mVtxFormat.test(3) ? "Y" : "N");
        LOG_INFO("�����Ƿ��������/��������Ϣ ? [{0}]", mVtxFormat.test(4) ? "Y" : "N");
        LOG_INFO("-----------------------------------------------------");

        std::string allMtls = "������";

        if (!mMaterialsCache.empty()) {
            auto it = mMaterialsCache.begin();
            allMtls = it->first + " (id = " + std::to_string(it->second) + ")";
            std::advance(it, 1);
            while (it != mMaterialsCache.end()) {
                allMtls += (", " + it->first + " (id = " + std::to_string(it->second) + ")");
                std::advance(it, 1);
            }
        }

        LOG_INFO("�ڲ�����: {0}", allMtls);
        LOG_INFO("-----------------------------------------------------");
    }

    Material& Model::SetMaterial(const std::string& matkey, std::shared_ptr<Material>&& material) {
        LOG_TRACK;
        LOG_ASSERT(mMaterialsCache.count(matkey) > 0, "��Ч�Ĳ��ʼ�: {0}", matkey);

        // ע�⣬��������material�������ʲ���������ԭʼasset_ref����ʱ��ֵ������
        // ������ǿ���ֱ���ƶ�����ָ��λ��

        GLuint matid = mMaterialsCache[matkey];
        mMaterials.insert_or_assign(matid, std::move(material));
        return mMaterials.at(matid);
    }

    void Model::AttachMotion(const std::string& filepath) {
        LOG_TRACK;
        if (!mIsAnimated) {
            LOG_ERROR("�޷����������ӵ�ģ�ͣ�ģ�ͱ����Ƕ�����");
        }

        // ����ѡ����ڿ���Assimp����ģ��ʱ�Ĵ����衣
        // ��Щѡ�����ڴ���ģ�͵�UV��ת�����ǻ�������ƽ�����ߡ�������Ч���ݡ���֤���ݽṹ���������߿ռ�����ƹ���Ȩ�ء�
        // ע�⣺���ض���ʱ���������aiProcess_PreTransformVertices��־��
        const unsigned int import_options = 0
            | aiProcess_FlipUVs
            | aiProcess_Triangulate
            | aiProcess_GenSmoothNormals
            | aiProcess_FindInvalidData
            | aiProcess_ValidateDataStructure
            | aiProcess_CalcTangentSpace
            | aiProcess_LimitBoneWeights
            // | aiProcess_PreTransformVertices  // ������ô˱�־�Լ��ض���
            ;

        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, true);

        LOG_INFO("��ʼ���ض���: {0}", filepath);
        const aiScene* scene = importer.ReadFile(filepath, import_options);

        // ע�⣬�������ǲ���Ҫ���`scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE`
        // ��Ϊ�ļ����Խ��������������������㡢���񣩣���ʱ���������ǲ������ģ���������Ȼ��Ч

        if (!scene || scene->mRootNode == nullptr) {
            LOG_ERROR("���ض���ʧ��: {0}", filepath);
            LOG_ERROR("Assimp����: {0}", importer.GetErrorString());
            return;
        }

        mAnimation = std::make_unique<Animation>(scene, this);
    }
}
