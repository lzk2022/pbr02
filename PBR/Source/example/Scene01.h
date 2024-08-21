#pragma once
#include "../scene/Scene.h"
#include "../scene/Entity.h"

namespace scene {
	class Scene01 : public Scene {
	public:
		using Scene::Scene;		// �̳л��๹�캯��

		// ��cpp�ļ�����д��3����������Ⱦ�µĳ���
		void Init() override;
		void OnSceneRender() override;
		void OnImGuiRender() override;

	public:
        Entity mCamera;          // �����ʵ��
        Entity mSkybox;          // ��պ�ʵ��
        Entity mDirectLight;    // ֱ���ʵ��
        Entity mOrbitLight;     // �����ʵ��
        Entity mPointLights[28];  // ���Դ���飬���28��

        Entity mSphere;          // ����ʵ��
        Entity mPlane;           // ƽ��ʵ��
        Entity mRunestone;       // ����ʯʵ��

        std::unique_ptr<SSBO> mPlColor;    // ���Դ��ɫ����ʱ������
        std::unique_ptr<SSBO> mPlPosition; // ���Դλ�õ���ʱ������
        std::unique_ptr<SSBO> mPlRange;    // ���Դ��Χ����ʱ������
        std::unique_ptr<SSBO> mPlIndex;    // ���Դ��������ʱ������

        std::shared_ptr<Texture> mIrradianceMap;   // ���ն���ͼ����Դ����
        std::shared_ptr<Texture> mPrefilteredMap;  // Ԥ���˻�����ͼ����Դ����
        std::shared_ptr<Texture> mBRDF_LUT;         // BRDF���ұ����Դ����

	private:
		void PrecomputeIBL(const std::string& hdri);
        void SetMaterial(Material& pbrMaterial, int id);  // ���ò���
        void SetPLBuffers();                     // ���õ��Դ������
        void UpdatePLColors();  // ���µ��Դ��ɫ
	};
}

