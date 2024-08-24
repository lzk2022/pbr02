#include "Mesh.h"
#include "../utils/Log.h"

#include <glm/glm.hpp>                   // GLM��OpenGL��ѧ��
#include <glm/ext.hpp>                   // GLM��OpenGL��ѧ�� ��չ
//#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
//#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ
//#include <glm/gtx/transform.hpp>         // GLM��OpenGL��ѧ�� �任
//#include <glm/gtc/type_ptr.hpp>          // GLM��OpenGL��ѧ�� ָ��
//#include <glm/gtx/string_cast.hpp>       // GLM��OpenGL��ѧ�� �ַ���ת��
//#include <glm/gtc/matrix_transform.hpp>  // GLM��OpenGL��ѧ�� ����任
//#include <glm/gtx/perpendicular.hpp>     // GLM��OpenGL��ѧ�� ��ֱ����

using namespace glm;

namespace component {

    // ��������ƫ����
    static const std::vector<GLint> gVaOffset = {  
        offsetof(Mesh::Vertex, position),    // ����λ��ƫ����
        offsetof(Mesh::Vertex, normal),      // ���㷨��ƫ����
        offsetof(Mesh::Vertex, uv),          // ����UV����ƫ����
        offsetof(Mesh::Vertex, uv2),         // �ڶ���UV����ƫ����
        offsetof(Mesh::Vertex, tangent),     // ��������ƫ����
        offsetof(Mesh::Vertex, binormal),    // ������ƫ����
        offsetof(Mesh::Vertex, boneId),      // ����IDƫ����
        offsetof(Mesh::Vertex, boneWt)       // ����Ȩ��ƫ����
    };

    static const std::vector<GLint> gVaSize = { 3, 3, 2, 2, 3, 3, 4, 4 };  // �������Դ�С
    static std::unique_ptr<VAO> gInternalVAO;  // �ڲ�ʹ�õĶ����������VAO������ʱ�ʲ�


	Mesh::Mesh(Primitive object) : Component()
	{
		LOG_TRACK;
        switch (object) {
        case Primitive::Sphere:      CreateSphere();    break;  // ������������
        case Primitive::Cube:        CreateCube();      break;  // ��������������
        case Primitive::Plane:       CreatePlane();     break;  // ����ƽ������
        case Primitive::Quad2D:      Create2DQuad();    break;  // ����2D�ı�������
        case Primitive::Torus:       CreateTorus();     break;  // ������������
        case Primitive::Capsule:     CreateCapsule();   break;  // ��������������
        case Primitive::Tetrahedron: CreatePyramid();   break;  // ��������������
        default: { LOG_ERROR("�޷���������δ����Ļ�����״"); }
        }
	}

    Mesh::Mesh(std::shared_ptr<VAO> vao, size_t numVerts)
        : Component(), mVAO(vao), mVerts(numVerts), mTris(numVerts / 3) {
        LOG_TRACK;
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) : Component() {
        CreateBuffers(vertices, indices);
        mMaterialId = mVAO->getId();  // ֻ���ڼ����ⲿģ��ʱ�Ż���ô˹��캯��
    }

    Mesh::Mesh(const std::shared_ptr<Mesh>& mesh) : Mesh(*mesh) {}  // ���ø��ƹ��캯��


