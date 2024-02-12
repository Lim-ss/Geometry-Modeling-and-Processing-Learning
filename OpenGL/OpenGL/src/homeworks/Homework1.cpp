#include "Homework1.h"

#include <algorithm>
#include <functional>

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
        m_FunctionPoints(),
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_pointSize(10.0f),
        m_lineWidth(5.0f),
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

        glEnable(GL_DEPTH_TEST);

        m_Proj = glm::ortho<float>(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);

        //PreDrawFunction([](float x)->float {return sin(x);}, 0.1f);
    }

    Homework1::~Homework1()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void Homework1::OnUpdate(double deltaTime)
    {

    }

    void Homework1::OnRender()
    {
        m_MVP = m_Proj * m_View * m_Model;
        
        SortPoint();

        m_VBO->ReData(m_points.data(), sizeof(VertexBuffer::point) * m_indices.size());
        m_IBO->ReData(m_indices.data(), (unsigned int)m_indices.size());

        m_Shader->SetUniformMat4f("u_MVP", m_MVP);

        
        glClearColor(0.2f, 0.8f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPointSize(m_pointSize);
        glLineWidth(m_lineWidth);
        m_Shader->Bind();
        m_VAO->Bind();
        m_IBO->Bind();

        glDrawElements(GL_POINTS, m_indices.size(), GL_UNSIGNED_INT, nullptr);
        //glDrawElements(GL_LINE_STRIP, m_indices.size(), GL_UNSIGNED_INT, nullptr);

        GetInterpolationPolynomial();
        PreDrawFunction([this](float x) { return this->interpolationPolynomial(x); }, 0.1f);
        m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
        if(m_points.size() != 0)
            glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
    }

    void Homework1::OnImguiRender()
    {

        ImGui::Text("Press SPACE to clear points");

        ImGui::SliderFloat("Point Size", &m_pointSize, 2.0f, 30.0f);

        ImGui::SliderFloat("Line Width", &m_lineWidth, 2.0f, 30.0f);

        //ImGui::Checkbox("Wireframe Mode", &m_IfWireframeMode);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework1::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {

    }

    void Homework1::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            ClearPoint();
        }
    }

    void Homework1::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
        {
            double xpos, ypos;
            int width, height;
            glfwGetCursorPos(window, &xpos, &ypos);
            glfwGetWindowSize(window, &width, &height);
            float x = (xpos - ((float)width / 2.0f)) * 20.0f / float(width);
            float y = - (ypos - ((float)height / 2.0f)) * 20.0f / float(height);
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

    void Homework1::PreDrawFunction(std::function<float(float)> func, float increments)
    {
        m_FunctionPoints.clear();
        float x = -100.0f;
        while (x <= 100.0f)
        {
            m_FunctionPoints.push_back({ {x, func(x), 0}, {0.0f, 0.5f, 1.0f} });
            x += increments;
        }
        
    }

    void Homework1::GetInterpolationPolynomial()
    {
        int size = m_points.size();
        if (size == 0)
        {
            m_polynomialCoefficient.clear();
            return;//avoid using empty matrix
        }
        Eigen::MatrixXd matrix_A(size, size);
        Eigen::MatrixXd matrix_b(size, 1);
        for (int i = 0; i < size; i++)
        {
            double t = 1.0;
            for (int j = 0; j < size; j++)
            {
                matrix_A.coeffRef(i, j) = t;
                t = t * m_points[i].position.x;
            }
            matrix_b.coeffRef(i, 0) = m_points[i].position.y;
        }
        Eigen::MatrixXd matrix_x(size, 1);
        matrix_x = matrix_A.colPivHouseholderQr().solve(matrix_b);
        m_polynomialCoefficient.clear();
        m_polynomialCoefficient.insert(m_polynomialCoefficient.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());
    }

    float Homework1::interpolationPolynomial(float x)
    {
        double t = 1.0, y = 0.0;
        int size = m_points.size();
        for (int i = 0; i < size; i++)
        {
            y += t * m_polynomialCoefficient[i];
            t = t * x;
        }
        return (float)y;
    }


}