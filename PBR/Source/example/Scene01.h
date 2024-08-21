#pragma once
#include "../scene/Scene.h"
#include "../scene/Entity.h"

namespace scene {
	class Scene01 : public Scene {
	public:
		using Scene::Scene;		// 继承基类构造函数

		// 在cpp文件中重写这3个函数来渲染新的场景
		void Init() override;
		void OnSceneRender() override;
		void OnImGuiRender() override;

	public:
        Entity mCamera;          // 摄像机实体
        Entity mSkybox;          // 天空盒实体
        Entity mDirectLight;    // 直射光实体
        Entity mOrbitLight;     // 轨道光实体
        Entity mPointLights[28];  // 点光源数组，最多28个

        Entity mSphere;          // 球体实体
        Entity mPlane;           // 平面实体
        Entity mRunestone;       // 符文石实体

        std::unique_ptr<SSBO> mPlColor;    // 点光源颜色的临时缓冲区
        std::unique_ptr<SSBO> mPlPosition; // 点光源位置的临时缓冲区
        std::unique_ptr<SSBO> mPlRange;    // 点光源范围的临时缓冲区
        std::unique_ptr<SSBO> mPlIndex;    // 点光源索引的临时缓冲区

        std::shared_ptr<Texture> mIrradianceMap;   // 辐照度贴图的资源引用
        std::shared_ptr<Texture> mPrefilteredMap;  // 预过滤环境贴图的资源引用
        std::shared_ptr<Texture> mBRDF_LUT;         // BRDF查找表的资源引用

	private:
		void PrecomputeIBL(const std::string& hdri);
        void SetMaterial(Material& pbrMaterial, int id);  // 设置材质
        void SetPLBuffers();                     // 设置点光源缓冲区
        void UpdatePLColors();  // 更新点光源颜色
	};
}

