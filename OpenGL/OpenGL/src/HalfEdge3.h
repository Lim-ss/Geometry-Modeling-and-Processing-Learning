#pragma once

#include <vector>
#include<string>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace HE3 {
    struct Vertex;
    struct HalfEdge;
    struct Face;

    class Mesh
    {
    public:
        Mesh(const std::string& filepath);
        ~Mesh();
        void Load(const std::string& filepath);
        void Load(aiMesh* mesh);
        void AdjustTriangleDiretionNoRecursion(int faceIndex);
        void overturnFace(int faceIndex);
        void PrintVertices();
        void PrintIndices1();
        void PrintIndices2();
        void PrintHalfEdges();
        void PrintFaces();
        void UpdateIndices();

        bool IsBoundaryVertex(int vertexIndex);
        int ShapeOfFace(int faceIndex);

    public:
        std::vector<Vertex> m_Vertices;
        std::vector<HalfEdge> m_Edges;
        std::vector<Face> m_Faces;
        std::vector<unsigned int> m_Indices1;//���ڻ�������
        std::vector<unsigned int> m_Indices2;//���ڻ��߿�

        //�Զ���ε���ʽ�洢index������opensubdiv�ӿ�
        std::vector<int> m_Vertsperface;
        std::vector<int> m_Indices3;
    };

    struct Vertex {
        int edgeIndex;//����Ǳ߽�㣬������ı�Ĭ���Ǳ߽�ߣ�����ֹһ���߽�ߣ������ȡһ��
        glm::vec3 position;//����λ��
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    struct HalfEdge {
        int vertexIndex;//�����ı�һ����ָ��Ķ��㣬����ñ��Ѿ�ʧЧ������δ���ü�ɾ�����ô���Ϊ-1
        int faceIndex;
        int oppositeEdgeIndex;  // �����
        int nextEdgeIndex;      // ��һ����
    };

    struct Face {
        int edgeIndex;//����������������һ���б���������������Ѿ�ʧЧ������δ���ü�ɾ�����ô���Ϊ-1
    };

}//namespace HE3