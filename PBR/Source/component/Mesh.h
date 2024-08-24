#pragma once
#include "../component/Component.h"
#include "../asset/VAO.h"
#include "../asset/Buffer.h"
#include <memory>
using namespace asset;
namespace component {
	// ö�����ͣ���ʾ����Ļ�������������
	enum class Primitive : uint8_t {
		Sphere, Cube, Plane, Quad2D, Torus, Capsule, Tetrahedron
	};

	class Mesh : public Component{
	public:
		// ���񶥵�ṹ��
		struct Vertex {
			glm::vec3  position;	// ����λ��
			glm::vec3  normal;		// ���㷨��
			glm::vec2  uv;			// ��������1
			glm::vec2  uv2;			// ��������2
			glm::vec3  tangent;		// ����
			glm::vec3  binormal;	// ������
			glm::ivec4 boneId;		// ����ID��ÿ������������4������
			glm::vec4  boneWt;		// ÿ��������Ȩ��
		};
		static_assert(sizeof(Vertex) == 20 * sizeof(float) + 4 * sizeof(int));  // ȷ������ṹ���С��ȷ

	public:
		Mesh(Primitive object);
		// ���캯����ʹ�ø�����VAO�Ͷ���������������
		Mesh(std::shared_ptr<asset::VAO> vao, size_t n_verts);

		// ���캯����ʹ�ø����Ķ�����������ݴ�������
		Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

		// ���캯����ʹ�ø�����������Դ���ô�������ĸ���
		Mesh(const std::shared_ptr<Mesh>& mesh_asset);

		void Draw() const;
		void SetMaterialID(GLuint mid) const;
		static void DrawQuad();
		static void DrawGrid();

	private:
		// ����һ����������
		void CreateSphere(float radius = 1.0f);
		void CreateCube(float size = 1.0f);
		void CreatePlane(float size = 10.0f);
		void Create2DQuad(float size = 1.0f);
		void CreateTorus(float R = 1.5f, float r = 0.5f);
		void CreateCapsule(float a = 2.0f, float r = 1.0f);
		void CreatePyramid(float s = 2.0f);

		void CreateBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

		
	
	public:
		size_t mVerts;	// ������
		size_t mTris;	// ��������
		std::shared_ptr<VAO> mVAO;		// �����������
		std::shared_ptr<VBO> mVBO;		// ���㻺��������
		std::shared_ptr<IBO> mIBO;		// ��������������
		mutable GLuint mMaterialId;
	};
}

