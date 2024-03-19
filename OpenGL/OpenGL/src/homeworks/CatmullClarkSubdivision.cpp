#include "CatmullClarkSubdivision.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Eigen/Core"
#include "Eigen/Geometry"
#include "Eigen/Dense"

#include "assimp/scene.h"

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

namespace module {

    CatmullClarkSubdivision::CatmullClarkSubdivision()
        :
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_IO(ImGui::GetIO()),
        m_scale(0.0f)
    {
        m_Mesh = std::make_unique<HE3::Mesh>("res/mesh/Nefertiti_face.obj");

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        m_VAO1 = std::make_unique<VertexArray>();
        m_VAO2 = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
        VertexBufferLayout layout;
        layout.Vacate(sizeof(HE3::Vertex::edgeIndex));//edge
        layout.Push<float>(3);//position
        layout.Push<float>(3);//color
        layout.Push<float>(3);//normal
        layout.Push<float>(2);//texCoord
        m_VAO1->AddBuffer(*m_VBO, layout);
        m_VAO2->AddBuffer(*m_VBO, layout);
        m_IBO1 = std::make_unique<IndexBuffer>(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
        m_IBO2 = std::make_unique<IndexBuffer>(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());
        m_Shader = std::make_unique<Shader>("res/shaders/MeshSubdiv.shader");
        m_Shader->Bind();
        m_Camera = std::make_unique<Camera>(m_View);

        glEnable(GL_POLYGON_OFFSET_FILL); // 启用填充面的多边形偏移
        glPolygonOffset(0.5f, 0.1f); // 设置多边形偏移因子和单位


    }

    CatmullClarkSubdivision::~CatmullClarkSubdivision()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    void CatmullClarkSubdivision::OnUpdate(double deltaTime)
    {
        m_Camera->CameraUpdate(deltaTime);
        m_Model = glm::scale(glm::mat4(1.0f), glm::vec3(pow(10.0f, m_scale)));//调整模型大小
    }

    void CatmullClarkSubdivision::OnRender()
    {
        int width, height;
        GLFWwindow* window = glfwGetCurrentContext();
        if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            //窗口没有最小化
            glfwGetWindowSize(window, &width, &height);
            m_Proj = glm::perspective(glm::radians(m_Camera->fov), (float)width / (float)height, 0.1f, 1000.0f);

        }
        m_MVP = m_Proj * m_View * m_Model;
        m_Shader->SetUniformMat4f("u_MVP", m_MVP);

        /* Render here */
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        Renderer renderer;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Mesh->UpdateIndices();
        m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
        m_IBO1->ReData(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
        m_IBO2->ReData(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_Shader->SetUniform1i("u_Mode", 3);//纯色
        renderer.DrawTriangle(*m_VAO1, *m_IBO1, *m_Shader, m_Mesh->m_Indices1.size());
        m_Shader->SetUniform1i("u_Mode", 2);//黑色
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        renderer.DrawLine(*m_VAO2, *m_IBO2, *m_Shader, m_Mesh->m_Indices2.size());
    }

    void CatmullClarkSubdivision::OnImguiRender()
    {
        if (ImGui::Button("ClarkSubdiv"))
        {
            ClarkSubdiv();
        }
        ImGui::InputText("index", m_input1, IM_ARRAYSIZE(m_input1));
        if (ImGui::Button("highlight vertex"))
        {
            int index = atoi(m_input1);
            if (index < m_Mesh->m_Vertices.size())
            {
                printf("highlight vertex:%d\n", index);
                m_Mesh->m_Vertices[index].color = glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }
        if (ImGui::Button("cancel highlight"))
        {
            printf("cancel highlight\n");
            for (int i = 0;i < m_Mesh->m_Vertices.size();i++)
            {
                m_Mesh->m_Vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);
            }
        }
        if (ImGui::Button("load model 1"))
        {
            m_Mesh->Load("res/mesh/Nefertiti_face.obj");
        }
        if (ImGui::Button("load model 2"))
        {
            m_Mesh->Load("res/mesh/Balls.obj");
        }
        if (ImGui::Button("load model 3"))
        {
            m_Mesh->Load("res/mesh/polygonTest.obj");
        }
        if (ImGui::Button("print"))
        {
            printf("face:%d,edges:%d,vertex:%d\n", m_Mesh->m_Faces.size(), m_Mesh->m_Edges.size(), m_Mesh->m_Vertices.size());
        }
        if (ImGui::Button("printIndices1"))
        {
            m_Mesh->PrintIndices1();
        }
        if (ImGui::Button("printEdges"))
        {
            m_Mesh->PrintHalfEdges();
        }
        if (ImGui::Button("printFaces"))
        {
            m_Mesh->PrintFaces();
        }
        if (ImGui::Button("printVertices"))
        {
            m_Mesh->PrintVertices();
        }
        ImGui::SliderFloat("model scale", &m_scale, -2.0f, 2.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void CatmullClarkSubdivision::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Camera::CursorPosCallback(window, xpos, ypos);
    }

    void CatmullClarkSubdivision::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Camera::KeyCallback(window, key, scancode, action, mods);
    }

    void CatmullClarkSubdivision::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Camera::MouseButtonCallback(window, button, action, mods);
    }

    void CatmullClarkSubdivision::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Camera::ScrollCallback(window, xoffset, yoffset);
    }

