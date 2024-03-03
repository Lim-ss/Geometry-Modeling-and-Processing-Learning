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
        void PrintMeanCurvatureVector();

        bool IsBoundaryVertex(int vertexIndex);
        glm::vec3 Laplace_Beltrami_Operator(int vertexIndex);
        glm::vec3 Laplace_Operator(int vertexIndex);
    public:
        std::vector<Vertex> m_Vertices;
        std::vector<HalfEdge> m_Edges;
        std::vector<Face> m_Faces;
        std::vector<unsigned int> m_Indices;//�����ڰ�߽ṹ��һ���֣�ֻ����opengl��ͼ
    };

    struct Vertex {
        int edgeIndex;
        glm::vec3 position;//����λ��
        glm::vec3 color;
        //�������ԣ��編�ߡ����������
    };

    struct HalfEdge {
        int vertexIndex;
        int faceIndex;
        int oppositeEdgeIndex;  // �����
        int nextEdgeIndex;      // ��һ����
    };

    struct Face {
        int edgeIndex;    // ������
        // ��������
    };

    glm::vec3 Excenter(glm::vec3 A, glm::vec3 B, glm::vec3 C);
    float AreaOfTriangle(glm::vec3 A, glm::vec3 B, glm::vec3 C);

}//namespace HE