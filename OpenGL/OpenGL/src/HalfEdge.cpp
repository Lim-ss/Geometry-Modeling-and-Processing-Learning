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
    //����ʹ��a < b < c
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

namespace HE {

	Mesh::Mesh(const std::string& filepath)
	{
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
        if (scene)
        {
            if (scene->mNumMeshes != 1)//Ŀǰֻ����һ��mesh�����
            {
                std::cout << "error: meshes number != 1" << std::endl;
                return;
            }
            aiMesh* mesh = scene->mMeshes[0];
            std::cout << "load mesh successful,num of vertices:" << mesh->mNumVertices << std::endl;

            m_Vertices.resize(mesh->mNumVertices);//һ���Է���ռ䣬����Ч��
            m_Edges.resize(3 * mesh->mNumFaces);
            m_Faces.resize(mesh->mNumFaces);
            m_Indices.resize(3 * mesh->mNumFaces);

            std::unordered_map<std::tuple<int, int>, int> cache1;//������԰�ߣ�ע��vector�е����ִ�С����
            std::unordered_map<int, int> cache2;//��Ϊ��Ե�ϵĶ��㣬ֵΪ���������Ϊ���ı�Ե��ߣ��������ӱ�Ե�İ�ߣ���nextedge

            //���������б�
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                m_Vertices[i].position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            }
            //�������б�
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                //if (face.mNumIndices != 3)//��ʵ���Բ��жϣ���Ϊ����ʱ����aiProcess_Triangulate����
                //    continue;

                m_Faces[i].edgeIndex = i * 3;
            }

            /*������岽��������б�*/
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                //e0 < e1 < e2, ������ߵ�index
                int e0 = i * 3 + 0;
                int e1 = i * 3 + 1;
                int e2 = i * 3 + 2;
                //v0 < v1 < v2, ���������index
                int v0 = face.mIndices[0];
                int v1 = face.mIndices[1];
                int v2 = face.mIndices[2];
                Sort(v0, v1, v2);

                //step1.�Ȳ��ܶ�ż�ߣ�ֱ�Ӵ������а��
                m_Edges[e0].vertexIndex = v0;
                m_Edges[e1].vertexIndex = v1;
                m_Edges[e2].vertexIndex = v2;

                m_Edges[e0].faceIndex = i;
                m_Edges[e1].faceIndex = i;
                m_Edges[e2].faceIndex = i;

                m_Edges[e0].oppositeEdgeIndex = -1;
                m_Edges[e1].oppositeEdgeIndex = -1;
                m_Edges[e2].oppositeEdgeIndex = -1;

                m_Edges[e0].nextEdgeIndex = e1;
                m_Edges[e1].nextEdgeIndex = e2;
                m_Edges[e2].nextEdgeIndex = e0;
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

