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
        std::vector<unsigned int> m_Indices;//�����ڰ�߽ṹ��һ���֣�ֻ����opengl��ͼ
    };

    struct Vertex {
        int edgeIndex;//����Ǳ߽�㣬������ı�Ĭ���Ǳ߽�ߣ�����ֹһ���߽�ߣ������ȡһ��
        glm::vec3 position;//����λ��
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;
        Eigen::Matrix4f QuadricErrorMetrix;
    };

    struct HalfEdge {
        int vertexIndex;//�����ı�һ����ָ��Ķ��㣬����ñ��Ѿ�ʧЧ������δ���ü�ɾ�����ô���Ϊ-1
        int faceIndex;
        int oppositeEdgeIndex;  // �����
        int nextEdgeIndex;      // ��һ����
        float QuadricError;
        glm::vec3 bestPosition;
    };

    struct Face {
        int edgeIndex;//����������������һ���б���������������Ѿ�ʧЧ������δ���ü�ɾ�����ô���Ϊ-1
    };

    glm::vec3 Excenter(glm::vec3 A, glm::vec3 B, glm::vec3 C);
    float AreaOfTriangle(glm::vec3 A, glm::vec3 B, glm::vec3 C);

}//namespace HE2