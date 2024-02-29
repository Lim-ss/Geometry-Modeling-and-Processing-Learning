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
        void PrintVertices();
        void PrintIndices();
        void PrintHalfEdges();

    public:
        std::vector<Vertex> m_Vertices;
        std::vector<HalfEdge> m_Edges;
        std::vector<Face> m_Faces;
        std::vector<unsigned int> m_Indices;//�����ڰ�߽ṹ��һ���֣�ֻ����opengl��ͼ
    };

    struct Vertex {
        HalfEdge* edge;
        glm::vec3 position;//����λ��
        //�������ԣ��編�ߡ����������
    };

    struct HalfEdge {
        Vertex* vertex;
        Face* face;
        HalfEdge* oppositeEdge;  // �����
        HalfEdge* nextEdge;      // ��һ����
    };

    struct Face {
        HalfEdge* edge;    // ������
        // ��������
    };

}//namespace HE