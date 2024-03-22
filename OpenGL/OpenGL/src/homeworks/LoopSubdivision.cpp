#include "LoopSubdivision.h"

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

static const float pi = 3.1415926535;

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

    LoopSubdivision::LoopSubdivision()
        :
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_IO(ImGui::GetIO()),
        m_scale(0.0f)
    {
        m_Mesh = std::make_unique<HE::Mesh>("res/mesh/Nefertiti_face.obj");

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        m_VAO = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
        VertexBufferLayout layout;
        layout.Vacate(sizeof(HE::Vertex::edgeIndex));//edge
        layout.Push<float>(3);//position
        layout.Push<float>(3);//color
        layout.Push<float>(3);//normal
        layout.Push<float>(2);//texCoord
        m_VAO->AddBuffer(*m_VBO, layout);
        m_IBO = std::make_unique<IndexBuffer>(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        m_Shader = std::make_unique<Shader>("res/shaders/MeshSubdiv.shader");
        m_Shader->Bind();
        m_Camera = std::make_unique<Camera>(m_View);

        glEnable(GL_POLYGON_OFFSET_FILL); // 启用填充面的多边形偏移
        glPolygonOffset(0.5f, 0.1f); // 设置多边形偏移因子和单位

        
    }

    LoopSubdivision::~LoopSubdivision()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    void LoopSubdivision::OnUpdate(double deltaTime)
    {
        m_Camera->CameraUpdate(deltaTime);
        m_Model = glm::scale(glm::mat4(1.0f), glm::vec3(pow(10.0f, m_scale)));//调整模型大小
    }

    void LoopSubdivision::OnRender()
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

        /*m_Mesh->UpdateIndices();
        m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
        m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());*/

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_Shader->SetUniform1i("u_Mode", 3);
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Mesh->m_Indices.size());
        m_Shader->SetUniform1i("u_Mode", 2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Mesh->m_Indices.size());
    }

    void LoopSubdivision::OnImguiRender()
    {
        if (ImGui::Button("Loop Subdiv"))
        {
            LoopSubdiv();
            m_Mesh->UpdateIndices();
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        }
        if (ImGui::Button("load model 1"))
        {
            m_Mesh->Reload("res/mesh/Nefertiti_face.obj");
            m_Mesh->UpdateIndices();
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        }
        if (ImGui::Button("load model 2"))
        {
            m_Mesh->Reload("res/mesh/SimpleCube.obj");
            m_Mesh->UpdateIndices();
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        }
        if (ImGui::Button("load model 3"))
        {
            m_Mesh->Reload("res/mesh/Balls.obj");
            m_Mesh->UpdateIndices();
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        }
        if (ImGui::Button("print"))
        {
            printf("face:%d,edges:%d,vertex:%d\n", m_Mesh->m_Faces.size(), m_Mesh->m_Edges.size(), m_Mesh->m_Vertices.size());
        }
        ImGui::SliderFloat("model scale", &m_scale, -2.0f, 2.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void LoopSubdivision::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Camera::CursorPosCallback(window, xpos, ypos);
    }

    void LoopSubdivision::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Camera::KeyCallback(window, key, scancode, action, mods);
    }

    void LoopSubdivision::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Camera::MouseButtonCallback(window, button, action, mods);
    }

    void LoopSubdivision::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Camera::ScrollCallback(window, xoffset, yoffset);
    }

    void LoopSubdivision::LoopSubdiv()
    {
        std::unordered_map<std::tuple<int, int>, int, TupleHash, TupleEqual> cache;//用于存储每条边对应的新点
        aiVector3D* Vertices = new aiVector3D[m_Mesh->m_Vertices.size() + m_Mesh->m_Edges.size() / 2];
        if (Vertices == nullptr)
        {
            printf("Vertices allocate failed\n");
        }
        aiFace* Faces = new aiFace[m_Mesh->m_Faces.size() * 4];
        if (Faces == nullptr)
        {
            printf("Faces allocate failed\n");
        }
        aiMesh mesh;
        mesh.mNumVertices = m_Mesh->m_Vertices.size() + m_Mesh->m_Edges.size() / 2;
        mesh.mNumFaces = m_Mesh->m_Faces.size() * 4;
        mesh.mVertices = Vertices;
        mesh.mFaces = Faces;

        for (int i = 0;i < m_Mesh->m_Vertices.size();i++)
        {
            //更新每个旧顶点的坐标，按原顺序存入临时数组Vertices中
            if (m_Mesh->IsBoundaryVertex(i))
            {
                //边界点，不更新位置
                aiVector3D newPos((ai_real)m_Mesh->m_Vertices[i].position.x, (ai_real)m_Mesh->m_Vertices[i].position.y, (ai_real)m_Mesh->m_Vertices[i].position.z);
                Vertices[i] = newPos;
                continue;
            }
            int num = 1;//邻域点数之和，初始化为0
            int e0 = m_Mesh->m_Vertices[i].edgeIndex;//第一条出边
            glm::vec3 sumPosition = m_Mesh->m_Vertices[m_Mesh->m_Edges[e0].vertexIndex].position;//邻域坐标之和，初始化为第一个邻域点坐标
            int e = m_Mesh->m_Edges[m_Mesh->m_Edges[e0].oppositeEdgeIndex].nextEdgeIndex;//初始化为第二条出边
            while (e != e0)
            {
                num++;
                sumPosition += m_Mesh->m_Vertices[m_Mesh->m_Edges[e].vertexIndex].position;
                e = m_Mesh->m_Edges[m_Mesh->m_Edges[e].oppositeEdgeIndex].nextEdgeIndex;
            }
            float beta = 1.0f / num * (0.625f - pow(0.375f + 0.25f * std::cos(2 * pi / num), 2));
            glm::vec3 newPosition = (1 - num * beta) * m_Mesh->m_Vertices[i].position + beta * sumPosition;
            aiVector3D newPos((ai_real)newPosition.x, (ai_real)newPosition.y, (ai_real)newPosition.z);
            Vertices[i] = newPos;
        }
        int index = m_Mesh->m_Vertices.size();//下一个新点要存入数组的位置
        for (int i = 0;i < m_Mesh->m_Edges.size();i++)
        {
            //添加每条边对应的新点
            //v1->v2
            int v1 = m_Mesh->m_Edges[m_Mesh->m_Edges[i].oppositeEdgeIndex].vertexIndex;
            int v2 = m_Mesh->m_Edges[i].vertexIndex;
            if (cache.find({ v1, v2 }) != cache.end())
            {
                //已经创建过了
                continue;
            }
            cache[{v1, v2}] = index;
            glm::vec3 position;
            int f1 = m_Mesh->m_Edges[i].faceIndex;
            int f2 = m_Mesh->m_Edges[m_Mesh->m_Edges[i].oppositeEdgeIndex].faceIndex;
            if (f1 == -1 || f2 == -1)
            {
                //边界
                position = 0.5f * m_Mesh->m_Vertices[v1].position + 0.5f * m_Mesh->m_Vertices[v2].position;
            }
            else
            {
                //非边界
                int v3 = m_Mesh->m_Edges[m_Mesh->m_Edges[i].nextEdgeIndex].vertexIndex;
                int v4 = m_Mesh->m_Edges[m_Mesh->m_Edges[m_Mesh->m_Edges[i].oppositeEdgeIndex].nextEdgeIndex].vertexIndex;
                position = 0.375f * m_Mesh->m_Vertices[v1].position + 0.375f * m_Mesh->m_Vertices[v2].position
                    + 0.125f * m_Mesh->m_Vertices[v3].position + 0.125f * m_Mesh->m_Vertices[v4].position;
            }
            aiVector3D newPos((ai_real)position.x, (ai_real)position.y, (ai_real)position.z);
            Vertices[index] = newPos;
            index++;
        }
        for (int i = 0;i < m_Mesh->m_Faces.size();i++)
        {
            //将每个面分裂成四个新面
            int v1 = m_Mesh->m_Edges[m_Mesh->m_Faces[i].edgeIndex].vertexIndex;
            int v2 = m_Mesh->m_Edges[m_Mesh->m_Edges[m_Mesh->m_Faces[i].edgeIndex].nextEdgeIndex].vertexIndex;
            int v3 = m_Mesh->m_Edges[m_Mesh->m_Edges[m_Mesh->m_Edges[m_Mesh->m_Faces[i].edgeIndex].nextEdgeIndex].nextEdgeIndex].vertexIndex;
            int v4 = cache[{v1, v2}];
            int v5 = cache[{v2, v3}];
            int v6 = cache[{v3, v1}];
            aiFace face;
            face.mNumIndices = 3;
            face.mIndices = new unsigned int[3];
            face.mIndices[0] = v1;
            face.mIndices[1] = v4;
            face.mIndices[2] = v6;
            Faces[4 * i + 0] = face;
            face.mIndices = new unsigned int[3];
            face.mIndices[0] = v2;
            face.mIndices[1] = v4;
            face.mIndices[2] = v5;
            Faces[4 * i + 1] = face;
            face.mIndices = new unsigned int[3];
            face.mIndices[0] = v3;
            face.mIndices[1] = v5;
            face.mIndices[2] = v6;
            Faces[4 * i + 2] = face;
            face.mIndices = new unsigned int[3];
            face.mIndices[0] = v4;
            face.mIndices[1] = v5;
            face.mIndices[2] = v6;
            Faces[4 * i + 3] = face;
        }

        m_Mesh->Reload(&mesh);//重新由新拓扑关系建立新网格

        return;
    }

}