    void Mesh::Draw() const
    {
        LOG_TRACK;
        mVAO->Draw(GL_TRIANGLES, mTris * 3);         // ʹ�������λ���ģʽ�����������
    }
    void Mesh::CreateSphere(float radius)
    {
        LOG_TRACK;
        constexpr float PI = glm::pi<float>();          // ��
        constexpr float PI_2 = glm::half_pi<float>();   // ��/2

        // Ĭ�ϵ�LOD = 100x100�������С
        unsigned int nRows = 100;                               // ����
        unsigned int nCols = 100;                               // ����
        unsigned int nVerts = (nRows + 1) * (nCols + 1);        // ������
        unsigned int nTris = nRows * nCols * 2;                 // ��������

        std::vector<Vertex> vertices;   // ��������
        std::vector<GLuint> indices;    // ��������
        vertices.reserve(nVerts);
        indices.reserve(nTris * 3);

        // ������������
        for (unsigned int col = 0; col <= nCols; ++col) {
            for (unsigned int row = 0; row <= nRows; ++row) {
                // δ���ŵ�UV���귶Χ��[0, 1]֮��
                float u = static_cast<float>(col) / nCols;
                float v = static_cast<float>(row) / nRows;

                float theta = PI * v - PI_2;                // ά�ȽǶȷ�Χ��[-PI/2, PI/2]�����ϼ�������
                float phi = PI * 2 * u;                     // ���ȽǶȷ�Χ��[0, 2PI]���Ƴ��Ȧ

                float x = cos(phi) * cos(theta);            // x����
                float y = sin(theta);                       // y����
                float z = sin(phi) * cos(theta) * (-1);     // z����

                // ���ڵ�λ���壬��ԭ��Ϊ���ģ����ߵ���λ��
                // �������Ƿ�����γ�ȽǶȣ�+theta����ת90��
                theta += PI_2;
                float r = cos(phi) * cos(theta);            // �����ߵ�x����
                float s = sin(theta);                       // �����ߵ�y����
                float t = sin(phi) * cos(theta) * (-1);     // �����ߵ�z����

                Vertex vertex{};
                vertex.position = vec3(x, y, z) * radius;   // ����λ��
                vertex.normal = vec3(x, y, z);              // ���㷨��
                vertex.uv = vec2(u, v);                     // ����UV����
                vertex.binormal = vec3(r, s, t);            // ���㸱����
                vertex.tangent = glm::cross(vertex.binormal, vertex.normal);  // ��������

                vertices.push_back(vertex);
            }
        }

        // ��������������
        for (unsigned int col = 0; col < nCols; ++col) {
            for (unsigned int row = 0; row < nRows; ++row) {
                auto index = col * (nRows + 1);

                // ��ʱ���˳�����������ζ�������
                indices.push_back(index + row + 1);
                indices.push_back(index + row);
                indices.push_back(index + row + 1 + nRows);

                // ��ʱ���˳�����������ζ�������
                indices.push_back(index + row + 1 + nRows + 1);
                indices.push_back(index + row + 1);
                indices.push_back(index + row + 1 + nRows);
            }
        }

       CreateBuffers(vertices, indices);  // �������㻺����������������
    }
    void Mesh::CreateCube(float size)
    {
        constexpr int verticeNum = 24;  // ����ֻ��Ҫ24�����������ǻ�6����
        constexpr int stride = 8;       // ÿ���������ݵĲ���Ϊ8��3��λ�� + 3������ + 2��UV���꣩

        std::vector<Vertex> vertices;   // ��������
        vertices.reserve(verticeNum);

        // �����嶥������
        static const float data[] = {
            // ----position----    -------normal------    ----uv----
            -1.0f, -1.0f, -1.0f,   +0.0f, -1.0f, +0.0f,   0.0f, 0.0f,
            -1.0f, -1.0f, +1.0f,   +0.0f, -1.0f, +0.0f,   0.0f, 1.0f,
            +1.0f, -1.0f, +1.0f,   +0.0f, -1.0f, +0.0f,   1.0f, 1.0f,
            +1.0f, -1.0f, -1.0f,   +0.0f, -1.0f, +0.0f,   1.0f, 0.0f,
            -1.0f, +1.0f, -1.0f,   +0.0f, +1.0f, +0.0f,   1.0f, 0.0f,
            -1.0f, +1.0f, +1.0f,   +0.0f, +1.0f, +0.0f,   1.0f, 1.0f,
            +1.0f, +1.0f, +1.0f,   +0.0f, +1.0f, +0.0f,   0.0f, 1.0f,
            +1.0f, +1.0f, -1.0f,   +0.0f, +1.0f, +0.0f,   0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,   +0.0f, +0.0f, -1.0f,   0.0f, 0.0f,
            -1.0f, +1.0f, -1.0f,   +0.0f, +0.0f, -1.0f,   0.0f, 1.0f,
            +1.0f, +1.0f, -1.0f,   +0.0f, +0.0f, -1.0f,   1.0f, 1.0f,
            +1.0f, -1.0f, -1.0f,   +0.0f, +0.0f, -1.0f,   1.0f, 0.0f,
            -1.0f, -1.0f, +1.0f,   +0.0f, +0.0f, +1.0f,   0.0f, 0.0f,
            -1.0f, +1.0f, +1.0f,   +0.0f, +0.0f, +1.0f,   0.0f, 1.0f,
            +1.0f, +1.0f, +1.0f,   +0.0f, +0.0f, +1.0f,   1.0f, 1.0f,
            +1.0f, -1.0f, +1.0f,   +0.0f, +0.0f, +1.0f,   1.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,   -1.0f, +0.0f, +0.0f,   0.0f, 0.0f,
            -1.0f, -1.0f, +1.0f,   -1.0f, +0.0f, +0.0f,   0.0f, 1.0f,
            -1.0f, +1.0f, +1.0f,   -1.0f, +0.0f, +0.0f,   1.0f, 1.0f,
            -1.0f, +1.0f, -1.0f,   -1.0f, +0.0f, +0.0f,   1.0f, 0.0f,
            +1.0f, -1.0f, -1.0f,   +1.0f, +0.0f, +0.0f,   0.0f, 0.0f,
            +1.0f, -1.0f, +1.0f,   +1.0f, +0.0f, +0.0f,   0.0f, 1.0f,
            +1.0f, +1.0f, +1.0f,   +1.0f, +0.0f, +0.0f,   1.0f, 1.0f,
            +1.0f, +1.0f, -1.0f,   +1.0f, +0.0f, +0.0f,   1.0f, 0.0f
        };

        // ������������
        for (unsigned int i = 0; i < verticeNum; i++) {
            unsigned int offset = i * stride;

            Vertex vertex{};
            vertex.position = vec3(data[offset + 0], data[offset + 1], data[offset + 2]) * size;    // ���ö���λ�ã������ŵ�ָ����С
            vertex.normal = vec3(data[offset + 3], data[offset + 4], data[offset + 5]);             // ���ö��㷨��
            vertex.uv = vec2(data[offset + 6], data[offset + 7]);                                   // ���ö���UV����

            vertices.push_back(vertex);
        }

        // ��ʱ���˳����������������
        std::vector<GLuint> indices{
            +0, +2, +1,   +0, +3, +2,   +4, +5, +6,
            +4, +6, +7,   +8, +9, 10,   +8, 10, 11,
            12, 15, 14,   12, 14, 13,   16, 17, 18,
            16, 18, 19,   20, 23, 22,   20, 22, 21
        };

        CreateBuffers(vertices, indices);  // �������㻺����������������
    }

