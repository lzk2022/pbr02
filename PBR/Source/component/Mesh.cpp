#include "Mesh.h"
#include "../utils/Log.h"

#include <glm/glm.hpp>                   // GLM：OpenGL数学库
#include <glm/ext.hpp>                   // GLM：OpenGL数学库 扩展
//#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
//#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展
//#include <glm/gtx/transform.hpp>         // GLM：OpenGL数学库 变换
//#include <glm/gtc/type_ptr.hpp>          // GLM：OpenGL数学库 指针
//#include <glm/gtx/string_cast.hpp>       // GLM：OpenGL数学库 字符串转换
//#include <glm/gtc/matrix_transform.hpp>  // GLM：OpenGL数学库 矩阵变换
//#include <glm/gtx/perpendicular.hpp>     // GLM：OpenGL数学库 垂直向量

using namespace glm;

namespace component {

    // 顶点属性偏移量
    static const std::vector<GLint> gVaOffset = {  
        offsetof(Mesh::Vertex, position),    // 顶点位置偏移量
        offsetof(Mesh::Vertex, normal),      // 顶点法线偏移量
        offsetof(Mesh::Vertex, uv),          // 顶点UV坐标偏移量
        offsetof(Mesh::Vertex, uv2),         // 第二组UV坐标偏移量
        offsetof(Mesh::Vertex, tangent),     // 切线向量偏移量
        offsetof(Mesh::Vertex, binormal),    // 副法线偏移量
        offsetof(Mesh::Vertex, boneId),      // 骨骼ID偏移量
        offsetof(Mesh::Vertex, boneWt)       // 骨骼权重偏移量
    };

    static const std::vector<GLint> gVaSize = { 3, 3, 2, 2, 3, 3, 4, 4 };  // 顶点属性大小
    static std::unique_ptr<VAO> gInternalVAO;  // 内部使用的顶点数组对象（VAO）的临时资产


	Mesh::Mesh(Primitive object) : Component()
	{
		LOG_TRACK;
        switch (object) {
        case Primitive::Sphere:      CreateSphere();    break;  // 创建球体网格
        case Primitive::Cube:        CreateCube();      break;  // 创建立方体网格
        case Primitive::Plane:       CreatePlane();     break;  // 创建平面网格
        case Primitive::Quad2D:      Create2DQuad();    break;  // 创建2D四边形网格
        case Primitive::Torus:       CreateTorus();     break;  // 创建环面网格
        case Primitive::Capsule:     CreateCapsule();   break;  // 创建胶囊体网格
        case Primitive::Tetrahedron: CreatePyramid();   break;  // 创建金字塔网格
        default: { LOG_ERROR("无法构造网格，未定义的基本形状"); }
        }
	}

    Mesh::Mesh(std::shared_ptr<VAO> vao, size_t numVerts)
        : Component(), mVAO(vao), mVerts(numVerts), mTris(numVerts / 3) {
        LOG_TRACK;
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) : Component() {
        CreateBuffers(vertices, indices);
        mMaterialId = mVAO->getId();  // 只有在加载外部模型时才会调用此构造函数
    }

    Mesh::Mesh(const std::shared_ptr<Mesh>& mesh) : Mesh(*mesh) {}  // 调用复制构造函数


