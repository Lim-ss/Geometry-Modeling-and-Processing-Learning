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
        std::vector<unsigned int> m_Indices1;//用于画三角面
        std::vector<unsigned int> m_Indices2;//用于画线框

        //以多边形的形式存储index，用于opensubdiv接口
        std::vector<int> m_Vertsperface;
        std::vector<int> m_Indices3;
    };

    struct Vertex {
        int edgeIndex;//如果是边界点，则这里的边默认是边界边，若不止一条边界边，则随机取一条
        glm::vec3 position;//顶点位置
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    struct HalfEdge {
        int vertexIndex;//正常的边一定有指向的顶点，如果该边已经失效，但还未来得及删除，该处设为-1
        int faceIndex;
        int oppositeEdgeIndex;  // 对向边
        int nextEdgeIndex;      // 下一条边
    };

    struct Face {
        int edgeIndex;//边索引，正常的面一定有边索引，如果该面已经失效，但还未来得及删除，该处设为-1
    };

}//namespace HE3