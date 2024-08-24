#pragma once
#include "../component/Component.h"
#include "../asset/VAO.h"
#include "../asset/Buffer.h"
#include <memory>
using namespace asset;
namespace component {
	// 枚举类型，表示网格的基本几何体类型
	enum class Primitive : uint8_t {
		Sphere, Cube, Plane, Quad2D, Torus, Capsule, Tetrahedron
	};

	class Mesh : public Component{
	public:
		// 网格顶点结构体
		struct Vertex {
			glm::vec3  position;	// 顶点位置
			glm::vec3  normal;		// 顶点法线
			glm::vec2  uv;			// 纹理坐标1
			glm::vec2  uv2;			// 纹理坐标2
			glm::vec3  tangent;		// 切线
			glm::vec3  binormal;	// 副切线
			glm::ivec4 boneId;		// 骨骼ID，每个顶点最多关联4个骨骼
			glm::vec4  boneWt;		// 每个骨骼的权重
		};
		static_assert(sizeof(Vertex) == 20 * sizeof(float) + 4 * sizeof(int));  // 确保顶点结构体大小正确

	public:
		Mesh(Primitive object);
		// 构造函数，使用给定的VAO和顶点数量创建网格
		Mesh(std::shared_ptr<asset::VAO> vao, size_t n_verts);

		// 构造函数，使用给定的顶点和索引数据创建网格
		Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

		// 构造函数，使用给定的网格资源引用创建网格的副本
		Mesh(const std::shared_ptr<Mesh>& mesh_asset);

		void Draw() const;
		void SetMaterialID(GLuint mid) const;
		static void DrawQuad();
		static void DrawGrid();

	private:
		// 创建一个球体网格
		void CreateSphere(float radius = 1.0f);
		void CreateCube(float size = 1.0f);
		void CreatePlane(float size = 10.0f);
		void Create2DQuad(float size = 1.0f);
		void CreateTorus(float R = 1.5f, float r = 0.5f);
		void CreateCapsule(float a = 2.0f, float r = 1.0f);
		void CreatePyramid(float s = 2.0f);

		void CreateBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

		
	
	public:
		size_t mVerts;	// 顶点数
		size_t mTris;	// 三角形数
		std::shared_ptr<VAO> mVAO;		// 顶点数组对象
		std::shared_ptr<VBO> mVBO;		// 顶点缓冲区对象
		std::shared_ptr<IBO> mIBO;		// 索引缓冲区对象
		mutable GLuint mMaterialId;
	};
}

