#include "HalfEdge3.h"

#include <iostream>
#include <unordered_map>
#include <tuple>
#include <cmath>
#include <algorithm>
#include <queue>

static struct TupleEqual {
    template <typename T, typename U>
    bool operator()(const std::tuple<T, U>& tuple1, const std::tuple<T, U>& tuple2) const
    {
        bool equal = ((std::get<0>(tuple1) == std::get<0>(tuple2)) && (std::get<1>(tuple1) == std::get<1>(tuple2))
            || (std::get<0>(tuple1) == std::get<1>(tuple2)) && (std::get<1>(tuple1) == std::get<0>(tuple2)));
        return equal;
    }
};

static struct TupleHash {
    template <typename T, typename U>
    std::size_t operator()(const std::tuple<T, U>& tuple) const {
        std::size_t seed1 = 0;
        std::size_t seed2 = 0;
        seed1 ^= std::hash<T>{}(std::get<0>(tuple)) + 0x9e3779b9 + (seed1 << 6) + (seed1 >> 2);
        seed1 ^= std::hash<U>{}(std::get<1>(tuple)) + 0x9e3779b9 + (seed1 << 6) + (seed1 >> 2);
        seed2 ^= std::hash<T>{}(std::get<1>(tuple)) + 0x9e3779b9 + (seed2 << 6) + (seed2 >> 2);
        seed2 ^= std::hash<U>{}(std::get<0>(tuple)) + 0x9e3779b9 + (seed2 << 6) + (seed2 >> 2);
        return seed1 ^ seed2;
    }
};

namespace HE3 {

    Mesh::Mesh(const std::string& filepath)
    {
        Load(filepath);
    }

    Mesh::~Mesh()
    {

    }