                //step2.�Ȳ��ܶ�ż��߷���ǿ�����ͬһ���ϵİ��
                if (cache1.find({ v0, v1 }) != cache1.end())
                {
                    m_Edges[e1].oppositeEdgeIndex = cache1[{ v0, v1 }];
                    m_Edges[cache1[{ v0, v1 }]].oppositeEdgeIndex = e1;
                    cache1.erase({ v0, v1 });
                }
                else
                {
                    cache1[{ v0, v1 }] = e1;
                }
                if (cache1.find({ v1, v2 }) != cache1.end())
                {
                    m_Edges[e2].oppositeEdgeIndex = cache1[{ v1, v2 }];
                    m_Edges[cache1[{ v1, v2 }]].oppositeEdgeIndex = e2;
                    cache1.erase({ v1, v2 });
                }
                else
                {
                    cache1[{ v1, v2 }] = e2;
                }
                if (cache1.find({ v0, v2 }) != cache1.end())
                {
                    m_Edges[e0].oppositeEdgeIndex = cache1[{ v0, v2 }];
                    m_Edges[cache1[{ v0, v2 }]].oppositeEdgeIndex = e0;
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
                //e0 < e1 < e2, ������ߵ�index
                int e0 = i * 3 + 0;
                int e1 = i * 3 + 1;
                int e2 = i * 3 + 2;
                //v0 < v1 < v2, ���������index
                int v0 = face.mIndices[0];
                int v1 = face.mIndices[1];
                int v2 = face.mIndices[2];
                Sort(v0, v1, v2);

                //step3.�����Ա�ʹ�÷����෴
                if (m_Edges[e0].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e0].oppositeEdgeIndex].vertexIndex == m_Edges[e0].vertexIndex)
                    AdjustTriangleDiretion(m_Edges[m_Edges[e0].oppositeEdgeIndex].faceIndex);
                if (m_Edges[e1].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e1].oppositeEdgeIndex].vertexIndex == m_Edges[e1].vertexIndex)
                    AdjustTriangleDiretion(m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex);
                if (m_Edges[e2].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e2].oppositeEdgeIndex].vertexIndex == m_Edges[e2].vertexIndex)
                    AdjustTriangleDiretion(m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex);

            }
            //step4.�����ϣ��1��ʣ�°�ߵĶԱ�(�����Ե)
            for (const auto& pair : cache1) 
            {
                HE::HalfEdge newEdge;
                int v0 = std::get<0>(pair.first);
                int v1 = std::get<1>(pair.first);
                int e = pair.second;
                int v = m_Edges[e].vertexIndex;

                if (v == v0)
                {
                    newEdge.vertexIndex = v1;
                    newEdge.faceIndex = -1;
                    newEdge.oppositeEdgeIndex = e;
                    newEdge.nextEdgeIndex = -1;

                    m_Edges.push_back(newEdge);
                    cache2[v0] = m_Edges.size() - 1;

                    m_Edges[e].oppositeEdgeIndex = m_Edges.size() - 1;
                }
                else//v == v1
                {
                    newEdge.vertexIndex = v0;
                    newEdge.faceIndex = -1;
                    newEdge.oppositeEdgeIndex = e;
                    newEdge.nextEdgeIndex = -1;

                    m_Edges.push_back(newEdge);
                    cache2[v1] = m_Edges.size() - 1;

                    m_Edges[e].oppositeEdgeIndex = m_Edges.size() - 1;
                }
                
            }
            
            //step5.���������Ե�İ��
            for (int i = 3 * m_Faces.size(); i < m_Edges.size(); i++)
            {
                int v = m_Edges[i].vertexIndex;//�ð��ָ��Ķ����index
                m_Edges[i].nextEdgeIndex = cache2[v];
            }

            //������ʼ�������б�(�����ڰ�߽ṹ��ֻ������opengl��ͼ���ڰ�߽ṹ�д������mesh��Ҫ���ú����Ը���indices)
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

    void Mesh::AdjustTriangleDiretion(int faceIndex)
    {
        /*ָ��һ��������,�ı������а�߷��򣬲��ݹ�ص�����ʹ���ٽ��������η��϶�ż����*/
        int edge1Index = m_Faces[faceIndex].edgeIndex;
        int edge2Index = m_Edges[edge1Index].nextEdgeIndex;
        int edge3Index = m_Edges[edge2Index].nextEdgeIndex;

        int vertex1Index = m_Edges[edge1Index].vertexIndex;
        int vertex2Index = m_Edges[edge2Index].vertexIndex;
        int vertex3Index = m_Edges[edge3Index].vertexIndex;

        m_Edges[edge1Index].vertexIndex = vertex3Index;
        m_Edges[edge2Index].vertexIndex = vertex1Index;
        m_Edges[edge3Index].vertexIndex = vertex2Index;

        m_Edges[edge1Index].nextEdgeIndex = edge3Index;
        m_Edges[edge2Index].nextEdgeIndex = edge1Index;
        m_Edges[edge3Index].nextEdgeIndex = edge2Index;

        //�ݹ���ú��������ſ����ж���һ�������Ƿ���Ҫ���ã�������ܳ����ظ���תһ��������
        if (m_Edges[edge1Index].oppositeEdgeIndex != -1 && m_Edges[m_Edges[edge1Index].oppositeEdgeIndex].vertexIndex == m_Edges[edge1Index].vertexIndex)
            AdjustTriangleDiretion(m_Edges[m_Edges[edge1Index].oppositeEdgeIndex].faceIndex);
        if (m_Edges[edge2Index].oppositeEdgeIndex != -1 && m_Edges[m_Edges[edge2Index].oppositeEdgeIndex].vertexIndex == m_Edges[edge2Index].vertexIndex)
            AdjustTriangleDiretion(m_Edges[m_Edges[edge2Index].oppositeEdgeIndex].faceIndex);
        if (m_Edges[edge3Index].oppositeEdgeIndex != -1 && m_Edges[m_Edges[edge3Index].oppositeEdgeIndex].vertexIndex == m_Edges[edge3Index].vertexIndex)
            AdjustTriangleDiretion(m_Edges[m_Edges[edge3Index].oppositeEdgeIndex].faceIndex);
        return;
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
            int vEnd = it->vertexIndex;
            int vStart = m_Edges[it->oppositeEdgeIndex].vertexIndex;
            int face = it->faceIndex;
            printf("v:%d->%d  f:%d\n", vStart, vEnd, face);
        }
    }
    
}//namespace HE