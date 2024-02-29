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
        std::vector<unsigned int> m_Indices;//不属于半边结构的一部分，只用于opengl绘图
    };

    struct Vertex {
        HalfEdge* edge;
        glm::vec3 position;//顶点位置
        //其他属性，如法线、纹理坐标等
    };

    struct HalfEdge {
        Vertex* vertex;
        Face* face;
        HalfEdge* oppositeEdge;  // 对向边
        HalfEdge* nextEdge;      // 下一条边
    };

    struct Face {
        HalfEdge* edge;    // 边索引
        // 其他属性
    };

}//namespace HE