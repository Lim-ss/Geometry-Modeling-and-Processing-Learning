#pragma once

#include <vector>
#include<string>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Eigen/Core"
#include "Eigen/Geometry"

namespace HE2 {
    struct Vertex;
    struct HalfEdge;
    struct Face;

    class Mesh
    {
    public:
        Mesh(const std::string& filepath);
        ~Mesh();
        void Reload(const std::string& filepath);
        void AdjustTriangleDiretion(int faceIndex);
        void PrintVertices();
        void PrintIndices();
        void PrintHalfEdges();
        void PrintFaces();
        void PrintMeanCurvatureVector();
        void PrintQEM();
        void PrintQuadricError();
        void UpdateIndices();
        void UpdateNormals();

        bool IsBoundaryVertex(int vertexIndex);
        glm::vec3 Laplace_Beltrami_Operator(int vertexIndex);
        glm::vec3 Laplace_Operator(int vertexIndex);
        int PrecursorEdge(int edgeIndex);
        glm::vec3 NormalOfFace(int faceIndex);
        int Degree(int vertexIndex);

        void DeleteFace(int faceIndex);
        void DeleteVertex(int vertexIndex);
        void EraseFace(int faceIndex);
        void EraseVertex(int vertexIndex);
        void EraseEdge(int edgeIndex);
        int EdgeContract(int edgeIndex);
        bool EdgeContractable(int edgeIndex);

    public:
        std::vector<Vertex> m_Vertices;
        std::vector<HalfEdge> m_Edges;
        std::vector<Face> m_Faces;
        std::vector<unsigned int> m_Indices;//不属于半边结构的一部分，只用于opengl绘图
    };

    struct Vertex {
        int edgeIndex;//如果是边界点，则这里的边默认是边界边，若不止一条边界边，则随机取一条
        glm::vec3 position;//顶点位置
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;
        Eigen::Matrix4f QuadricErrorMetrix;
    };

    struct HalfEdge {
        int vertexIndex;//正常的边一定有指向的顶点，如果该边已经失效，但还未来得及删除，该处设为-1
        int faceIndex;
        int oppositeEdgeIndex;  // 对向边
        int nextEdgeIndex;      // 下一条边
        float QuadricError;
        glm::vec3 bestPosition;
    };

    struct Face {
        int edgeIndex;//边索引，正常的面一定有边索引，如果该面已经失效，但还未来得及删除，该处设为-1
    };

    glm::vec3 Excenter(glm::vec3 A, glm::vec3 B, glm::vec3 C);
    float AreaOfTriangle(glm::vec3 A, glm::vec3 B, glm::vec3 C);

}//namespace HE2