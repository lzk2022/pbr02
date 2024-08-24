#pragma once
#include "../scene/Scene.h"

namespace scene {
	class Scene02 : public Scene {
	public:
		using Scene::Scene;
		void Init() override;
		void OnSceneRender() override;
		void OnImGuiRender() override;

	private:
		void PrecomputeIBL(const std::string& hdri);  // Ԥ����ͼ��������գ�IBL������
		void SetBuffers();                         // ���û���������
		void SetMaterial(Material& pbrMat, bool cloth, bool textured);  // ���ò��ʺ���

	private:
		Entity mCamera;            // ���ʵ��
		Entity mSkybox;            // ��պ�ʵ��
		Entity mDirectLight;      // ֱ���ʵ��
		Entity cloth_model;       // ��̬����ģ��ʵ��

		std::shared_ptr<Texture> mMapIrradiance;   // ��������ն���ͼ����
		std::shared_ptr<Texture> mMapPrefiltered;  // Ԥ���˻�����ͼ����
		std::shared_ptr<Texture> mMapBRDF_LUT;         // ����BRDF��ͼ����

		// ��̬��������ͨ��ģ������
		std::unique_ptr<VAO>  mVAO;     // ����VAO
		std::unique_ptr<IBO>  mIBO;     // ����IBO
		std::unique_ptr<VBO>  mVBO;     // ��������VBO����̬��
		std::unique_ptr<SSBO> mPos[2];  // λ��VBO����ΪSSBO���£�
		std::unique_ptr<SSBO> mVel[2];  // �ٶ�SSBO
		std::unique_ptr<SSBO> mNormal;  // ����VBO����ΪSSBO���£�

		std::vector<vec4> mInitPos;    // �����ʼλ������
		std::vector<vec4> mInitVel;    // �����ʼ�ٶ����飨vec4(0.0)��
		std::vector<vec2> mTextureCoord;   // ���������������飨��̬��
		std::vector<uint> mIndices;     // ��������
	};
}