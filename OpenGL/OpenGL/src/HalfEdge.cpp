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
                m_Vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);
            }
            //创建面列表
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                //if (face.mNumIndices != 3)//其实可以不判断，因为加载时用了aiProcess_Triangulate参数
                //    continue;

                m_Faces[i].edgeIndex = i * 3;
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

                //step2.先不管对偶半边方向，强行配对同一边上的半边
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
                if (m_Edges[e0].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e0].oppositeEdgeIndex].vertexIndex == m_Edges[e0].vertexIndex)
                    AdjustTriangleDiretion(m_Edges[m_Edges[e0].oppositeEdgeIndex].faceIndex);
                if (m_Edges[e1].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e1].oppositeEdgeIndex].vertexIndex == m_Edges[e1].vertexIndex)
                    AdjustTriangleDiretion(m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex);
                if (m_Edges[e2].oppositeEdgeIndex != -1 && m_Edges[m_Edges[e2].oppositeEdgeIndex].vertexIndex == m_Edges[e2].vertexIndex)
                    AdjustTriangleDiretion(m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex);

            }
            //step4.补齐哈希表1中剩下半边的对边(网格边缘)
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

            //step5.连接网格边缘的半边
            for (int i = 3 * m_Faces.size(); i < m_Edges.size(); i++)
            {
                int v = m_Edges[i].vertexIndex;//该半边指向的顶点的index
                m_Edges[i].nextEdgeIndex = cache2[v];
            }

            //给顶点添加半边信息,由于边缘的半边在半边列表的末尾，因此边缘点的edge一定是边缘半边，这方便了由一点查找所有半边的操作
            for (int i = 0; i < m_Edges.size(); i++)
            {
                m_Vertices[m_Edges[m_Edges[i].oppositeEdgeIndex].vertexIndex].edgeIndex = i;
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

        std::vector<glm::vec3> vector;//存vi->vj的向量
        glm::vec3 vi = m_Vertices[vertexIndex].position;//目标顶点坐标
        int firstEdgeIndex = m_Vertices[vertexIndex].edgeIndex;//用于比较是否走完了一圈
        int EdgeIndex = firstEdgeIndex;//出边

        vector.push_back(m_Vertices[m_Edges[EdgeIndex].vertexIndex].position - vi);
        EdgeIndex = m_Edges[m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;//切换到下一条边
        while (EdgeIndex != firstEdgeIndex)
        {
            vector.push_back(m_Vertices[m_Edges[EdgeIndex].vertexIndex].position - vi);
            EdgeIndex = m_Edges[m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;
        }

        std::vector<float> angle1;//存储每条边对应的与前一条边相邻的角(弧度制)
        std::vector<float> angle2;//存储每条边对应的与后一条边相邻的角(弧度制)

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

        //求Voronoi cell的面积
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

        std::vector<glm::vec3> vector;//存vi->vj的向量
        glm::vec3 vi = m_Vertices[vertexIndex].position;//目标顶点坐标
        int firstEdgeIndex = m_Vertices[vertexIndex].edgeIndex;//用于比较是否走完了一圈
        int EdgeIndex = firstEdgeIndex;//一邻域顶点

        vector.push_back(m_Vertices[m_Edges[EdgeIndex].vertexIndex].position - vi);
        EdgeIndex = m_Edges[m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;//切换到下一条边
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
        /*指定一个三角面,改变其所有半边方向，并递归地调用以使得临近的三角形符合对偶规则*/
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

        //递归调用函数结束才可以判断下一个邻面是否需要调用，否则可能出现重复翻转一个面的情况
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

        //bi表示ei对应的边是否为边界
        bool b1 = (m_Edges[m_Edges[e1].oppositeEdgeIndex].faceIndex == -1);
        bool b2 = (m_Edges[m_Edges[e2].oppositeEdgeIndex].faceIndex == -1);
        bool b3 = (m_Edges[m_Edges[e3].oppositeEdgeIndex].faceIndex == -1);
        if (!b1 && !b2 && !b3)
        {
            //内部三角形
            m_Edges[e1].faceIndex = -1;
            m_Edges[e2].faceIndex = -1;
            m_Edges[e3].faceIndex = -1;
        }
        else if (b1 && !b2 && !b3)
        {
            //有一条边在边缘
            int t = PrecursorEdge(e1);
            m_Edges[t].nextEdgeIndex = e2;
            m_Edges[e3].nextEdgeIndex = m_Edges[e1].nextEdgeIndex;
            m_Edges[e2].faceIndex = -1;
            m_Edges[e3].faceIndex = -1;
        }

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
        //否则需要与末尾元素进行交换，并修改与原本末尾元素相关的索引（只有相邻的边或面受影响）
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
        //否则需要与末尾元素进行交换，并修改与原本末尾元素相关的索引（只有相邻的边或面受影响）
        int oldIndex = m_Faces.size() - 1;
        int newIndex = faceIndex;

        int t = m_Faces[oldIndex].edgeIndex;
        m_Edges[t].faceIndex = newIndex;
        t = m_Edges[t].nextEdgeIndex;
        m_Edges[t].faceIndex = newIndex;
        t = m_Edges[t].nextEdgeIndex;
        m_Edges[t].faceIndex = newIndex;

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
        //否则需要与末尾元素进行交换，并修改与原本末尾元素相关的索引（只有相邻的边或面受影响）
        int oldIndex = m_Edges.size() - 1;
        int newIndex = edgeIndex;

        if (m_Edges[oldIndex].faceIndex != -1)
        {
            //内部边
            m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].oppositeEdgeIndex = newIndex;
            m_Edges[m_Edges[m_Edges[oldIndex].nextEdgeIndex].nextEdgeIndex].nextEdgeIndex = newIndex;
            if (m_Faces[m_Edges[oldIndex].faceIndex].edgeIndex == oldIndex)
                m_Faces[m_Edges[oldIndex].faceIndex].edgeIndex = newIndex;
            if (m_Vertices[m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].vertexIndex].edgeIndex == oldIndex)
                m_Vertices[m_Edges[m_Edges[oldIndex].oppositeEdgeIndex].vertexIndex].edgeIndex = newIndex;
        }
        else
        {
            //外部边
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
        //返回该半边的直接前驱
        if (m_Edges[edgeIndex].faceIndex != -1)
        {
            //内部边直接沿着三角形转一圈效率更高
            return m_Edges[m_Edges[edgeIndex].nextEdgeIndex].nextEdgeIndex;
        }
        //否则为外部边，只能沿着顶点转一圈
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
        glm::vec3 v1 = m_Vertices[m_Edges[e1].vertexIndex].position;
        glm::vec3 v2 = m_Vertices[m_Edges[e2].vertexIndex].position;
        glm::vec3 v3 = m_Vertices[m_Edges[e3].vertexIndex].position;

        glm::vec3 vector1 = v1 - v2;
        glm::vec3 vector2 = v3 - v1;
        return glm::normalize(glm::cross(vector1, vector2));
    }
}//namespace HE