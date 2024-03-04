#include "Homework9.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Eigen/Core"
#include "Eigen/Geometry"
#include "Eigen/Sparse"

namespace module {

    Homework9::Homework9()
        :
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_IO(ImGui::GetIO()),
        m_WireframeMode(false),
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
        m_Shader = std::make_unique<Shader>("res/shaders/Homework9.shader");
        m_Shader->Bind();
        m_Camera = std::make_unique<Camera>(m_View);

    }

    Homework9::~Homework9()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void Homework9::OnUpdate(double deltaTime)
    {
        m_Camera->CameraUpdate(deltaTime);
        m_Model = glm::scale(glm::mat4(1.0f), glm::vec3(pow(10.0f, m_scale)));//调整模型大小
    }

    void Homework9::OnRender()
    {
        if (m_WireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        int width, height;
        GLFWwindow* window = glfwGetCurrentContext();
        glfwGetWindowSize(window, &width, &height);
        m_Proj = glm::perspective(glm::radians(m_Camera->fov), (float)width / (float)height, 0.1f, 1000.0f);

        m_MVP = m_Proj * m_View * m_Model;
        m_Shader->SetUniformMat4f("u_MVP", m_MVP);
        m_Shader->SetUniformMat4f("u_Model", m_Model);
        m_Shader->SetUniform3f("u_LightPosition", 5.0f, 5.0f, 5.0f);
        m_Shader->SetUniform3f("u_LightColor", 1.0f, 1.0f, 1.0f);
        m_Shader->SetUniform3f("u_ViewPosition", m_Camera->m_cameraPos.x, m_Camera->m_cameraPos.y, m_Camera->m_cameraPos.z);

        /* Render here */
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        Renderer renderer;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Mesh->UpdateIndices();
        m_Mesh->UpdateNormals();
        m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
        m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Mesh->m_Indices.size());
    }

    void Homework9::OnImguiRender()
    {

        ImGui::Text("Press ESC to disable the cursor");

        ImGui::Checkbox("Wireframe Mode", &m_WireframeMode);

        ImGui::SliderFloat("model scale", &m_scale, -2.0f, 2.0f);

        ImGui::InputText("index", m_input, IM_ARRAYSIZE(m_input));
        // 在ImGui中创建按钮，用于确定坐标
        if (ImGui::Button("erase face"))
        {
            int index = atoi(m_input);
            if (index < m_Mesh->m_Faces.size())
            {
                printf("erase face:%d\n", index);
                m_Mesh->EraseFace(index);
            }
        }
        if (ImGui::Button("erase vertex"))
        {
            int index = atoi(m_input);
            if (index < m_Mesh->m_Vertices.size())
            {
                printf("erase vertex:%d\n", index);
                m_Mesh->EraseVertex(index);
            }
        }
        if (ImGui::Button("erase edge"))
        {
            int index = atoi(m_input);
            if (index < m_Mesh->m_Edges.size())
            {
                printf("erase edge:%d\n", index);
                m_Mesh->EraseEdge(index);
            }
        }

        if (ImGui::Button("print edges"))
        {
            m_Mesh->PrintHalfEdges();
        }
        if (ImGui::Button("print vertices"))
        {
            m_Mesh->PrintVertices();
        }
        if (ImGui::Button("print face"))
        {
            m_Mesh->PrintFaces();
        }

        if (ImGui::Button("load model 1"))
        {
            m_Mesh->Reload("res/mesh/Nefertiti_face.obj");
        }
        if (ImGui::Button("load model 2"))
        {
            m_Mesh->Reload("res/mesh/Balls.obj");
        }
        if (ImGui::Button("load model 3"))
        {
            m_Mesh->Reload("res/mesh/simpleCube.obj");
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework9::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Camera::CursorPosCallback(window, xpos, ypos);
    }

    void Homework9::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Camera::KeyCallback(window, key, scancode, action, mods);
    }

}