    void CatmullClarkSubdivision::ClarkSubdiv()
    {

        std::unordered_map<std::tuple<int, int>, int, TupleHash, TupleEqual> cache;//新边点
        aiVector3D* Vertices = new aiVector3D[m_Mesh->m_Vertices.size() + m_Mesh->m_Faces.size() + m_Mesh->m_Edges.size() / 2];
        if (Vertices == nullptr)
        {
            printf("Vertices allocate failed\n");
        }
        int newfacesNum = 0;
        for (int i = 0;i < m_Mesh->m_Faces.size();i++)
        {
            newfacesNum += m_Mesh->ShapeOfFace(i);
        }
        aiFace* Faces = new aiFace[newfacesNum];
        if (Faces == nullptr)
        {
            printf("Faces allocate failed\n");
        }
        aiMesh mesh;
        mesh.mNumVertices = m_Mesh->m_Vertices.size() + m_Mesh->m_Faces.size() + m_Mesh->m_Edges.size() / 2;
        mesh.mNumFaces = newfacesNum;
        mesh.mVertices = Vertices;
        mesh.mFaces = Faces;

        for (int i = 0;i < m_Mesh->m_Faces.size();i++)
        {
            //创建新面点，排在旧顶点之后
            int e0 = m_Mesh->m_Faces[i].edgeIndex;
            int e = e0;
            int num = 1;
            glm::vec3 sum = m_Mesh->m_Vertices[m_Mesh->m_Edges[e0].vertexIndex].position;
            while (m_Mesh->m_Edges[e].nextEdgeIndex != e0)
            {
                e = m_Mesh->m_Edges[e].nextEdgeIndex;
                num++;
                sum += m_Mesh->m_Vertices[m_Mesh->m_Edges[e].vertexIndex].position;
            }
            glm::vec3 average = sum / (float)num;
            Vertices[m_Mesh->m_Vertices.size() + i] = aiVector3D((ai_real)average.x, (ai_real)average.y, (ai_real)average.z);
            //face[i]的新面点存在了Vertices[m_Mesh->m_Vertices.size() + i]里
        }
        int nextVertexIndex = m_Mesh->m_Vertices.size() + m_Mesh->m_Faces.size();//下一个新点要存入数组的位置
        for (int i = 0;i < m_Mesh->m_Edges.size();i++)
        {
            //创建新边点，在新面点之后
            //v1->v2
            int v1 = m_Mesh->m_Edges[m_Mesh->m_Edges[i].oppositeEdgeIndex].vertexIndex;
            int v2 = m_Mesh->m_Edges[i].vertexIndex;
            if (cache.find({ v1, v2 }) != cache.end())
            {
                //已经创建过了
                continue;
            }
            cache[{v1, v2}] = nextVertexIndex;
            glm::vec3 position;
            int f1 = m_Mesh->m_Edges[i].faceIndex;
            int f2 = m_Mesh->m_Edges[m_Mesh->m_Edges[i].oppositeEdgeIndex].faceIndex;
            if (f1 == -1 || f2 == -1)
            {
                //边界,取两点中点
                position = 0.5f * m_Mesh->m_Vertices[v1].position + 0.5f * m_Mesh->m_Vertices[v2].position;
            }
            else
            {
                //非边界，取两端点和两面点平均
                int v3 = m_Mesh->m_Vertices.size() + f1;
                int v4 = m_Mesh->m_Vertices.size() + f2;
                glm::vec3 v3pos = glm::vec3(Vertices[v3].x, Vertices[v3].y, Vertices[v3].z);
                glm::vec3 v4pos = glm::vec3(Vertices[v4].x, Vertices[v4].y, Vertices[v4].z);
                position = 0.25f * m_Mesh->m_Vertices[v1].position + 0.25f * m_Mesh->m_Vertices[v2].position + 0.25f * v3pos + 0.25f * v4pos;
            }
            Vertices[nextVertexIndex] = aiVector3D((ai_real)position.x, (ai_real)position.y, (ai_real)position.z);
            nextVertexIndex++;
        }
        for (int i = 0;i < m_Mesh->m_Vertices.size();i++)
        {
            //更新旧顶点位置
            if (m_Mesh->IsBoundaryVertex(i))
            {
                //边界点，不更新位置
                Vertices[i] = aiVector3D((ai_real)m_Mesh->m_Vertices[i].position.x, (ai_real)m_Mesh->m_Vertices[i].position.y, (ai_real)m_Mesh->m_Vertices[i].position.z);
                continue;
            }
            std::vector<int> outedges;
            outedges.push_back(m_Mesh->m_Vertices[i].edgeIndex);
            while (m_Mesh->m_Edges[m_Mesh->m_Edges[outedges.back()].oppositeEdgeIndex].nextEdgeIndex != outedges.front())
            {
                outedges.push_back(m_Mesh->m_Edges[m_Mesh->m_Edges[outedges.back()].oppositeEdgeIndex].nextEdgeIndex);
            }
            glm::vec3 Q = glm::vec3(0, 0, 0);//面点平均
            glm::vec3 R = glm::vec3(0, 0, 0);//临边中点的平均
            glm::vec3 S = m_Mesh->m_Vertices[i].position;//旧点坐标
            for (int j = 0;j < outedges.size();j++)
            {
                int fv = m_Mesh->m_Vertices.size() + m_Mesh->m_Edges[outedges[j]].faceIndex;//面点在Vertices里的索引
                Q += glm::vec3(Vertices[fv].x, Vertices[fv].y, Vertices[fv].z);
                glm::vec3 v1pos = m_Mesh->m_Vertices[i].position;
                glm::vec3 v2pos = m_Mesh->m_Vertices[m_Mesh->m_Edges[outedges[j]].vertexIndex].position;
                R += 0.5f * (v1pos + v2pos);
            }
            int n = outedges.size();
            Q /= n;
            R /= n;
            glm::vec3 newPos = (Q + 2.0f * R + S * (float)(n - 3)) / (float)n;
            Vertices[i] = aiVector3D((ai_real)newPos.x, (ai_real)newPos.y, (ai_real)newPos.z);
        }

        int nextFaceIndex = 0;//下一个新面要存入数组的位置
        for (int i = 0;i < m_Mesh->m_Faces.size();i++)
        {
            //将每个面分裂成边若干个四边形面
            std::vector<int> edgesIndex;
            std::vector<int> verticesIndex;
            edgesIndex.push_back(m_Mesh->m_Faces[i].edgeIndex);
            verticesIndex.push_back(m_Mesh->m_Edges[edgesIndex.back()].vertexIndex);
            while (m_Mesh->m_Edges[edgesIndex.back()].nextEdgeIndex != edgesIndex.front())
            {
                edgesIndex.push_back(m_Mesh->m_Edges[edgesIndex.back()].nextEdgeIndex);
                verticesIndex.push_back(m_Mesh->m_Edges[edgesIndex.back()].vertexIndex);
            }
            std::vector<int> edgeVertices;//新边点
            int n = edgesIndex.size();
            for (int j = 0;j < n;j++)
            {
                if (j != n - 1)
                    edgeVertices.push_back(cache[{verticesIndex[j], verticesIndex[j + 1]}]);
                else
                    edgeVertices.push_back(cache[{verticesIndex[n - 1], verticesIndex[0]}]);
            }
            int fv = m_Mesh->m_Vertices.size() + i;
            for (int j = 0;j < n;j++)
            {
                aiFace face;
                face.mNumIndices = 4;
                face.mIndices = new unsigned int[4];
                face.mIndices[0] = fv;
                face.mIndices[1] = edgeVertices[j];
                face.mIndices[2] = j + 1 < n ? verticesIndex[j + 1] : verticesIndex[0];
                face.mIndices[3] = j + 1 < n ? edgeVertices[j + 1] : edgeVertices[0];
                Faces[nextFaceIndex] = face;
                nextFaceIndex++;
            }
        }

        m_Mesh->Load(&mesh);//重新由新拓扑关系建立新网格

        return;
    }
}