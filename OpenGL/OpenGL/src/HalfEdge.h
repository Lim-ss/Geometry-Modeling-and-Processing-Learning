#pragma once

#include <vector>
#include<string>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace HE {
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
        void UpdateIndices();
        void UpdateNormals();

        bool IsBoundaryVertex(int vertexIndex);
        glm::vec3 Laplace_Beltrami_Operator(int vertexIndex);
        glm::vec3 Laplace_Operator(int vertexIndex);
        int PrecursorEdge(int edgeIndex);
        glm::vec3 NormalOfFace(int faceIndex);

        void DeleteFace(int faceIndex);
        void DeleteVertex(int vertexIndex);
        void EraseFace(int faceIndex);
        void EraseVertex(int vertexIndex);
        void EraseEdge(int edgeIndex);

    public:
        std::vector<Vertex> m_Vertices;
        std::vector<HalfEdge> m_Edges;
        std::vector<Face> m_Faces;
        std::vector<unsigned int> m_Indices;//不属于半边结构的一部分，只用于opengl绘图
    };

    struct Vertex {
        int edgeIndex;
        glm::vec3 position;//顶点位置
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    struct HalfEdge {
        int vertexIndex;
        int faceIndex;
        int oppositeEdgeIndex;  // 对向边
        int nextEdgeIndex;      // 下一条边
    };

    struct Face {
        int edgeIndex;    // 边索引
        // 其他属性
    };

    glm::vec3 Excenter(glm::vec3 A, glm::vec3 B, glm::vec3 C);
    float AreaOfTriangle(glm::vec3 A, glm::vec3 B, glm::vec3 C);

}//namespace HE