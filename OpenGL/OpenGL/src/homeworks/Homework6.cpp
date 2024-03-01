#include "Homework6.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace module {

    Homework6::Homework6()
        :
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_IO(ImGui::GetIO()),
        m_WireframeMode(false),
        m_UpdateMesh(false)
    {
        //m_Mesh = std::make_unique<HE::Mesh>("res/mesh/Nefertiti_face.obj");
        m_Mesh = std::make_unique<HE::Mesh>("res/mesh/Balls.obj");
        //m_Mesh = std::make_unique<HE::Mesh>("res/mesh/simpleCube.obj");
        //m_Mesh = std::make_unique<HE::Mesh>("res/mesh/triangle.obj");

        m_Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));//调整模型大小
        //m_Mesh->PrintVertices();
        //m_Mesh->PrintIndices();
        //m_Mesh->PrintHalfEdges();
        //m_Mesh->PrintMeanCurvatureVector();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        m_VAO = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
        VertexBufferLayout layout;
        layout.Vacate(sizeof(HE::Vertex::edgeIndex));//edge
        layout.Push<float>(3);//position
        //layout.Push<float>(3);//normal
        //layout.Push<float>(2);//texCoord
        m_VAO->AddBuffer(*m_VBO, layout);
        m_IBO = std::make_unique<IndexBuffer>(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        m_Shader = std::make_unique<Shader>("res/shaders/Homework6.shader");
        m_Shader->Bind();
        m_Camera = std::make_unique<Camera>(m_View);

    }

    Homework6::~Homework6()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void Homework6::OnUpdate(double deltaTime)
    {
        m_Camera->CameraUpdate(deltaTime);
        if (m_UpdateMesh)
            MinimalSurfaceLocalMethod(0.05);
    }

    void Homework6::OnRender()
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

        /* Render here */
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        Renderer renderer;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE::Vertex) * m_Mesh->m_Vertices.size());
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Mesh->m_Indices.size());
    }

    void Homework6::OnImguiRender()
    {

        ImGui::Text("Press ESC to disable the cursor");

        ImGui::Checkbox("Wireframe Mode", &m_WireframeMode);

        ImGui::Checkbox("Update Mesh", &m_UpdateMesh);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework6::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Camera::CursorPosCallback(window, xpos, ypos);
    }

    void Homework6::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Camera::KeyCallback(window, key, scancode, action, mods);
    }

    void Homework6::MinimalSurfaceLocalMethod(float lambda)
    {
        std::vector<glm::vec3> TemporaryPosition;
        TemporaryPosition.resize(m_Mesh->m_Vertices.size());
        for (int i = 0;i < m_Mesh->m_Vertices.size();i++)
        {
            //TemporaryPosition[i] = m_Mesh->m_Vertices[i].position - lambda * m_Mesh->Laplace_Beltrami_Operator(i);
            TemporaryPosition[i] = m_Mesh->m_Vertices[i].position - lambda * m_Mesh->Laplace_Operator(i);
        }
        for (int i = 0;i < m_Mesh->m_Vertices.size();i++)
        {
            m_Mesh->m_Vertices[i].position = TemporaryPosition[i];
        }
    }
}