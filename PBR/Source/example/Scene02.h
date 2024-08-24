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
		void PrecomputeIBL(const std::string& hdri);  // 预计算图像基础光照（IBL）函数
		void SetBuffers();                         // 设置缓冲区函数
		void SetMaterial(Material& pbrMat, bool cloth, bool textured);  // 设置材质函数

	private:
		Entity mCamera;            // 相机实体
		Entity mSkybox;            // 天空盒实体
		Entity mDirectLight;      // 直射光实体
		Entity cloth_model;       // 静态布料模型实体

		std::shared_ptr<Texture> mMapIrradiance;   // 漫反射辐照度贴图引用
		std::shared_ptr<Texture> mMapPrefiltered;  // 预过滤环境贴图引用
		std::shared_ptr<Texture> mMapBRDF_LUT;         // 环境BRDF贴图引用

		// 动态布料网格通过模拟生成
		std::unique_ptr<VAO>  mVAO;     // 布料VAO
		std::unique_ptr<IBO>  mIBO;     // 布料IBO
		std::unique_ptr<VBO>  mVBO;     // 纹理坐标VBO（静态）
		std::unique_ptr<SSBO> mPos[2];  // 位置VBO（作为SSBO更新）
		std::unique_ptr<SSBO> mVel[2];  // 速度SSBO
		std::unique_ptr<SSBO> mNormal;  // 法线VBO（作为SSBO更新）

		std::vector<vec4> mInitPos;    // 顶点初始位置数组
		std::vector<vec4> mInitVel;    // 顶点初始速度数组（vec4(0.0)）
		std::vector<vec2> mTextureCoord;   // 顶点纹理坐标数组（静态）
		std::vector<uint> mIndices;     // 索引数组
	};
}