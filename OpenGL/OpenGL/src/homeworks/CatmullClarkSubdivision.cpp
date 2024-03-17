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
        if (ImGui::Button("Loop Subdiv"))
        {
            ;
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
            m_Mesh->Load("res/mesh/SimpleCube.obj");
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

}