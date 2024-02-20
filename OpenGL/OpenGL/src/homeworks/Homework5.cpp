#include "Homework5.h"

#include <algorithm>
#include <functional>
#include <cmath>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


namespace module {

    Homework5::Homework5()
        :
        m_pointsA(),
        m_pointsB(),
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_pointSize(5.0f),
        m_lineWidth(3.0f),
        m_alpha(0.125f),
        m_link(false),
        m_currentBuffer(CurrentBuffer::A),
        m_IO(ImGui::GetIO())
    {
        m_VAO = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_pointsA.data(), sizeof(VertexBuffer::point) * m_pointsA.size());
        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        m_VAO->AddBuffer(*m_VBO, layout);
        m_Shader = std::make_unique<Shader>("res/shaders/Homework1.shader");

        m_Proj = glm::ortho<float>(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);
        m_MVP = m_Proj * m_View * m_Model;
    }

    Homework5::~Homework5()
    {

    }

    void Homework5::OnUpdate(double deltaTime)
    {

    }

    void Homework5::OnRender()
    {
        glClearColor(0.2f, 0.8f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(m_pointSize);
        glLineWidth(m_lineWidth);
        m_Shader->Bind();
        m_Shader->SetUniformMat4f("u_MVP", m_MVP);
        m_VAO->Bind();
        
        if (m_currentBuffer == A)
        {
            m_VBO->ReData(m_pointsA.data(), sizeof(VertexBuffer::point) * m_pointsA.size());
            glDrawArrays(GL_LINE_STRIP, 0, m_pointsA.size());
            glDrawArrays(GL_POINTS, 0, m_pointsA.size());
        }
        else
        {
            m_VBO->ReData(m_pointsB.data(), sizeof(VertexBuffer::point) * m_pointsB.size());
            glDrawArrays(GL_LINE_STRIP, 0, m_pointsB.size());
            glDrawArrays(GL_POINTS, 0, m_pointsB.size());
        }
    }

    void Homework5::OnImguiRender()
    {
        ImGui::Text("Press SPACE to clear points");

        if (ImGui::Button("Link"))
            Link();//首尾相接
        if (ImGui::Button("Chaiukin2"))
            Chaiukin2();
        if (ImGui::Button("Chaiukin3"))
            Chaiukin3();
        if (ImGui::Button("4 point subdivision"))
            fourPointInterpolatorySubdivision();
        if (ImGui::Button("Clear Point"))
            ClearPoint();

        ImGui::SliderFloat("alpha", &m_alpha, 0.0f, 0.5f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework5::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {

    }

    void Homework5::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            ClearPoint();
        }
    }

    void Homework5::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            double xpos, ypos;
            int width, height;
            glfwGetCursorPos(window, &xpos, &ypos);
            glfwGetWindowSize(window, &width, &height);
            float x = (xpos - ((float)width / 2.0f)) * 20.0f / float(width);
            float y = -(ypos - ((float)height / 2.0f)) * 20.0f / float(height);
            AddPoint({ x, y, 0.0f }, { 1.0f, 0.0f, 0.0f });
        }
    }

    void Homework5::AddPoint(glm::vec3 position, glm::vec3 color)
    {
        if (m_link == false)
        {
            if(m_currentBuffer == A)
                m_pointsA.push_back({ position, color });
            else
                m_pointsB.push_back({ position, color });
        }
    }

    void Homework5::ClearPoint()
    {
        m_link = false;
        m_pointsA.clear();
        m_pointsB.clear();
    }

    void Homework5::Link()
    {
        if (m_link == false && (m_pointsA.size() > 1 || m_pointsB.size() > 1) )
        {
            m_link = true;
            if (m_currentBuffer == A)
                m_pointsA.push_back(m_pointsA[0]);
            else
                m_pointsB.push_back(m_pointsB[0]);
        }
    }

    void Homework5::UnLink()
    {
        if (m_link == false)
            return;
        if (m_currentBuffer == A)
            m_pointsA.pop_back();
        else
            m_pointsB.pop_back();
        m_link = false;
    }

    void Homework5::Chaiukin2()
    {
        if (m_link == false)//只处理封闭曲线
            return;


        if (m_currentBuffer == A)
        {
            UnLink();
            for (int i = 0;i < m_pointsA.size() - 1; i++)
            {
                m_pointsB.push_back({ 0.75f * m_pointsA[i].position + 0.25f * m_pointsA[i + 1].position, {1.0f, 0.0f, 0.0f} });
                m_pointsB.push_back({ 0.25f * m_pointsA[i].position + 0.75f * m_pointsA[i + 1].position, {1.0f, 0.0f, 0.0f} });

            }
            m_pointsB.push_back({ 0.75f * m_pointsA[m_pointsA.size() - 1].position + 0.25f * m_pointsA[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsB.push_back({ 0.25f * m_pointsA[m_pointsA.size() - 1].position + 0.75f * m_pointsA[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsA.clear();
            m_currentBuffer = B;
            Link();
        }
        else
        {
            UnLink();
            for (int i = 0;i < m_pointsB.size() - 1; i++)
            {
                m_pointsA.push_back({ 0.75f * m_pointsB[i].position + 0.25f * m_pointsB[i + 1].position, {1.0f, 0.0f, 0.0f} });
                m_pointsA.push_back({ 0.25f * m_pointsB[i].position + 0.75f * m_pointsB[i + 1].position, {1.0f, 0.0f, 0.0f} });
            }
            m_pointsA.push_back({ 0.75f * m_pointsB[m_pointsB.size() - 1].position + 0.25f * m_pointsB[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsA.push_back({ 0.25f * m_pointsB[m_pointsB.size() - 1].position + 0.75f * m_pointsB[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsB.clear();
            m_currentBuffer = A;
            Link();
        }
    }

    void Homework5::Chaiukin3()
    {
        if (m_link == false)//只处理封闭曲线
            return;

        if (m_currentBuffer == A)
        {
            UnLink();
            m_pointsB.push_back({ 0.75f * m_pointsA[0].position + 0.125f * m_pointsA[m_pointsA.size() - 1].position + 0.125f * m_pointsA[1].position, {1.0f, 0.0f, 0.0f} });
            m_pointsB.push_back({ 0.5f * m_pointsA[0].position + 0.5f * m_pointsA[1].position, {1.0f, 0.0f, 0.0f} });
            for (int i = 1;i < m_pointsA.size() - 1; i++)
            {
                m_pointsB.push_back({ 0.75f * m_pointsA[i].position + 0.125f * m_pointsA[i - 1].position + 0.125f * m_pointsA[i + 1].position, {1.0f, 0.0f, 0.0f} });
                m_pointsB.push_back({ 0.5f * m_pointsA[i].position + 0.5f * m_pointsA[i + 1].position, {1.0f, 0.0f, 0.0f} });
            }
            m_pointsB.push_back({ 0.75f * m_pointsA[m_pointsA.size() - 1].position + 0.125f * m_pointsA[m_pointsA.size() - 2].position + 0.125f * m_pointsA[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsB.push_back({ 0.5f * m_pointsA[m_pointsA.size() - 1].position + 0.5f * m_pointsA[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsA.clear();
            m_currentBuffer = B;
            Link();
        }
        else
        {
            UnLink();
            m_pointsA.push_back({ 0.75f * m_pointsB[0].position + 0.125f * m_pointsB[m_pointsB.size() - 1].position + 0.125f * m_pointsB[1].position, {1.0f, 0.0f, 0.0f} });
            m_pointsA.push_back({ 0.5f * m_pointsB[0].position + 0.5f * m_pointsB[1].position, {1.0f, 0.0f, 0.0f} });
            for (int i = 1;i < m_pointsB.size() - 1; i++)
            {
                m_pointsA.push_back({ 0.75f * m_pointsB[i].position + 0.125f * m_pointsB[i - 1].position + 0.125f * m_pointsB[i + 1].position, {1.0f, 0.0f, 0.0f} });
                m_pointsA.push_back({ 0.5f * m_pointsB[i].position + 0.5f * m_pointsB[i + 1].position, {1.0f, 0.0f, 0.0f} });
            }
            m_pointsA.push_back({ 0.75f * m_pointsB[m_pointsB.size() - 1].position + 0.125f * m_pointsB[m_pointsB.size() - 2].position + 0.125f * m_pointsB[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsA.push_back({ 0.5f * m_pointsB[m_pointsB.size() - 1].position + 0.5f * m_pointsB[0].position, {1.0f, 0.0f, 0.0f} });
            m_pointsB.clear();
            m_currentBuffer = A;
            Link();
        }
    }

    void Homework5::fourPointInterpolatorySubdivision()
    {
        if (m_link == false)//只处理封闭曲线
            return;

        if (m_currentBuffer == A)
        {
            UnLink();
            m_pointsB.push_back(m_pointsA[0]);
            m_pointsB.push_back({ 0.5f * (1 + m_alpha) * (m_pointsA[0].position + m_pointsA[1].position) - 0.5f * m_alpha * (m_pointsA[m_pointsA.size() - 1].position + m_pointsA[2].position), {1.0f, 0.0f, 0.0f} });
            for (int i = 1;i < m_pointsA.size() - 2; i++)
            {
                m_pointsB.push_back(m_pointsA[i]);
                m_pointsB.push_back({ 0.5f * (1 + m_alpha) * (m_pointsA[i].position + m_pointsA[i + 1].position) - 0.5f * m_alpha * (m_pointsA[i - 1].position + m_pointsA[i + 1].position), {1.0f, 0.0f, 0.0f} });
            }
            m_pointsB.push_back(m_pointsA[m_pointsA.size() - 2]);
            m_pointsB.push_back({ 0.5f * (1 + m_alpha) * (m_pointsA[m_pointsA.size() - 2].position + m_pointsA[m_pointsA.size() - 1].position) - 0.5f * m_alpha * (m_pointsA[m_pointsA.size() - 3].position + m_pointsA[0].position), {1.0f, 0.0f, 0.0f} });
            m_pointsB.push_back(m_pointsA[m_pointsA.size() - 1]);
            m_pointsB.push_back({ 0.5f * (1 + m_alpha) * (m_pointsA[m_pointsA.size() - 1].position + m_pointsA[0].position) - 0.5f * m_alpha * (m_pointsA[m_pointsA.size() - 2].position + m_pointsA[1].position), {1.0f, 0.0f, 0.0f} });
            m_pointsA.clear();
            m_currentBuffer = B;
            Link();
        }
        else
        {
            UnLink();
            m_pointsA.push_back(m_pointsB[0]);
            m_pointsA.push_back({ 0.5f * (1 + m_alpha) * (m_pointsB[0].position + m_pointsB[1].position) - 0.5f * m_alpha * (m_pointsB[m_pointsB.size() - 1].position + m_pointsB[2].position), {1.0f, 0.0f, 0.0f} });
            for (int i = 1;i < m_pointsB.size() - 2; i++)
            {
                m_pointsA.push_back(m_pointsB[i]);
                m_pointsA.push_back({ 0.5f * (1 + m_alpha) * (m_pointsB[i].position + m_pointsB[i + 1].position) - 0.5f * m_alpha * (m_pointsB[i - 1].position + m_pointsB[i + 1].position), {1.0f, 0.0f, 0.0f} });
            }
            m_pointsA.push_back(m_pointsB[m_pointsB.size() - 2]);
            m_pointsA.push_back({ 0.5f * (1 + m_alpha) * (m_pointsB[m_pointsB.size() - 2].position + m_pointsB[m_pointsB.size() - 1].position) - 0.5f * m_alpha * (m_pointsB[m_pointsB.size() - 3].position + m_pointsB[0].position), {1.0f, 0.0f, 0.0f} });
            m_pointsA.push_back(m_pointsB[m_pointsB.size() - 1]);
            m_pointsA.push_back({ 0.5f * (1 + m_alpha) * (m_pointsB[m_pointsB.size() - 1].position + m_pointsB[0].position) - 0.5f * m_alpha * (m_pointsB[m_pointsB.size() - 2].position + m_pointsB[1].position), {1.0f, 0.0f, 0.0f} });
            m_pointsB.clear();
            m_currentBuffer = A;
            Link();
        }
    }
}