    void Mesh::CreatePlane(float size){
        LOG_TRACK;
        constexpr int nVertices = 8;   // ���񶥵���
        constexpr int stride = 8;       // ÿ���������ݵĲ���Ϊ8��3��λ�� + 3������ + 2��UV���꣩

        // ƽ�涥������
        static const float data[] = {
            // ---position----    ------normal-----    ----uv----
            -1.0f, 0.0f, +1.0f,   0.0f, +1.0f, 0.0f,   0.0f, 0.0f,
            +1.0f, 0.0f, +1.0f,   0.0f, +1.0f, 0.0f,   1.0f, 0.0f,
            +1.0f, 0.0f, -1.0f,   0.0f, +1.0f, 0.0f,   1.0f, 1.0f,
            -1.0f, 0.0f, -1.0f,   0.0f, +1.0f, 0.0f,   0.0f, 1.0f,
            -1.0f, 0.0f, +1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
            +1.0f, 0.0f, +1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
            +1.0f, 0.0f, -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
            -1.0f, 0.0f, -1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f
        };

        std::vector<Vertex> vertices;   // ��������
        vertices.reserve(nVertices);

        // ������������
        for (unsigned int i = 0; i < nVertices; i++) {
            unsigned int offset = i * stride;

            Vertex vertex{};
            vertex.position = vec3(data[offset + 0], data[offset + 1], data[offset + 2]) * size;  // ���ö���λ�ã������ŵ�ָ����С
            vertex.normal = vec3(data[offset + 3], data[offset + 4], data[offset + 5]);          // ���ö��㷨��
            vertex.uv = vec2(data[offset + 6], data[offset + 7]);                             // ���ö���UV����

            vertices.push_back(vertex);
        }

        // ��ʱ���˳����������������
        std::vector<GLuint> indices{ 0, 1, 2, 2, 3, 0, 6, 5, 4, 4, 7, 6 };

        CreateBuffers(vertices, indices);  // �������㻺����������������
    }

    void Mesh::Create2DQuad(float size){
        LOG_TRACK;
    }

    void Mesh::CreateTorus(float R, float r){
        LOG_TRACK;
    }

    void Mesh::CreateCapsule(float a, float r){
        LOG_TRACK;
    }

    void Mesh::CreatePyramid(float s){
        LOG_TRACK;
    }

    void Mesh::CreateBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices){
        LOG_TRACK;
        mVAO = std::make_shared<VAO>();
        mVBO = std::make_shared<VBO>(vertices.size() * sizeof(Vertex), &vertices[0]);
        mIBO = std::make_shared<IBO>(indices.size() * sizeof(GLuint), &indices[0]);

        for (GLuint i = 0; i < 8; i++) {
            GLenum vaType = i == 6 ? GL_INT : GL_FLOAT;
            mVAO->SetVBO(mVBO->Id(), i, gVaOffset[i], gVaSize[i], sizeof(Vertex), vaType);
        }
        mVAO->SetIBO(mIBO->Id());

        mVerts = vertices.size();                   // ���ö�������
        mTris = indices.size() / 3;                 // ��������������
    }

    void Mesh::SetMaterialID(GLuint mid) const{
        mMaterialId = mid;
    }

    void Mesh::DrawQuad()
    {
        // �޻�����Ⱦ�������ǻ���һ���ı��Σ�����Ҫʹ���κ���������
        // �ο�����: https://trass3r.github.io/coding/2019/09/11/bufferless-rendering.html
        // �ο�����: https://stackoverflow.com/a/59739538/10677643

        if (gInternalVAO == nullptr) {
            gInternalVAO = std::make_unique<VAO>();  // ����ڲ�VAOΪ�գ����װһ���µ�VAO
        }

        gInternalVAO->Bind();  // ���ڲ�VAO
        glDrawArrays(GL_TRIANGLES, 0, 3);  // ���������Σ�ʹ��3�����㣬����3�ζ�����ɫ��
    }

    void Mesh::DrawGrid()
    {
        if (gInternalVAO == nullptr) {
            gInternalVAO = std::make_unique<VAO>();  // ����ڲ�VAOΪ�գ����װһ���µ�VAO
        }

        gInternalVAO->Bind();  // ���ڲ�VAO
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);  // ���������Σ�ʹ��6�����㣬����6��ʵ��
    }
 
}
