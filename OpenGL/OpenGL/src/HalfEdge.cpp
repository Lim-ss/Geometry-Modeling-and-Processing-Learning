#include "HalfEdge.h"

#include <iostream>
#include <unordered_map>
#include <tuple>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "HashForTuple.h"

void Sort(int& a, int& b, int& c)
{
    //排序使得a < b < c
    if (a > b)
    {
        int t = a; a = b; b = t;
    }
    if (a > c)
    {
        int t = a; a = c; c = b; b = t;
    }
    else if (b > c)
    {
        int t = b;
        b = c;
        c = t;
    }
    return;
}

void AdjustTriangleDiretion(HE::Face* face)
{
    /*指定一个三角面,改变其所有半边方向，并递归地调用以使得临近的三角形符合对偶规则*/
    HE::HalfEdge* edge1 = face->edge;
    HE::HalfEdge* edge2 = edge1->nextEdge;
    HE::HalfEdge* edge3 = edge2->nextEdge;

    HE::Vertex* vertex1 = edge1->vertex;
    HE::Vertex* vertex2 = edge2->vertex;
    HE::Vertex* vertex3 = edge3->vertex;

    edge1->vertex = vertex3;
    edge2->vertex = vertex1;
    edge3->vertex = vertex2;

    edge1->nextEdge = edge3;
    edge2->nextEdge = edge1;
    edge3->nextEdge = edge2;

    //递归调用函数结束才可以判断下一个邻面是否需要调用，否则可能出现重复翻转一个面的情况
    if (edge1->oppositeEdge != nullptr && edge1->oppositeEdge->vertex == edge1->vertex)
        AdjustTriangleDiretion(edge1->oppositeEdge->face);
    if (edge2->oppositeEdge != nullptr && edge2->oppositeEdge->vertex == edge2->vertex)
        AdjustTriangleDiretion(edge2->oppositeEdge->face);
    if (edge3->oppositeEdge != nullptr && edge3->oppositeEdge->vertex == edge3->vertex)
        AdjustTriangleDiretion(edge3->oppositeEdge->face);

    return;
}

namespace HE {

	Mesh::Mesh(const std::string& filepath)
	{
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
        if (scene)
        {
            if (scene->mNumMeshes != 1)//目前只考虑一个mesh的情况
            {
                std::cout << "error: meshes number != 1" << std::endl;
                return;
            }
            aiMesh* mesh = scene->mMeshes[0];
            std::cout << "load mesh successful,num of vertices:" << mesh->mNumVertices << std::endl;

            m_Vertices.resize(mesh->mNumVertices);//一次性分配空间，增加效率
            m_Edges.resize(3 * mesh->mNumFaces);
            m_Faces.resize(mesh->mNumFaces);
            m_Indices.resize(3 * mesh->mNumFaces);

            std::unordered_map<std::tuple<int, int>, int> cache1;//用于配对半边，注意vector中的数字从小到大
            std::unordered_map<int, int> cache2;//键为边缘上的顶点，值为以这个顶点为起点的边缘半边，用于连接边缘的半边，即nextedge

            //创建顶点列表
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                m_Vertices[i].position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            }
            //创建面列表
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                //if (face.mNumIndices != 3)//其实可以不判断，因为加载时用了aiProcess_Triangulate参数
                //    continue;

                m_Faces[i].edge = &m_Edges[i * 3];
            }

            /*下面分五步创建半边列表*/
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                //e0 < e1 < e2, 三个半边的index
                int e0 = i * 3 + 0;
                int e1 = i * 3 + 1;
                int e2 = i * 3 + 2;
                //v0 < v1 < v2, 三个顶点的index
                int v0 = face.mIndices[0];
                int v1 = face.mIndices[1];
                int v2 = face.mIndices[2];
                Sort(v0, v1, v2);

                //step1.先不管对偶边，直接创建所有半边
                m_Edges[e0].vertex = &m_Vertices[v0];
                m_Edges[e1].vertex = &m_Vertices[v1];
                m_Edges[e2].vertex = &m_Vertices[v2];

                m_Edges[e0].face = &m_Faces[i];
                m_Edges[e1].face = &m_Faces[i];
                m_Edges[e2].face = &m_Faces[i];

                m_Edges[e0].oppositeEdge = nullptr;
                m_Edges[e1].oppositeEdge = nullptr;
                m_Edges[e2].oppositeEdge = nullptr;

