#include "HalfEdge.h"

#include <iostream>
#include <unordered_map>
#include <tuple>
#include <cmath>

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
        Reload(filepath);
    }

    Mesh::~Mesh()
    {

    }

    void Mesh::Reload(const std::string& filepath)
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
                m_Vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);
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

            //��������Ӱ����Ϣ,���ڱ�Ե�İ���ڰ���б��ĩβ����˱�Ե���edgeһ���Ǳ�Ե��ߣ��ⷽ������һ��������а�ߵĲ���
            for (int i = 0; i < m_Edges.size(); i++)
            {
                m_Vertices[m_Edges[m_Edges[i].oppositeEdgeIndex].vertexIndex].edgeIndex = i;
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

    bool Mesh::IsBoundaryVertex(int vertexIndex)
    {
        if (m_Edges[m_Vertices[vertexIndex].edgeIndex].faceIndex == -1)
            return true;
        else
            return false;
    }

    glm::vec3 Mesh::Laplace_Beltrami_Operator(int vertexIndex)
    {
        if (IsBoundaryVertex(vertexIndex))
            return glm::vec3(0.0f, 0.0f, 0.0f);

        std::vector<glm::vec3> vector;//��vi->vj������
        glm::vec3 vi = m_Vertices[vertexIndex].position;//Ŀ�궥������
        int firstEdgeIndex = m_Vertices[vertexIndex].edgeIndex;//���ڱȽ��Ƿ�������һȦ
        int EdgeIndex = firstEdgeIndex;//����

        vector.push_back(m_Vertices[m_Edges[EdgeIndex].vertexIndex].position - vi);
        EdgeIndex = m_Edges[m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;//�л�����һ����
        while (EdgeIndex != firstEdgeIndex)
        {
            vector.push_back(m_Vertices[m_Edges[EdgeIndex].vertexIndex].position - vi);
            EdgeIndex = m_Edges[m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;
        }

        std::vector<float> angle1;//�洢ÿ���߶�Ӧ����ǰһ�������ڵĽ�(������)
        std::vector<float> angle2;//�洢ÿ���߶�Ӧ�����һ�������ڵĽ�(������)

        int n = vector.size();
        glm::vec3 t1 = vector[n - 1] - vector[0];
        glm::vec3 t2 = vector[1] - vector[0];
        angle1.push_back(std::acos(glm::dot(vector[0], t1) / (glm::length(vector[0]) * glm::length(t1))));
        angle2.push_back(std::acos(glm::dot(vector[0], t2) / (glm::length(vector[0]) * glm::length(t2))));
        for (int i = 1;i <= n - 2;i++)
        {
            t1 = vector[i - 1] - vector[i];
            t2 = vector[i + 1] - vector[i];
            angle1.push_back(std::acos(glm::dot(vector[i], t1) / (glm::length(vector[i]) * glm::length(t1))));
            angle2.push_back(std::acos(glm::dot(vector[i], t2) / (glm::length(vector[i]) * glm::length(t2))));
        }
        t1 = vector[n - 2] - vector[n - 1];
        t2 = vector[0] - vector[n - 1];
        angle1.push_back(std::acos(glm::dot(vector[n - 1], t1) / (glm::length(vector[n - 1]) * glm::length(t1))));
        angle2.push_back(std::acos(glm::dot(vector[n - 1], t2) / (glm::length(vector[n - 1]) * glm::length(t2))));

        //��Voronoi cell�����
        float A = 0.0f;
        for (int i = 0;i < n;i++)
        {
            A += 0.125f * glm::dot(vector[i], vector[i]) * ((1.0f / tan(angle1[i])) + (1.0f / tan(angle2[i])));
        }

        glm::vec3 Hn = glm::vec3(0.0f, 0.0f, 0.0f);
        for (int i = 0;i < n;i++)
        {
            Hn += -((1.0f / tan(angle1[i])) + (1.0f / tan(angle2[i]))) * vector[i];
        }
        Hn *= (0.5f / A);
        return Hn;
    }

    glm::vec3 Mesh::Laplace_Operator(int vertexIndex)
    {
        if (IsBoundaryVertex(vertexIndex))
            return glm::vec3(0.0f, 0.0f, 0.0f);

        std::vector<glm::vec3> vector;//��vi->vj������
        glm::vec3 vi = m_Vertices[vertexIndex].position;//Ŀ�궥������
        int firstEdgeIndex = m_Vertices[vertexIndex].edgeIndex;//���ڱȽ��Ƿ�������һȦ
        int EdgeIndex = firstEdgeIndex;//һ���򶥵�

        vector.push_back(m_Vertices[m_Edges[EdgeIndex].vertexIndex].position - vi);
        EdgeIndex = m_Edges[m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;//�л�����һ����
        while (EdgeIndex != firstEdgeIndex)
        {
            vector.push_back(m_Vertices[m_Edges[EdgeIndex].vertexIndex].position - vi);
            EdgeIndex = m_Edges[m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;
        }

        glm::vec3 Hn = glm::vec3(0.0f, 0.0f, 0.0f);
        for (int i = 0;i < vector.size();i++)
        {
            Hn -= vector[i];
        }
        return Hn;
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
            printf("edge:%d  position:%f,%f,%f\n", it->edgeIndex, it->position.x, it->position.y, it->position.z);
        }
    }

    void Mesh::PrintIndices()
    {
        printf("Indices:\n");
        for (auto it = m_Indices.begin(); it != m_Indices.end(); it += 3)
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

    void Mesh::PrintMeanCurvatureVector()
    {
        for (int i = 0;i < m_Vertices.size();i++)
        {
            printf("Mean Curvature Vector");
            glm::vec3 a = Laplace_Beltrami_Operator(i);
            printf("v%d : %f,%f,%f\n", i, a.x, a.y, a.z);
        }
    }
    

    glm::vec3 Excenter(glm::vec3 A, glm::vec3 B, glm::vec3 C)
    {
        glm::vec3 MAB = 0.5f * A + 0.5f * B;
        glm::vec3 MBC = 0.5f * B + 0.5f * C;
        glm::vec3 vectorAB = B - A;
        glm::vec3 vectorBC = C - B;
        glm::vec3 faceNormal = glm::cross(vectorAB, vectorBC);
        glm::vec3 NAB = glm::normalize(glm::cross(faceNormal, vectorAB));
        glm::vec3 NBC = glm::normalize(glm::cross(faceNormal, vectorBC));

        float t = NAB.x * NBC.y - NBC.x * NAB.y;
        float lambda1 = (NBC.y * (MBC.x - MAB.x) - NBC.x * (MBC.y - MAB.y)) / t;
        //float lambda2 = (NAB.y * (MBC.x - MAB.x) - NAB.x * (MBC.y - MAB.y)) / t;

        glm::vec3 excenter = MAB + lambda1 * NAB;
        return excenter;
    }

    float AreaOfTriangle(glm::vec3 A, glm::vec3 B, glm::vec3 C)
    {
        float a = sqrtf(pow(B.x - C.x, 2) + pow(B.y - C.y, 2) + pow(B.z - C.z, 2));
        float b = sqrtf(pow(C.x - A.x, 2) + pow(C.y - A.y, 2) + pow(C.z - A.z, 2));
        float c = sqrtf(pow(A.x - B.x, 2) + pow(A.y - B.y, 2) + pow(A.z - B.z, 2));

        float p = (a + b + c) / 2.0f;
        return sqrtf(p * (p - a) * (p - b) * (p - c));
    }

    
    
}//namespace HE