    void Mesh::Draw() const
    {
        LOG_TRACK;
        mVAO->Draw(GL_TRIANGLES, mTris * 3);         // 使用三角形绘制模式绘制网格对象
    }
    void Mesh::CreateSphere(float radius)
    {
        LOG_TRACK;
        constexpr float PI = glm::pi<float>();          // π
        constexpr float PI_2 = glm::half_pi<float>();   // π/2

        // 默认的LOD = 100x100的网格大小
        unsigned int nRows = 100;                               // 行数
        unsigned int nCols = 100;                               // 列数
        unsigned int nVerts = (nRows + 1) * (nCols + 1);        // 顶点数
        unsigned int nTris = nRows * nCols * 2;                 // 三角形数

        std::vector<Vertex> vertices;   // 顶点数组
        std::vector<GLuint> indices;    // 索引数组
        vertices.reserve(nVerts);
        indices.reserve(nTris * 3);

        // 创建顶点数据
        for (unsigned int col = 0; col <= nCols; ++col) {
            for (unsigned int row = 0; row <= nRows; ++row) {
                // 未缩放的UV坐标范围在[0, 1]之间
                float u = static_cast<float>(col) / nCols;
                float v = static_cast<float>(row) / nRows;

                float theta = PI * v - PI_2;                // 维度角度范围在[-PI/2, PI/2]，从南极到北极
                float phi = PI * 2 * u;                     // 经度角度范围在[0, 2PI]，绕赤道圈

                float x = cos(phi) * cos(theta);            // x坐标
                float y = sin(theta);                       // y坐标
                float z = sin(phi) * cos(theta) * (-1);     // z坐标

                // 对于单位球体，以原点为中心，法线等于位置
                // 副法线是法线绕纬度角度（+theta）旋转90度
                theta += PI_2;
                float r = cos(phi) * cos(theta);            // 副法线的x坐标
                float s = sin(theta);                       // 副法线的y坐标
                float t = sin(phi) * cos(theta) * (-1);     // 副法线的z坐标

                Vertex vertex{};
                vertex.position = vec3(x, y, z) * radius;   // 顶点位置
                vertex.normal = vec3(x, y, z);              // 顶点法线
                vertex.uv = vec2(u, v);                     // 顶点UV坐标
                vertex.binormal = vec3(r, s, t);            // 顶点副法线
                vertex.tangent = glm::cross(vertex.binormal, vertex.normal);  // 顶点切线

                vertices.push_back(vertex);
            }
        }

        // 创建三角形索引
        for (unsigned int col = 0; col < nCols; ++col) {
            for (unsigned int row = 0; row < nRows; ++row) {
                auto index = col * (nRows + 1);

                // 逆时针的顺序排列三角形顶点索引
                indices.push_back(index + row + 1);
                indices.push_back(index + row);
                indices.push_back(index + row + 1 + nRows);

                // 逆时针的顺序排列三角形顶点索引
                indices.push_back(index + row + 1 + nRows + 1);
                indices.push_back(index + row + 1);
                indices.push_back(index + row + 1 + nRows);
            }
        }

       CreateBuffers(vertices, indices);  // 创建顶点缓冲区和索引缓冲区
    }
    void Mesh::CreateCube(float size)
    {
        constexpr int verticeNum = 24;  // 我们只需要24个顶点来三角化6个面
        constexpr int stride = 8;       // 每个顶点数据的步长为8（3个位置 + 3个法线 + 2个UV坐标）

        std::vector<Vertex> vertices;   // 顶点数组
        vertices.reserve(verticeNum);

        // 立方体顶点数据
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

        // 解析顶点数据
        for (unsigned int i = 0; i < verticeNum; i++) {
            unsigned int offset = i * stride;

            Vertex vertex{};
            vertex.position = vec3(data[offset + 0], data[offset + 1], data[offset + 2]) * size;    // 设置顶点位置，并缩放到指定大小
            vertex.normal = vec3(data[offset + 3], data[offset + 4], data[offset + 5]);             // 设置顶点法线
            vertex.uv = vec2(data[offset + 6], data[offset + 7]);                                   // 设置顶点UV坐标

            vertices.push_back(vertex);
        }

        // 逆时针的顺序排列三角形索引
        std::vector<GLuint> indices{
            +0, +2, +1,   +0, +3, +2,   +4, +5, +6,
            +4, +6, +7,   +8, +9, 10,   +8, 10, 11,
            12, 15, 14,   12, 14, 13,   16, 17, 18,
            16, 18, 19,   20, 23, 22,   20, 22, 21
        };

        CreateBuffers(vertices, indices);  // 创建顶点缓冲区和索引缓冲区
    }

    void Mesh::CreatePlane(float size){
        LOG_TRACK;
        constexpr int nVertices = 8;   // 网格顶点数
        constexpr int stride = 8;       // 每个顶点数据的步长为8（3个位置 + 3个法线 + 2个UV坐标）

        // 平面顶点数据
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

        std::vector<Vertex> vertices;   // 顶点数组
        vertices.reserve(nVertices);

        // 解析顶点数据
        for (unsigned int i = 0; i < nVertices; i++) {
            unsigned int offset = i * stride;

            Vertex vertex{};
            vertex.position = vec3(data[offset + 0], data[offset + 1], data[offset + 2]) * size;  // 设置顶点位置，并缩放到指定大小
            vertex.normal = vec3(data[offset + 3], data[offset + 4], data[offset + 5]);          // 设置顶点法线
            vertex.uv = vec2(data[offset + 6], data[offset + 7]);                             // 设置顶点UV坐标

            vertices.push_back(vertex);
        }

        // 逆时针的顺序排列三角形索引
        std::vector<GLuint> indices{ 0, 1, 2, 2, 3, 0, 6, 5, 4, 4, 7, 6 };

        CreateBuffers(vertices, indices);  // 创建顶点缓冲区和索引缓冲区
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

        mVerts = vertices.size();                   // 设置顶点数量
        mTris = indices.size() / 3;                 // 计算三角形数量
    }

    void Mesh::SetMaterialID(GLuint mid) const{
        mMaterialId = mid;
    }

    void Mesh::DrawQuad()
    {
        // 无缓冲渲染允许我们绘制一个四边形，不需要使用任何网格数据
        // 参考链接: https://trass3r.github.io/coding/2019/09/11/bufferless-rendering.html
        // 参考链接: https://stackoverflow.com/a/59739538/10677643

        if (gInternalVAO == nullptr) {
            gInternalVAO = std::make_unique<VAO>();  // 如果内部VAO为空，则包装一个新的VAO
        }

        gInternalVAO->Bind();  // 绑定内部VAO
        glDrawArrays(GL_TRIANGLES, 0, 3);  // 绘制三角形，使用3个顶点，调用3次顶点着色器
    }

    void Mesh::DrawGrid()
    {
        if (gInternalVAO == nullptr) {
            gInternalVAO = std::make_unique<VAO>();  // 如果内部VAO为空，则包装一个新的VAO
        }

        gInternalVAO->Bind();  // 绑定内部VAO
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);  // 绘制六边形，使用6个顶点，调用6次实例
    }
 
}