                m_Edges[e0].nextEdge = &m_Edges[e1];
                m_Edges[e1].nextEdge = &m_Edges[e2];
                m_Edges[e2].nextEdge = &m_Edges[e0];
            }
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                int e0 = i * 3 + 0;
                int e1 = i * 3 + 1;
                int e2 = i * 3 + 2;
                int v0 = face.mIndices[0];
                int v1 = face.mIndices[1];
                int v2 = face.mIndices[2];
                Sort(v0, v1, v2);

                //step2.先不管对偶半边方向，强行配对同一边上的半边
                if (cache1.find({ v0, v1 }) != cache1.end())
                {
                    m_Edges[e0].oppositeEdge = &m_Edges[cache1[{ v0, v1 }]];
                    m_Edges[cache1[{ v0, v1 }]].oppositeEdge = &m_Edges[e0];
                    cache1.erase({ v0, v1 });
                }
                else
                {
                    cache1[{ v0, v1 }] = e1;
                }
                if (cache1.find({ v1, v2 }) != cache1.end())
                {
                    m_Edges[e1].oppositeEdge = &m_Edges[cache1[{ v1, v2 }]];
                    m_Edges[cache1[{ v1, v2 }]].oppositeEdge = &m_Edges[e1];
                    cache1.erase({ v1, v2 });
                }
                else
                {
                    cache1[{ v1, v2 }] = e2;
                }
                if (cache1.find({ v0, v2 }) != cache1.end())
                {
                    m_Edges[e2].oppositeEdge = &m_Edges[cache1[{ v0, v2 }]];
                    m_Edges[cache1[{ v0, v2 }]].oppositeEdge = &m_Edges[e2];
                    cache1.erase({ v0, v2 });
                }
                else
                {
                    cache1[{ v0, v2 }] = e0;
                }
            }
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                //e0 < e1 < e2, 三个半边的index
                int e0 = i * 3 + 0;
                int e1 = i * 3 + 1;
                int e2 = i * 3 + 2;
                //v0 < v1 < v2, 三个顶点的index
                int v0 = face.mIndices[0];
                int v1 = face.mIndices[1];
                int v2 = face.mIndices[2];
                Sort(v0, v1, v2);

                //step3.调整对边使得方向相反
                if (m_Edges[e0].oppositeEdge != nullptr && m_Edges[e0].oppositeEdge->vertex == m_Edges[e0].vertex)
                    AdjustTriangleDiretion(m_Edges[e0].oppositeEdge->face);
                if (m_Edges[e1].oppositeEdge != nullptr && m_Edges[e1].oppositeEdge->vertex == m_Edges[e1].vertex)
                    AdjustTriangleDiretion(m_Edges[e1].oppositeEdge->face);
                if (m_Edges[e2].oppositeEdge != nullptr && m_Edges[e2].oppositeEdge->vertex == m_Edges[e2].vertex)
                    AdjustTriangleDiretion(m_Edges[e2].oppositeEdge->face);

            }
            //step4.补齐哈希表1中剩下半边的对边(网格边缘)
            for (const auto& pair : cache1) 
            {
                HE::HalfEdge newEdge;
                int v0 = std::get<0>(pair.first);
                int v1 = std::get<1>(pair.first);
                int e = pair.second;
                int v = std::distance(m_Vertices.data(), m_Edges[e].vertex);//e指向的顶点的index

                if (v == v0)
                {
                    newEdge.vertex = &m_Vertices[v1];
                    newEdge.face = nullptr;
                    newEdge.oppositeEdge = &m_Edges[e];
                    newEdge.nextEdge = nullptr;

                    m_Edges.push_back(newEdge);
                    cache2[v0] = m_Edges.size() - 1;

                    m_Edges[e].oppositeEdge = &m_Edges.back();
                }
                else//v == v1
                {
                    newEdge.vertex = &m_Vertices[v0];
                    newEdge.face = nullptr;
                    newEdge.oppositeEdge = &m_Edges[e];
                    newEdge.nextEdge = nullptr;

                    m_Edges.push_back(newEdge);
                    cache2[v1] = m_Edges.size() - 1;

                    m_Edges[e].oppositeEdge = &m_Edges.back();
                }
                
            }
            
            //step5.连接网格边缘的半边
            for (int i = 3 * m_Faces.size(); i < m_Edges.size(); i++)
            {
                int v = std::distance(m_Vertices.data(), m_Edges[i].vertex);//该半边指向的顶点的index
                m_Edges[i].nextEdge = &m_Edges[cache2[v]];
            }

            //创建初始的索引列表(不属于半边结构，只是用于opengl绘图，在半边结构中处理过的mesh需要调用函数以更新indices)
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                m_Indices[i * 3 + 0] = face.mIndices[0];
                m_Indices[i * 3 + 1] = face.mIndices[1];
                m_Indices[i * 3 + 2] = face.mIndices[2];
            }
        }
        importer.FreeScene();
    }

    Mesh::~Mesh()
    {

    }

    void Mesh::PrintVertices()
    {
        printf("Vertices' position:\n");
        for (auto it = m_Vertices.begin(); it != m_Vertices.end(); ++it) 
        {
            printf("%f,%f,%f\n", it->position.x, it->position.y, it->position.z);
        }
    }

    void Mesh::PrintIndices()
    {
        printf("Indices:\n");
        for (auto it = m_Indices.begin(); it != m_Indices.end(); it+=3)
        {
            printf("%d,%d,%d\n", *it, *(it + 1), *(it + 2));
        }
    }

    void Mesh::PrintHalfEdges()
    {
        printf("HalfEdges:\n");
        for (auto it = m_Edges.begin(); it != m_Edges.end(); it++)
        {
            int vEnd = std::distance(m_Vertices.data(), it->vertex);
            int vStart = std::distance(m_Vertices.data(), it->oppositeEdge->vertex);
            int face = std::distance(m_Faces.data(), it->face);
            printf("v:%d->%d  f:%d\n", vStart, vEnd, face);
        }
    }
    
}//namespace HE