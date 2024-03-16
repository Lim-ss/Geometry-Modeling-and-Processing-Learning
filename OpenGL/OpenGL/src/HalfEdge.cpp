#include "HalfEdge.h"

#include <iostream>
#include <unordered_map>
#include <tuple>
#include <cmath>
#include <algorithm>
#include <queue>

#include "HashForTuple.h"

static void Sort(int& a, int& b, int& c)
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

            Reload(mesh);
        }
        importer.FreeScene();
    }

    void Mesh::Reload(aiMesh* mesh)
    {
        //����Ǵ���Ĳ���mesh���ֶ�����ģ�������Ҫ��д���±���:mesh->mNumVertices,mesh->mNumFaces,mesh->mVertices,mesh->mFaces

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

        //step3.�����Ա�ʹ�÷����෴
        AdjustTriangleDiretionNoRecursion(0);

        /*
        //�ݹ鷽��������ջ��������ٲ���
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            //e0 < e1 < e2, ������ߵ�index
            int e0 = i * 3 + 0;
            int e1 = i * 3 + 1;
            int e2 = i * 3 + 2;
            
            if (m_Edges[e0].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e0].oppositeEdgeIndex].vertexIndex == m_Edges[e0].vertexIndex)
                AdjustTriangleDiretion(m_Edges[m_Edges[e0].oppositeEdgeIndex].faceIndex);
            if (m_Edges[e1].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e1].oppositeEdgeIndex].vertexIndex == m_Edges[e1].vertexIndex)
                AdjustTriangleDiretion(m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex);
            if (m_Edges[e2].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e2].oppositeEdgeIndex].vertexIndex == m_Edges[e2].vertexIndex)
                AdjustTriangleDiretion(m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex);
        }
        */

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

            int e1 = m_Faces[faceIndex].edgeIndex;
            int e2 = m_Edges[e1].nextEdgeIndex;
            int e3 = m_Edges[e2].nextEdgeIndex;

            if (m_Edges[e1].oppositeEdgeIndex != -1 && confirmList[m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex] == false)
            {
                fringe.push(m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex);
                confirmList[m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex] = true;
                if (m_Edges[m_Edges[e1].oppositeEdgeIndex].vertexIndex == m_Edges[e1].vertexIndex)
                {
                    overturnTriangle(m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex);
                }
            }
            if (m_Edges[e2].oppositeEdgeIndex != -1 && confirmList[m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex] == false)
            {
                fringe.push(m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex);
                confirmList[m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex] = true;
                if (m_Edges[m_Edges[e2].oppositeEdgeIndex].vertexIndex == m_Edges[e2].vertexIndex)
                {
                    overturnTriangle(m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex);
                }
            }
            if (m_Edges[e3].oppositeEdgeIndex != -1 && confirmList[m_Edges[m_Edges[e3].oppositeEdgeIndex].faceIndex] == false)
            {
                fringe.push(m_Edges[m_Edges[e3].oppositeEdgeIndex].faceIndex);
                confirmList[m_Edges[m_Edges[e3].oppositeEdgeIndex].faceIndex] = true;
                if (m_Edges[m_Edges[e3].oppositeEdgeIndex].vertexIndex == m_Edges[e3].vertexIndex)
                {
                    overturnTriangle(m_Edges[m_Edges[e3].oppositeEdgeIndex].faceIndex);
                }
            }
        }
        return;
    }

    void Mesh::overturnTriangle(int faceIndex)
    {
        //��תһ����ı߳��򣬲����ǶԱ�
        int e1 = m_Faces[faceIndex].edgeIndex;
        int e2 = m_Edges[e1].nextEdgeIndex;
        int e3 = m_Edges[e2].nextEdgeIndex;

        int v1 = m_Edges[e1].vertexIndex;
        int v2 = m_Edges[e2].vertexIndex;
        int v3 = m_Edges[e3].vertexIndex;

        m_Edges[e1].vertexIndex = v3;
        m_Edges[e2].vertexIndex = v1;
        m_Edges[e3].vertexIndex = v2;

        m_Edges[e1].nextEdgeIndex = e3;
        m_Edges[e2].nextEdgeIndex = e1;
        m_Edges[e3].nextEdgeIndex = e2;
    }

    void Mesh::PrintVertices()
    {
        printf("Vertices' position:\n");
        for (int i = 0;i < m_Vertices.size();i++)
        {
            printf("v%d  ->e:%d  position:%f,%f,%f\n",i, m_Vertices[i].edgeIndex, m_Vertices[i].position.x, m_Vertices[i].position.y, m_Vertices[i].position.z);
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
        for (int i = 0;i < m_Edges.size();i++)
        {
            int vEnd = m_Edges[i].vertexIndex;
            int vStart = m_Edges[m_Edges[i].oppositeEdgeIndex].vertexIndex;
            int face = m_Edges[i].faceIndex;
            printf("e%d  v:%d->%d  f:%d  opp:%d\n", i, vStart, vEnd, face, m_Edges[i].oppositeEdgeIndex);
        }
    }

    void Mesh::PrintFaces()
    {
        printf("Faces:\n");
        for (int i = 0;i < m_Faces.size();i++)
        {
            int v1 = m_Edges[m_Faces[i].edgeIndex].vertexIndex;
            int v2 = m_Edges[m_Edges[m_Faces[i].edgeIndex].nextEdgeIndex].vertexIndex;
            int v3 = m_Edges[m_Edges[m_Edges[m_Faces[i].edgeIndex].nextEdgeIndex].nextEdgeIndex].vertexIndex;
            glm::vec3 normal = NormalOfFace(i);
            printf("face%d: indice(%d,%d,%d)  normal(%f,%f,%f)  \n", i, v1, v2, v3, normal.x, normal.y, normal.z);
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
    
    static glm::vec3 Excenter(glm::vec3 A, glm::vec3 B, glm::vec3 C)
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

    static float AreaOfTriangle(glm::vec3 A, glm::vec3 B, glm::vec3 C)
    {
        float a = sqrtf(pow(B.x - C.x, 2) + pow(B.y - C.y, 2) + pow(B.z - C.z, 2));
        float b = sqrtf(pow(C.x - A.x, 2) + pow(C.y - A.y, 2) + pow(C.z - A.z, 2));
        float c = sqrtf(pow(A.x - B.x, 2) + pow(A.y - B.y, 2) + pow(A.z - B.z, 2));

        float p = (a + b + c) / 2.0f;
        return sqrtf(p * (p - a) * (p - b) * (p - c));
    }

    void Mesh::UpdateIndices()
    {
        m_Indices.clear();
        m_Indices.reserve(m_Faces.size() * 3);
        for (auto& face : m_Faces)
        {
            m_Indices.push_back(m_Edges[face.edgeIndex].vertexIndex);
            m_Indices.push_back(m_Edges[m_Edges[face.edgeIndex].nextEdgeIndex].vertexIndex);
            m_Indices.push_back(m_Edges[m_Edges[m_Edges[face.edgeIndex].nextEdgeIndex].nextEdgeIndex].vertexIndex);
        }
    }

    void Mesh::UpdateNormals()
    {
        for (int i = 0;i < m_Vertices.size();i++)
        {
            int start = m_Vertices[i].edgeIndex;
            if (start >= (int)m_Edges.size())
            {
                //printf("UpdateNormals() : invalid edgeIndex,%d >= %d\n", start, m_Edges.size());
                continue;
            }
            if (m_Edges[start].faceIndex >= (int)m_Faces.size())
            {
                //printf("UpdateNormals() : invalid faceIndex,%d >= %d\n", m_Edges[start].faceIndex, m_Faces.size());
                continue;
            }
            glm::vec3 averageNormal = NormalOfFace(m_Edges[start].faceIndex);
            int t = m_Edges[m_Edges[start].oppositeEdgeIndex].nextEdgeIndex;
            while (t != start)
            {
                averageNormal += NormalOfFace(m_Edges[t].faceIndex);
                t = m_Edges[m_Edges[t].oppositeEdgeIndex].nextEdgeIndex;
            }
            m_Vertices[i].normal = glm::normalize(averageNormal);
        }
    }
    
    void Mesh::DeleteFace(int faceIndex)
    {
        int e1 = m_Faces[faceIndex].edgeIndex;
        int e2 = m_Edges[m_Faces[faceIndex].edgeIndex].nextEdgeIndex;
        int e3 = m_Edges[m_Edges[m_Faces[faceIndex].edgeIndex].nextEdgeIndex].nextEdgeIndex;

        //bi��ʾei��Ӧ�ı��Ƿ�Ϊ�߽�
        bool b1 = (m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex == -1);
        bool b2 = (m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex == -1);
        bool b3 = (m_Edges[m_Edges[e3].oppositeEdgeIndex].faceIndex == -1);
        if (!b1 && !b2 && !b3)
        {
            //�ڲ�������
            m_Edges[e1].faceIndex = -1;
            m_Edges[e2].faceIndex = -1;
            m_Edges[e3].faceIndex = -1;
        }
        else if (b1 && !b2 && !b3)
        {
            //e1�ڱ�Ե
            int e4 = m_Edges[e1].oppositeEdgeIndex;
            int e5 = PrecursorEdge(e4);
            m_Edges[e5].nextEdgeIndex = e2;
            m_Edges[e3].nextEdgeIndex = m_Edges[e4].nextEdgeIndex;
            m_Edges[e2].faceIndex = -1;
            m_Edges[e3].faceIndex = -1;
            m_Vertices[m_Edges[e5].vertexIndex].edgeIndex = e2;
            m_Vertices[m_Edges[e2].vertexIndex].edgeIndex = e3;

            //���ΪʧЧ
            m_Edges[e1].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            //ɾ��e1��e4����ɾ���ں���ķ�ֹ����ʧЧ
            if (e1 > e4)
            {
                EraseEdge(e1);
                EraseEdge(e4);
            }
            else
            {
                EraseEdge(e4);
                EraseEdge(e1);
            }
        }
        else if (!b1 && b2 && !b3)
        {
            //e2�ڱ�Ե
            int e4 = m_Edges[e2].oppositeEdgeIndex;
            int e5 = PrecursorEdge(e4);
            m_Edges[e5].nextEdgeIndex = e3;
            m_Edges[e1].nextEdgeIndex = m_Edges[e4].nextEdgeIndex;
            m_Edges[e3].faceIndex = -1;
            m_Edges[e1].faceIndex = -1;
            m_Vertices[m_Edges[e5].vertexIndex].edgeIndex = e3;
            m_Vertices[m_Edges[e3].vertexIndex].edgeIndex = e1;

            //���ΪʧЧ
            m_Edges[e2].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            //ɾ��e2��e4����ɾ���ں���ķ�ֹ����ʧЧ
            if (e2 > e4)
            {
                EraseEdge(e2);
                EraseEdge(e4);
            }
            else
            {
                EraseEdge(e4);
                EraseEdge(e2);
            }
        }
        else if (!b1 && !b2 && b3)
        {
            //e3�ڱ�Ե
            int e4 = m_Edges[e3].oppositeEdgeIndex;
            int e5 = PrecursorEdge(e4);
            m_Edges[e5].nextEdgeIndex = e1;
            m_Edges[e2].nextEdgeIndex = m_Edges[e4].nextEdgeIndex;
            m_Edges[e1].faceIndex = -1;
            m_Edges[e2].faceIndex = -1;
            m_Vertices[m_Edges[e5].vertexIndex].edgeIndex = e1;
            m_Vertices[m_Edges[e1].vertexIndex].edgeIndex = e2;

            //���ΪʧЧ
            m_Edges[e3].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            //ɾ��e3��e4����ɾ���ں���ķ�ֹ����ʧЧ
            if (e3 > e4)
            {
                EraseEdge(e3);
                EraseEdge(e4);
            }
            else
            {
                EraseEdge(e4);
                EraseEdge(e3);
            }
        }
        else if (b1 && b2 && !b3)
        {
            //e1��e2�ڱ�Ե
            int e4 = m_Edges[e1].oppositeEdgeIndex;
            int e5 = m_Edges[e2].oppositeEdgeIndex;
            int e6 = PrecursorEdge(e5);
            m_Edges[e6].nextEdgeIndex = e3;
            m_Edges[e3].nextEdgeIndex = m_Edges[e4].nextEdgeIndex;
            m_Edges[e3].faceIndex = -1;
            m_Vertices[m_Edges[e6].vertexIndex].edgeIndex = e3;

            if (m_Edges[e5].nextEdgeIndex != e4)
            {
                m_Edges[PrecursorEdge(e4)].nextEdgeIndex = m_Edges[e5].nextEdgeIndex;
                m_Vertices[m_Edges[e5].vertexIndex].edgeIndex = m_Edges[e5].nextEdgeIndex;
            }
            else
            {
                EraseVertex(m_Edges[e5].vertexIndex);
            }

            //���ΪʧЧ
            m_Edges[e1].vertexIndex = -1;
            m_Edges[e2].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            m_Edges[e5].vertexIndex = -1;

            std::vector<int> edgeToDelete;
            edgeToDelete.push_back(e1);
            edgeToDelete.push_back(e2);
            edgeToDelete.push_back(e4);
            edgeToDelete.push_back(e5);
            std::sort(edgeToDelete.begin(), edgeToDelete.end());//����
            for (int i = 0;i < 4;i++)
            {
                EraseEdge(edgeToDelete.back());
                edgeToDelete.pop_back();
            }
        }
        else if (!b1 && b2 && b3)
        {
            //e2��e3�ڱ�Ե
            int e4 = m_Edges[e2].oppositeEdgeIndex;
            int e5 = m_Edges[e3].oppositeEdgeIndex;
            int e6 = PrecursorEdge(e5);
            m_Edges[e6].nextEdgeIndex = e1;
            m_Edges[e1].nextEdgeIndex = m_Edges[e4].nextEdgeIndex;
            m_Edges[e1].faceIndex = -1;
            m_Vertices[m_Edges[e6].vertexIndex].edgeIndex = e1;

            if (m_Edges[e5].nextEdgeIndex != e4)
            {
                m_Edges[PrecursorEdge(e4)].nextEdgeIndex = m_Edges[e5].nextEdgeIndex;
                m_Vertices[m_Edges[e5].vertexIndex].edgeIndex = m_Edges[e5].nextEdgeIndex;
            }
            else
            {
                EraseVertex(m_Edges[e5].vertexIndex);
            }

            //���ΪʧЧ
            m_Edges[e2].vertexIndex = -1;
            m_Edges[e3].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            m_Edges[e5].vertexIndex = -1;

            std::vector<int> edgeToDelete;
            edgeToDelete.push_back(e2);
            edgeToDelete.push_back(e3);
            edgeToDelete.push_back(e4);
            edgeToDelete.push_back(e5);
            std::sort(edgeToDelete.begin(), edgeToDelete.end());//����
            for (int i = 0;i < 4;i++)
            {
                EraseEdge(edgeToDelete.back());
                edgeToDelete.pop_back();
            }
        }
        else if (b1 && !b2 && b3)
        {
            //e3��e1�ڱ�Ե
            int e4 = m_Edges[e3].oppositeEdgeIndex;
            int e5 = m_Edges[e1].oppositeEdgeIndex;
            int e6 = PrecursorEdge(e5);
            m_Edges[e6].nextEdgeIndex = e2;
            m_Edges[e2].nextEdgeIndex = m_Edges[e4].nextEdgeIndex;
            m_Edges[e2].faceIndex = -1;
            m_Vertices[m_Edges[e6].vertexIndex].edgeIndex = e2;

            if (m_Edges[e5].nextEdgeIndex != e4)
            {
                m_Edges[PrecursorEdge(e4)].nextEdgeIndex = m_Edges[e5].nextEdgeIndex;
                m_Vertices[m_Edges[e5].vertexIndex].edgeIndex = m_Edges[e5].nextEdgeIndex;
            }
            else
            {
                EraseVertex(m_Edges[e5].vertexIndex);
            }

            //���ΪʧЧ
            m_Edges[e3].vertexIndex = -1;
            m_Edges[e1].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            m_Edges[e5].vertexIndex = -1;

            std::vector<int> edgeToDelete;
            edgeToDelete.push_back(e3);
            edgeToDelete.push_back(e1);
            edgeToDelete.push_back(e4);
            edgeToDelete.push_back(e5);
            std::sort(edgeToDelete.begin(), edgeToDelete.end());//����
            for (int i = 0;i < 4;i++)
            {
                EraseEdge(edgeToDelete.back());
                edgeToDelete.pop_back();
            }
        }
        else//(b1 && b2 && b3)
        {
            //���߾�Ϊ��Ե��������������Ȼ��������������
            int e4 = m_Edges[e1].oppositeEdgeIndex;
            int e5 = m_Edges[e2].oppositeEdgeIndex;
            int e6 = m_Edges[e3].oppositeEdgeIndex;
            if (m_Edges[e5].nextEdgeIndex != e4)
            {
                m_Edges[PrecursorEdge(e4)].nextEdgeIndex = m_Edges[e5].nextEdgeIndex;
                m_Vertices[m_Edges[e5].vertexIndex].edgeIndex = m_Edges[e5].nextEdgeIndex;
            }
            else
            {
                EraseVertex(m_Edges[e5].vertexIndex);
            }
            if (m_Edges[e6].nextEdgeIndex != e5)
            {
                m_Edges[PrecursorEdge(e5)].nextEdgeIndex = m_Edges[e6].nextEdgeIndex;
                m_Vertices[m_Edges[e6].vertexIndex].edgeIndex = m_Edges[e6].nextEdgeIndex;
            }
            else
            {
                EraseVertex(m_Edges[e6].vertexIndex);
            }
            if (m_Edges[e4].nextEdgeIndex != e6)
            {
                m_Edges[PrecursorEdge(e6)].nextEdgeIndex = m_Edges[e4].nextEdgeIndex;
                m_Vertices[m_Edges[e4].vertexIndex].edgeIndex = m_Edges[e4].nextEdgeIndex;
            }
            else
            {
                EraseVertex(m_Edges[e4].vertexIndex);
            }

            m_Edges[e1].vertexIndex = -1;
            m_Edges[e2].vertexIndex = -1;
            m_Edges[e3].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            m_Edges[e5].vertexIndex = -1;
            m_Edges[e6].vertexIndex = -1;

            std::vector<int> edgeToDelete;
            edgeToDelete.push_back(e1);
            edgeToDelete.push_back(e2);
            edgeToDelete.push_back(e3);
            edgeToDelete.push_back(e4);
            edgeToDelete.push_back(e5);
            edgeToDelete.push_back(e6);
            std::sort(edgeToDelete.begin(), edgeToDelete.end());//����
            for (int i = 0;i < 6;i++)
            {
                EraseEdge(edgeToDelete.back());
                edgeToDelete.pop_back();
            }
        }

        EraseFace(faceIndex);
    }

    void Mesh::DeleteVertex(int vertexIndex)
    {

    }

    void Mesh::EraseVertex(int vertexIndex)
    {
        if (vertexIndex == m_Vertices.size() - 1)
        {
            m_Vertices.pop_back();
            return;
        }
        //������Ҫ��ĩβԪ�ؽ��н��������޸���ԭ��ĩβԪ����ص�������ֻ�����ڵı߻�����Ӱ�죩
        int oldIndex = m_Vertices.size() - 1;
        int newIndex = vertexIndex;
        int t = m_Edges[m_Vertices[oldIndex].edgeIndex].oppositeEdgeIndex;
        while (1)
        {
            m_Edges[t].vertexIndex = newIndex;
            t = m_Edges[t].nextEdgeIndex;
            if (t == m_Vertices[oldIndex].edgeIndex)
                break;
            t = m_Edges[t].oppositeEdgeIndex;
        }
        m_Vertices[newIndex] = m_Vertices[oldIndex];
        m_Vertices.pop_back();
        return;

    }

    void Mesh::EraseFace(int faceIndex)
    {
        if (faceIndex == m_Faces.size() - 1)
        {
            m_Faces.pop_back();
            return;
        }
        //������Ҫ��ĩβԪ�ؽ��н��������޸���ԭ��ĩβԪ����ص�������ֻ�����ڵı߻�����Ӱ�죩
        int oldIndex = m_Faces.size() - 1;
        int newIndex = faceIndex;

        if (m_Faces[oldIndex].edgeIndex == -1)
        {
            //ʧЧ�棬ʲô������
        }
        else
        {
            int t = m_Faces[oldIndex].edgeIndex;
            m_Edges[t].faceIndex = newIndex;
            t = m_Edges[t].nextEdgeIndex;
            m_Edges[t].faceIndex = newIndex;
            t = m_Edges[t].nextEdgeIndex;
            m_Edges[t].faceIndex = newIndex;

        }
        m_Faces[newIndex] = m_Faces[oldIndex];
        m_Faces.pop_back();
        return;
    }

    void Mesh::EraseEdge(int edgeIndex)
    {
        if (edgeIndex == m_Edges.size() - 1)
        {
            m_Edges.pop_back();
            return;
        }
        //������Ҫ��ĩβԪ�ؽ��н��������޸���ԭ��ĩβԪ����ص�������ֻ�����ڵı߻�����Ӱ�죩
        int oldIndex = m_Edges.size() - 1;
        int newIndex = edgeIndex;

        if (m_Edges[oldIndex].vertexIndex == -1)
        {
            //ʧЧ�ߣ�ɶ�����ø�
        }
        else if (m_Edges[oldIndex].faceIndex != -1)
        {
            //�ڲ���
            m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].oppositeEdgeIndex = newIndex;
            m_Edges[m_Edges[m_Edges[oldIndex].nextEdgeIndex].nextEdgeIndex].nextEdgeIndex = newIndex;
            if (m_Faces[m_Edges[oldIndex].faceIndex].edgeIndex == oldIndex)
                m_Faces[m_Edges[oldIndex].faceIndex].edgeIndex = newIndex;
            if (m_Vertices[m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].vertexIndex].edgeIndex == oldIndex)
                m_Vertices[m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].vertexIndex].edgeIndex = newIndex;
        }
        else
        {
            //�ⲿ��
            m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].oppositeEdgeIndex = newIndex;
            m_Edges[PrecursorEdge(oldIndex)].nextEdgeIndex = newIndex;
            m_Vertices[m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].vertexIndex].edgeIndex = newIndex;
        }
        m_Edges[newIndex] = m_Edges[oldIndex];
        m_Edges.pop_back();
        return;
    }

    int Mesh::PrecursorEdge(int edgeIndex)
    {
        //���ظð�ߵ�ֱ��ǰ��
        if (m_Edges[edgeIndex].faceIndex != -1)
        {
            //�ڲ���ֱ������������תһȦЧ�ʸ���
            return m_Edges[m_Edges[edgeIndex].nextEdgeIndex].nextEdgeIndex;
        }
        //����Ϊ�ⲿ�ߣ�ֻ�����Ŷ���תһȦ
        int t = edgeIndex;
        while (m_Edges[m_Edges[t].oppositeEdgeIndex].nextEdgeIndex != edgeIndex)
        {
            t = m_Edges[m_Edges[t].oppositeEdgeIndex].nextEdgeIndex;
        }
        return m_Edges[t].oppositeEdgeIndex;
    }

    glm::vec3 Mesh::NormalOfFace(int faceIndex)
    {
        if (faceIndex == -1)
            return glm::vec3(0, 0, 0);
        int e1 = m_Faces[faceIndex].edgeIndex;
        int e2 = m_Edges[e1].nextEdgeIndex;
        int e3 = m_Edges[e2].nextEdgeIndex;

        if (m_Edges[e1].vertexIndex >= m_Vertices.size() || m_Edges[e2].vertexIndex >= m_Vertices.size() || m_Edges[e3].vertexIndex >= m_Vertices.size())
        {
            //printf("NormalOfFace() : invalid vertexIndex,(%d,%d,%d),%d\n", m_Edges[e1].vertexIndex, m_Edges[e2].vertexIndex, m_Edges[e3].vertexIndex, m_Vertices.size());
            return glm::vec3(0, 0, 0);
        }

        glm::vec3 v1 = m_Vertices[m_Edges[e1].vertexIndex].position;
        glm::vec3 v2 = m_Vertices[m_Edges[e2].vertexIndex].position;
        glm::vec3 v3 = m_Vertices[m_Edges[e3].vertexIndex].position;

        glm::vec3 vector1 = v1 - v2;
        glm::vec3 vector2 = v3 - v1;
        return glm::normalize(glm::cross(vector1, vector2));
    }

    int Mesh::EdgeContract(int edgeIndex)
    {
        /*
        �ϲ�����ܻ�����˻��������Σ�Ŀǰ��ûд��������⴦��һ��˼·�Ǻϲ���ɺ��鵽�˻����������ٺϲ�һ�Σ�������Ѿ��˻�Ϊ�ߵ��������ٴ��˻�Ϊ��
        */
        int f1 = m_Edges[edgeIndex].faceIndex;
        int f2 = m_Edges[m_Edges[edgeIndex].oppositeEdgeIndex].faceIndex;
        if (f1 != -1 && f2 != -1)
        {
            //�ڲ���
            int e1 = edgeIndex;
            int e2 = m_Edges[e1].nextEdgeIndex;
            int e3 = m_Edges[e2].nextEdgeIndex;
            int e4 = m_Edges[e1].oppositeEdgeIndex;
            int e5 = m_Edges[e4].nextEdgeIndex;
            int e6 = m_Edges[e5].nextEdgeIndex;
            int e7 = m_Edges[e3].oppositeEdgeIndex;
            int e8 = m_Edges[e2].oppositeEdgeIndex;
            int e9 = m_Edges[e6].oppositeEdgeIndex;
            int e10 = m_Edges[e5].oppositeEdgeIndex;
            
            int v1 = m_Edges[e1].vertexIndex;
            int v2 = m_Edges[e4].vertexIndex;
            int v3 = m_Edges[e2].vertexIndex;
            int v4 = m_Edges[e5].vertexIndex;

            if (Degree(v3) == 3 || Degree(v4) == 3)
            {
                return -1;//�޷�������
            }

            glm::vec3 mid = 0.5f * m_Vertices[v1].position + 0.5f * m_Vertices[v2].position;
            m_Vertices[v1].position = mid;
            m_Vertices[v2].position = mid;

            if (IsBoundaryVertex(v1))//��ֹ�߽���Ĭ�ϱ߲��Ǳ߽��
            {
                //�ϲ���v1�������ж�v2��ָ���Ϊv1
                int t = e10;
                while (t != e3)
                {
                    m_Edges[t].vertexIndex = v1;
                    t = m_Edges[m_Edges[t].nextEdgeIndex].oppositeEdgeIndex;
                }

                //������������ںϲ�����֮ǰ
                m_Edges[e7].oppositeEdgeIndex = e8;
                m_Edges[e8].oppositeEdgeIndex = e7;
                m_Edges[e9].oppositeEdgeIndex = e10;
                m_Edges[e10].oppositeEdgeIndex = e9;

                if (m_Vertices[v3].edgeIndex == e3)
                {
                    m_Vertices[v3].edgeIndex = e8;
                }
                if (m_Vertices[v4].edgeIndex == e6)
                {
                    m_Vertices[v4].edgeIndex = e10;
                }

                //���������Ҫ�ڳ���ʧЧ�㡢�ߡ���֮�⣬���˹�ϵ��ȫ�ָ�֮��
                EraseVertex(v2);//����ֻɾ��һ�����㣬���Բ��ÿ���ʧЧ��ɾ��˳��
            }
            else if (IsBoundaryVertex(v2))
            {
                //�ϲ���v2�������ж�v1��ָ���Ϊv2
                int t = e8;
                while (t != e6)
                {
                    m_Edges[t].vertexIndex = v2;
                    t = m_Edges[m_Edges[t].nextEdgeIndex].oppositeEdgeIndex;
                }

                //������������ںϲ�����֮ǰ
                m_Edges[e7].oppositeEdgeIndex = e8;
                m_Edges[e8].oppositeEdgeIndex = e7;
                m_Edges[e9].oppositeEdgeIndex = e10;
                m_Edges[e10].oppositeEdgeIndex = e9;

                if (m_Vertices[v3].edgeIndex == e3)
                {
                    m_Vertices[v3].edgeIndex = e8;
                }
                if (m_Vertices[v4].edgeIndex == e6)
                {
                    m_Vertices[v4].edgeIndex = e10;
                }

                //���������Ҫ�ڳ���ʧЧ�㡢�ߡ���֮�⣬���˹�ϵ��ȫ�ָ�֮��
                EraseVertex(v1);
            }
            else
            {
                //�ϲ���v1����Ҫ��ֹv1ָ��ɾ���ı�
                int t = e10;
                while (t != e3)
                {
                    m_Edges[t].vertexIndex = v1;
                    t = m_Edges[m_Edges[t].nextEdgeIndex].oppositeEdgeIndex;
                }
                m_Vertices[v1].edgeIndex = e9;

                //������������ںϲ�����֮ǰ
                m_Edges[e7].oppositeEdgeIndex = e8;
                m_Edges[e8].oppositeEdgeIndex = e7;
                m_Edges[e9].oppositeEdgeIndex = e10;
                m_Edges[e10].oppositeEdgeIndex = e9;

                if (m_Vertices[v3].edgeIndex == e3)
                {
                    m_Vertices[v3].edgeIndex = e8;
                }
                if (m_Vertices[v4].edgeIndex == e6)
                {
                    m_Vertices[v4].edgeIndex = e10;
                }

                //���������Ҫ�ڳ���ʧЧ�㡢�ߡ���֮�⣬���˹�ϵ��ȫ�ָ�֮��
                EraseVertex(v2);
            }

            //�����Щ��ͱ�ΪʧЧ
            m_Faces[f1].edgeIndex = -1;
            m_Faces[f2].edgeIndex = -1;
            m_Edges[e1].vertexIndex = -1;
            m_Edges[e2].vertexIndex = -1;
            m_Edges[e3].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;
            m_Edges[e5].vertexIndex = -1;
            m_Edges[e6].vertexIndex = -1;

            std::vector<int> edgeToDelete;
            edgeToDelete.push_back(e1);
            edgeToDelete.push_back(e2);
            edgeToDelete.push_back(e3);
            edgeToDelete.push_back(e4);
            edgeToDelete.push_back(e5);
            edgeToDelete.push_back(e6);
            std::sort(edgeToDelete.begin(), edgeToDelete.end());//����
            for (int i = 0;i < 6;i++)
            {
                EraseEdge(edgeToDelete.back());
                edgeToDelete.pop_back();
            }

            std::vector<int> faceToDelete;
            faceToDelete.push_back(f1);
            faceToDelete.push_back(f2);
            std::sort(faceToDelete.begin(), faceToDelete.end());//����
            for (int i = 0;i < 2;i++)
            {
                EraseFace(faceToDelete.back());
                faceToDelete.pop_back();
            }
        }
        else if ((f1 != -1 && f2 == -1) || (f1 == -1 && f2 != -1))
        {
            //�߽��
            int e1;
            if (f1 != -1)
                e1 = edgeIndex;
            else
                e1 = m_Edges[edgeIndex].oppositeEdgeIndex;
            int e2 = m_Edges[e1].nextEdgeIndex;
            int e3 = m_Edges[e2].nextEdgeIndex;
            int e4 = m_Edges[e1].oppositeEdgeIndex;
            int e5 = PrecursorEdge(e4);
            int e6 = m_Edges[e4].nextEdgeIndex;
            int e7 = m_Edges[e3].oppositeEdgeIndex;
            int e8 = m_Edges[e2].oppositeEdgeIndex;

            int v1 = m_Edges[e1].vertexIndex;
            int v2 = m_Edges[e4].vertexIndex;
            int v3 = m_Edges[e2].vertexIndex;

            if (Degree(v3) == 3)
            {
                return -1;//�޷�������
            }
            //�ϲ���v1
            {
                int t = e4;
                while (t != e3)
                {
                    m_Edges[t].vertexIndex = v1;
                    t = m_Edges[m_Edges[t].nextEdgeIndex].oppositeEdgeIndex;
                }
            }

            //������������ںϲ�����֮ǰ
            m_Edges[e7].oppositeEdgeIndex = e8;
            m_Edges[e8].oppositeEdgeIndex = e7;
            m_Edges[e5].nextEdgeIndex = e6;

            if (m_Vertices[v3].edgeIndex == e3)
            {
                m_Vertices[v3].edgeIndex = e8;
            }

            //���������Ҫ�ڳ���ʧЧ�㡢�ߡ���֮�⣬���˹�ϵ��ȫ�ָ�֮��
            EraseVertex(v2);

            //�����Щ��ΪʧЧ
            m_Edges[e1].vertexIndex = -1;
            m_Edges[e2].vertexIndex = -1;
            m_Edges[e3].vertexIndex = -1;
            m_Edges[e4].vertexIndex = -1;

            std::vector<int> edgeToDelete;
            edgeToDelete.push_back(e1);
            edgeToDelete.push_back(e2);
            edgeToDelete.push_back(e3);
            edgeToDelete.push_back(e4);
            std::sort(edgeToDelete.begin(), edgeToDelete.end());//����
            for (int i = 0;i < 4;i++)
            {
                EraseEdge(edgeToDelete.back());
                edgeToDelete.pop_back();
            }

            if (f1 != -1)
                EraseFace(f1);
            else
                EraseFace(f2);
        }
        else//(f1 == -1 && f2 == -1)
        {
            //�������������
            printf("error: EdgeContract apply to invalid edge\n");
            exit(-1);
        }

        return 1;
    }

    int Mesh::Degree(int vertexIndex)
    {
        //�󶥵�Ķ�
        int num = 1;
        int e0 = m_Vertices[vertexIndex].edgeIndex;
        int e = m_Edges[m_Edges[e0].oppositeEdgeIndex].nextEdgeIndex;
        while (e != e0)
        {
            num++;
            e = m_Edges[m_Edges[e].oppositeEdgeIndex].nextEdgeIndex;
        }
        return num;
    }
}//namespace HE