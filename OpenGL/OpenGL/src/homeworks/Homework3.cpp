#include "Homework3.h"

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

    Homework3::Homework3()
        :
        m_points(),
        m_indices(),
        m_FunctionPoints(),
        m_parameter(),
        m_functionShouldUpdate(false),
        m_showfunction(),
        m_highestPower(3),
        m_lambda(0.0f),
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
    }

    Homework3::~Homework3()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void Homework3::OnUpdate(double deltaTime)
    {

    }

    void Homework3::OnRender()
    {
        m_MVP = m_Proj * m_View * m_Model;

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

        if (m_points.size() > 1)
        {
            GenerateParameter(Homework3::uniform);
            GetFittingFunction();
            PreDrawFunction({ 0.0f, 1.0f, 1.0f });
            m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
            glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
            
        }
    }

    void Homework3::OnImguiRender()
    {

        ImGui::Text("Press SPACE to clear points");

        ImGui::SliderFloat("Point Size", &m_pointSize, 2.0f, 30.0f);

        ImGui::SliderFloat("Line Width", &m_lineWidth, 2.0f, 30.0f);

        ImGui::SliderFloat("lambda of ridge regression", &m_lambda, 0.0f, 1.0f);

        ImGui::SliderInt("highest power", &m_highestPower, 1, 10);

        /*ImGui::Checkbox("Polynomial Interpolation", &m_showfunction[0]);
        ImGui::Checkbox("Gauss Interpolation", &m_showfunction[1]);
        ImGui::Checkbox("Linear Regression", &m_showfunction[2]);
        ImGui::Checkbox("Ridge Regression", &m_showfunction[3]);*/

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework3::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {

    }

    void Homework3::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            ClearPoint();
        }
    }

    void Homework3::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
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

    void Homework3::AddPoint(glm::vec3 position, glm::vec3 color)
    {
        m_points.push_back({ position, color });
        m_indices.push_back(m_indices.size());
        m_functionShouldUpdate = true;
    }

    void Homework3::ClearPoint()
    {
        m_points.clear();
        m_indices.clear();
        m_functionShouldUpdate = true;
    }

    void Homework3::SortPoint()
    {
        std::sort(m_points.begin(), m_points.end(), [](VertexBuffer::point a, VertexBuffer::point b) {return a.position.x > b.position.x;});
    }

    void Homework3::PreDrawFunction(glm::vec3 color)
    {
        if (m_functionShouldUpdate == false)
            return;
        m_FunctionPoints.clear();
        float t = -0.0f;
        while (t <= 1.0f)
        {
            m_FunctionPoints.push_back({ {fittingFunctionX(t), fittingFunctionY(t), 0}, color});
            t += 0.01f;
        }
        m_functionShouldUpdate = true;//bug here
    }

    void Homework3::GenerateParameter(ParameterMethod method)
    {
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        if (size == 1)
            return;
        switch (method)
        {
        case Homework3::uniform:
        {
            float step = 1.0f / (size - 1);
            float t = 0.0f;
            m_parameter.clear();
            for (int i = 0; i < size; i++)
            {
                m_parameter.push_back(t);
                t += step;
            }
            break;
        }

        case Homework3::chord_length:
            break;
        case Homework3::centripetal:
            break;
        default:
            break;
        }
    }

    void Homework3::GetFittingFunction()
    {
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        if (size == 1)
            return;
        Eigen::MatrixXd matrix_A(size, m_highestPower + 1);
        Eigen::MatrixXd matrix_b1(size, 1);
        Eigen::MatrixXd matrix_b2(size, 1);
        Eigen::MatrixXd matrix_x(m_highestPower + 1, 1);
        for (int i = 0; i < size; i++)
        {
            double t = 1.0;
            for (int j = 0; j < m_highestPower + 1; j++)
            {
                matrix_A.coeffRef(i, j) = t;
                t = t * m_parameter[i];
            }
            matrix_b1.coeffRef(i, 0) = m_points[i].position.x;
            matrix_b2.coeffRef(i, 0) = m_points[i].position.y;
        }
        m_Coefficient_X.clear();
        m_Coefficient_Y.clear();
        //由于左侧矩阵对称正定，所以可以ldlt分解，比Householder Qr分解更好
        matrix_x = (matrix_A.transpose() * matrix_A + m_lambda * Eigen::MatrixXd::Identity(m_highestPower + 1, m_highestPower + 1)).ldlt().solve(matrix_A.transpose() * matrix_b1);
        m_Coefficient_X.insert(m_Coefficient_X.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());
        matrix_x = (matrix_A.transpose() * matrix_A + m_lambda * Eigen::MatrixXd::Identity(m_highestPower + 1, m_highestPower + 1)).ldlt().solve(matrix_A.transpose() * matrix_b2);
        m_Coefficient_Y.insert(m_Coefficient_Y.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());

        /*std::vector<double>::iterator it;
        for (it = m_Coefficient_X.begin(); it != m_Coefficient_X.end(); ++it) {
             std::cout << *it << " ";
        }
        std::cout << std::endl;
        for (it = m_Coefficient_Y.begin(); it != m_Coefficient_Y.end(); ++it) {
             std::cout << *it << " ";
        }
        std::cout << std::endl << std::endl;*/
    }

    float Homework3::fittingFunctionX(float x)
    {
        //这里最好改用秦九昭算法，还没来得及写
        double t = 1.0, y = 0.0;
        int size = m_points.size();
        for (int i = 0; i < m_highestPower + 1; i++)
        {
            y += t * m_Coefficient_X[i];
            t = t * x;
        }
        return (float)y;
    }

    float Homework3::fittingFunctionY(float x)
    {
        //这里最好改用秦九昭算法，还没来得及写
        double t = 1.0, y = 0.0;
        int size = m_points.size();
        for (int i = 0; i < m_highestPower + 1; i++)
        {
            y += t * m_Coefficient_Y[i];
            t = t * x;
        }
        return (float)y;
    }
}