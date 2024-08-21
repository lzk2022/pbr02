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

		// �жϵ�ǰ�ڵ��Ƿ�Ϊ�����ڵ�
		bool IsBone() const;
		// �жϵ�ǰ�ڵ��Ƿ��Ƕ����ڵ㣨���ǹ����ڵ㲢���ܶ���Ӱ�죩
		bool Animated() const;

	public:
		int mNId = -1;    // �ڵ�ID��������ڵ���0
		int mPId = -1;    // ���ڵ�ID�����ڸ��ڵ�Ϊ-1
		int mBId = -1;    // ����ID������ڵ㲻�ǹ����ڵ���Ϊ-1
		bool mAlive = 0;  // �Ƿ��ǹ����ڵ�����ͨ��Ӱ��

		std::string mName;   // �ڵ�����
		glm::mat4 mN2P;      // �ڵ�ռ䵽���ڵ�ռ�ı任��������ڸ��ڵ�ľֲ��任��
		glm::mat4 mM2N;      // ģ�Ϳռ䣨�����ƣ����ڵ�ռ䣨�����ռ䣩�ı任���󣬽������ڹ����ڵ�
		glm::mat4 mN2M;      // �ڵ�ռ䵽ģ�Ϳռ�ı任��������ʱ���£�����ڵ㲻�������
	};
	enum class Quality : uint32_t {  // ��������Ԥ��
		Auto = 0x0,
		Low = aiProcessPreset_TargetRealtime_Fast,
		Medium = aiProcessPreset_TargetRealtime_Quality,
		High = aiProcessPreset_TargetRealtime_MaxQuality
	};

	/*************************************************Model****************************************************/
	class Model : public Component {
	public:
		// �ݹ鴦��ڵ����ṹ��������ȱ������洢ÿ���ڵ�Ĳ����Ϣ
		void ProcessTree(aiNode* aiNode, int parent);
		// �ݹ鴦��ڵ㣬����ǰ�ڵ��µ���������
		void ProcessNode(aiNode* aiNode);
		// �����������ݣ����춥������������Mesh����
		void ProcessMesh(aiMesh* aiMesh);
		// ����������ݣ������ʹؼ�����ID����ӳ�䣬����������������
		void ProcessMaterial(aiMaterial* aiMaterial, const Mesh& mesh);
		// ���캯��������ģ���ļ�����ʼ��ģ�Ͷ���
		Model(const std::string& filepath, Quality quality, bool animate = false);
		// ���Ӷ��������ز�����ģ�͵Ķ�������
		void AttachMotion(const std::string& filepath);
		Material& SetMaterial(const std::string& matkey, std::shared_ptr<Material>&& material);

	public:
		unsigned int mNodeNum = 0;     // �ڵ�����
		unsigned int mBoneNum = 0;     // ��������
		unsigned int mMesheNum = 0;    // ��������
		unsigned int mVertNum = 0;     // ��������
		unsigned int mTriNum = 0;      // ����������
		bool mIsAnimated = false;        // �Ƿ��������

		std::vector<Node> mNodes;             // �ڵ��б�
		std::vector<Mesh> mMeshes;            // �����б�
		std::unordered_map<GLuint, Material> mMaterials;  // ����ӳ�䣬��������ID�Ͳ��ʶ���
		std::unique_ptr<Animation> mAnimation;  // ���������Ψһָ��

	private:
		const aiScene* mAiRoot = nullptr;  // Assimp�������ڵ�ָ��
		std::bitset<6> mVtxFormat;         // �����ʽλ������������ģ�Ͷ��������
		std::unordered_map<std::string, GLuint> mMaterialsCache;  // ���ʻ��棬���ڴ洢���ʹؼ��ֺ�ID��ӳ��
		std::string mFilePath;

	};

}