    void Mesh::Load(const std::string& filepath)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath.c_str(), aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
        if (scene)
        {
            if (scene->mNumMeshes != 1)//Ŀǰֻ����һ��mesh�����
            {
                std::cout << "error: meshes number != 1" << std::endl;
                return;
            }
            aiMesh* mesh = scene->mMeshes[0];
            std::cout << "load mesh successful,num of vertices:" << mesh->mNumVertices << std::endl;

            Load(mesh);
        }
        importer.FreeScene();
    }

    void Mesh::Load(aiMesh* mesh)
    {
        //����Ǵ���Ĳ���mesh���ֶ�����ģ�������Ҫ��д���±���:mesh->mNumVertices,mesh->mNumFaces,mesh->mVertices,mesh->mFaces

        //Ԥ�����������Ŀռ�(���մ�С��һ����Ԥ����Ŀռ��С)
        m_Vertices.clear();
        m_Edges.clear();
        m_Faces.clear();
        m_Vertices.reserve(mesh->mNumVertices);
        m_Edges.reserve(3 * mesh->mNumFaces);
        m_Faces.reserve(mesh->mNumFaces);

        std::unordered_map<std::tuple<int, int>, int, TupleHash, TupleEqual> cache1;//������԰��

        //���������б�
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            HE3::Vertex v;
            v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            v.color = glm::vec3(1.0f, 1.0f, 1.0f);
            m_Vertices.push_back(v);
        }

        /*������岽��������б����е�һ���д��������б�*/
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            //step1.�Ȳ��ܶ�ż�ߣ�ֱ�Ӵ��������������ڰ��

            aiFace face = mesh->mFaces[i];
            std::vector<HE3::HalfEdge> edges(face.mNumIndices);
            int base = m_Edges.size();
            for (int j = 0;j < face.mNumIndices;j++)
            {
                edges[j].vertexIndex = face.mIndices[j];
                edges[j].faceIndex = i;
                edges[j].oppositeEdgeIndex = -1;
                if (j == 0)
                    edges[j].nextEdgeIndex = base + face.mNumIndices - 1;
                else
                    edges[j].nextEdgeIndex = base + j - 1;
            }
            for (int j = 0;j < face.mNumIndices;j++)
            {
                m_Vertices[edges[j].vertexIndex].edgeIndex = edges[j].nextEdgeIndex;//��ͬ�����ζ�Ӧͬһ������ᷴ���޸ĵ��edgeindex��������ָ�����
                m_Edges.push_back(edges[j]);
            }
            m_Faces.push_back({ base + 1 });
        }

        //step2.�Ȳ��ܶ�ż��߷���ǿ�����ͬһ���ϵİ��

        for (int i = 0;i < m_Edges.size();i++)
        {
            int v2 = m_Edges[i].vertexIndex;
            int e = i;
            while (m_Edges[e].nextEdgeIndex != i)
            {
                e = m_Edges[e].nextEdgeIndex;
            }
            int v1 = m_Edges[e].vertexIndex;
            //v1->v2
            if (cache1.find({ v1, v2 }) != cache1.end())
            {
                m_Edges[i].oppositeEdgeIndex = cache1[{v1, v2}];
                m_Edges[cache1[{v1, v2}]].oppositeEdgeIndex = i;
                cache1.erase({ v1, v2 });
            }
            else
            {
                cache1[{v1, v2}] = i;
            }
        }

        //step3.�����Ա�ʹ�÷����෴
        AdjustTriangleDiretionNoRecursion(0);

        //step4.�����ϣ��1��ʣ�°�ߵĶԱ�(�����Ե)
        int innerEdgesNum = m_Edges.size();
        for (const auto& pair : cache1)
        {
            HE3::HalfEdge newEdge;
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
                m_Vertices[v0].edgeIndex = m_Edges.size() - 1;
                m_Edges[e].oppositeEdgeIndex = m_Edges.size() - 1;
            }
            else//v == v1
            {
                newEdge.vertexIndex = v0;
                newEdge.faceIndex = -1;
                newEdge.oppositeEdgeIndex = e;
                newEdge.nextEdgeIndex = -1;

                m_Edges.push_back(newEdge);
                m_Vertices[v1].edgeIndex = m_Edges.size() - 1;
                m_Edges[e].oppositeEdgeIndex = m_Edges.size() - 1;
            }
        }

        //step5.���������Ե�İ��
        for (int i = innerEdgesNum; i < m_Edges.size(); i++)
        {
            int v = m_Edges[i].vertexIndex;//�ð��ָ��Ķ����index
            m_Edges[i].nextEdgeIndex = m_Vertices[v].edgeIndex;
        }

        //������ʼ�������б�(�����ڰ�߽ṹ��ֻ������opengl��ͼ���ڰ�߽ṹ�д������mesh��Ҫ���ú����Ը���indices)
        UpdateIndices();
    }

    bool Mesh::IsBoundaryVertex(int vertexIndex)
    {
        if (m_Edges[m_Vertices[vertexIndex].edgeIndex].faceIndex == -1)
            return true;
        else
            return false;
    }

    void Mesh::AdjustTriangleDiretionNoRecursion(int faceIndex)
    {
        //�ǵݹ鷽����ָ��һ����������Ϊ��㣬������ȱ�����������������
        std::queue<int> fringe;//��������ı�Ե
        std::vector<bool> confirmList(m_Faces.size(), false);//�Ѿ�ȷ�Ϸ�����ȷ�������Ѿ�������е���
        fringe.push(faceIndex);
        confirmList[faceIndex] = true;

        //��ʼ����ɣ�������ѭ��ֱ�������涼��������ȷ����
        while (!fringe.empty())
        {
            int faceIndex = fringe.front();
            fringe.pop();

            std::vector<int> edgesIndex;
            edgesIndex.push_back(m_Faces[faceIndex].edgeIndex);
            while (m_Edges[edgesIndex.back()].nextEdgeIndex != edgesIndex.front())
            {
                edgesIndex.push_back(m_Edges[edgesIndex.back()].nextEdgeIndex);
            }
            for (int i = 0;i < edgesIndex.size();i++)
            {
                if (m_Edges[edgesIndex[i]].oppositeEdgeIndex != -1 && confirmList[m_Edges[m_Edges[edgesIndex[i]].oppositeEdgeIndex].faceIndex] == false)
                {
                    fringe.push(m_Edges[m_Edges[edgesIndex[i]].oppositeEdgeIndex].faceIndex);
                    confirmList[m_Edges[m_Edges[edgesIndex[i]].oppositeEdgeIndex].faceIndex] = true;
                    if (m_Edges[m_Edges[edgesIndex[i]].oppositeEdgeIndex].vertexIndex == m_Edges[edgesIndex[i]].vertexIndex)
                    {
                        overturnFace(m_Edges[m_Edges[edgesIndex[i]].oppositeEdgeIndex].faceIndex);
                    }
                }
            }
            
        }
        return;
    }

    void Mesh::overturnFace(int faceIndex)
    {
        //��תһ����ı߳��򣬲����ǶԱ�
        std::vector<int> edgesIndex;
        std::vector<int> verticesIndex;
        edgesIndex.push_back(m_Faces[faceIndex].edgeIndex);
        verticesIndex.push_back(m_Edges[edgesIndex.back()].vertexIndex);
        while (m_Edges[edgesIndex.back()].nextEdgeIndex != edgesIndex.front())
        {
            edgesIndex.push_back(m_Edges[edgesIndex.back()].nextEdgeIndex);
            verticesIndex.push_back(m_Edges[edgesIndex.back()].vertexIndex);
        }
        for (int i = 0;i < edgesIndex.size();i++)
        {
            m_Vertices[verticesIndex[i]].edgeIndex = edgesIndex[i];
            m_Edges[edgesIndex[i]].vertexIndex = (i - 1 == -1) ? verticesIndex[edgesIndex.size() - 1] : verticesIndex[i - 1];
            m_Edges[edgesIndex[i]].nextEdgeIndex = (i - 1 == -1) ? edgesIndex[edgesIndex.size() - 1] : edgesIndex[i - 1];
        }
    }

    void Mesh::PrintVertices()
    {
        printf("Vertices' position:\n");
        for (int i = 0;i < m_Vertices.size();i++)
        {
            printf("v%d  ->e:%d  position:%f,%f,%f\n", i, m_Vertices[i].edgeIndex, m_Vertices[i].position.x, m_Vertices[i].position.y, m_Vertices[i].position.z);
        }
    }

    void Mesh::PrintIndices1()
    {
        printf("Indices1:\n");
        for (auto it = m_Indices1.begin(); it != m_Indices1.end(); it += 3)
        {
            printf("%d,%d,%d\n", *it, *(it + 1), *(it + 2));
        }
    }

    void Mesh::PrintIndices2()
    {
        printf("Indices2:\n");
        for (auto it = m_Indices2.begin(); it != m_Indices2.end(); it += 2)
        {
            printf("%d,%d\n", *it, *(it + 1));
        }
    }

    void Mesh::PrintHalfEdges()
    {
        printf("HalfEdges:\n");
        for (int i = 0;i < m_Edges.size();i++)
        {
            int vEnd = m_Edges[i].vertexIndex;
            int vStart = m_Edges[m_Edges[i].oppositeEdgeIndex].vertexIndex;
            int face = m_Edges[i].faceIndex;
            printf("e%d  v:%d->%d  f:%d  opp:%d  next:%d\n", i, vStart, vEnd, face, m_Edges[i].oppositeEdgeIndex, m_Edges[i].nextEdgeIndex);
        }
    }

    void Mesh::PrintFaces()
    {
        printf("Faces:\n");
        for (int i = 0;i < m_Faces.size();i++)
        {
            std::vector<int> edgesIndex;
            std::vector<int> verticesIndex;
            edgesIndex.push_back(m_Faces[i].edgeIndex);
            verticesIndex.push_back(m_Edges[edgesIndex.back()].vertexIndex);
            while (m_Edges[edgesIndex.back()].nextEdgeIndex != edgesIndex.front())
            {
                edgesIndex.push_back(m_Edges[edgesIndex.back()].nextEdgeIndex);
                verticesIndex.push_back(m_Edges[edgesIndex.back()].vertexIndex);
            }
            printf("face%d: indice(", i);
            for (int j = 0;j < verticesIndex.size();j++)
            {
                printf("%d,", verticesIndex[j]);
            }
            printf(")\n");
        }
    }

    void Mesh::UpdateIndices()
    {
        m_Indices1.clear();
        m_Indices1.reserve(m_Faces.size() * 4);
        m_Indices2.clear();
        m_Indices2.reserve(m_Edges.size() * 2);
        //����indice1
        for (auto& face : m_Faces)
        {
            std::vector<int> edgesIndex;
            std::vector<int> verticesIndex;
            edgesIndex.push_back(face.edgeIndex);
            verticesIndex.push_back(m_Edges[edgesIndex.back()].vertexIndex);
            while (m_Edges[edgesIndex.back()].nextEdgeIndex != edgesIndex.front())
            {
                edgesIndex.push_back(m_Edges[edgesIndex.back()].nextEdgeIndex);
                verticesIndex.push_back(m_Edges[edgesIndex.back()].vertexIndex);
            }
            for (int i = 0;i < verticesIndex.size() - 2;i++)
            {
                m_Indices1.push_back(verticesIndex[0]);
                m_Indices1.push_back(verticesIndex[i+1]);
                m_Indices1.push_back(verticesIndex[i+2]);
            }
        }
        //����indice2
        for (auto& edge : m_Edges)
        {
            int v1 = m_Edges[edge.oppositeEdgeIndex].vertexIndex;
            int v2 = edge.vertexIndex;
            m_Indices2.push_back(v1);
            m_Indices2.push_back(v2);
        }
    }

    int Mesh::ShapeOfFace(int faceIndex)
    {
        int e0 = m_Faces[faceIndex].edgeIndex;
        int e = e0;
        int num = 1;
        while (m_Edges[e].nextEdgeIndex != e0)
        {
            e = m_Edges[e].nextEdgeIndex;
            num++;
        }
        return num;
    }

}//namespace HE3