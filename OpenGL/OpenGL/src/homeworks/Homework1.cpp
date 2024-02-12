#include "Homework1.h"

#include <algorithm>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


namespace module {

    Homework1::Homework1()
        :
        m_points(),
        m_indices(),
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_IO(ImGui::GetIO())
    {
        m_VAO = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_points.data(), sizeof(VertexBuffer::point) * m_indices.size());
        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        m_VAO->AddBuffer(*m_VBO, layout);
        m_IBO = std::make_unique<IndexBuffer>(m_indices.data(), (unsigned int)m_indices.size());//ATTENTION! argument.2 is count,not size
        m_Shader = std::make_unique<Shader>("res/shaders/Homework1.shader");
        //m_Camera = std::make_unique<Camera>(m_View);

        glEnable(GL_DEPTH_TEST);

        m_Proj = glm::ortho<float>(-100.0f, 100.0f, -100.0f, 100.0f, -1.0f, 1.0f);

        /*AddPoint({ 100.0f,  100.0f,  0.0f }, { 1.0f, 0.0f, 0.0f });
        AddPoint({ 50.0f,  100.0f,  0.0f }, { 1.0f, 0.0f, 0.0f });
        AddPoint({ 50.0f,  50.0f,  0.0f }, { 1.0f, 0.0f, 0.0f });
        AddPoint({ 0.0f,  0.0f,  0.0f }, { 1.0f, 0.0f, 0.0f });*/
    }

    Homework1::~Homework1()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void Homework1::OnUpdate(double deltaTime)
    {
        //m_Camera->CameraUpdate(deltaTime);
    }

    void Homework1::OnRender()
    {
        m_MVP = m_Proj * m_View * m_Model;
        
        SortPoint();

        m_VBO->ReData(m_points.data(), sizeof(VertexBuffer::point) * m_indices.size());
        m_IBO->ReData(m_indices.data(), (unsigned int)m_indices.size());

        m_Shader->SetUniformMat4f("u_MVP", m_MVP);

        /* Render here */
        glClearColor(0.2f, 0.8f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPointSize(20.0f);
        m_Shader->Bind();
        m_VAO->Bind();
        m_IBO->Bind();

        glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, nullptr);
        glDrawElements(GL_LINE_STRIP, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }

    void Homework1::OnImguiRender()
    {

        ImGui::Text("Press ESC to disable the cursor");

        //ImGui::SliderFloat("fov", &m_Camera->fov, 30.0f, 120.0f);

        //ImGui::Checkbox("Wireframe Mode", &m_IfWireframeMode);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework1::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        //Camera::CursorPosCallback(window, xpos, ypos);
    }

    void Homework1::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        //Camera::KeyCallback(window, key, scancode, action, mods);
    }

    void Homework1::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
        {
            double xpos, ypos;
            int width, height;
            glfwGetCursorPos(window, &xpos, &ypos);
            glfwGetWindowSize(window, &width, &height);
            float x = (xpos - ((float)width / 2.0f)) * 200.0f / float(width);
            float y = - (ypos - ((float)height / 2.0f)) * 200.0f / float(height);
            AddPoint({ x, y, 0.0f }, { 1.0f, 0.0f, 0.0f });
        }
    }

    void Homework1::AddPoint(glm::vec3 position, glm::vec3 color)
    {
        m_points.push_back({ position, color });
        m_indices.push_back(m_indices.size());
    }

    void Homework1::ClearPoint()
    {
        m_points.clear();
        m_indices.clear();
    }

    void Homework1::SortPoint()
    {
        std::sort(m_points.begin(), m_points.end(), [](VertexBuffer::point a, VertexBuffer::point b) {return a.position.x > b.position.x;});
    }
}