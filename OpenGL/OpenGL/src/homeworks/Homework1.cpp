#include "Homework1.h"

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

    Homework1::Homework1()
        :
        m_points(),
        m_indices(),
        m_FunctionPoints(),
        m_functionShouldUpdate(false),
        m_showfunction(),
        m_sigma(1.0f),
        m_highestPower(3),
        m_lambda(1.0f),
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

        if (m_points.size() != 0)
        {
            if (m_showfunction[0])
            {
                GetInterpolationPolynomial();
                PreDrawFunction([this](float x) { return this->interpolationPolynomial(x); }, 0.1f, {0.0f, 0.5f, 1.0f});
                m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
                glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
            }
            if (m_showfunction[1])
            {
                GetInterpolationGauss();
                PreDrawFunction([this](float x) { return this->interpolationGauss(x); }, 0.1f, { 1.0f, 0.3f, 1.0f });
                m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
                glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
            }
            if (m_showfunction[2])
            {
                GetApproximationPolynomial();
                PreDrawFunction([this](float x) { return this->approximationPolynomial(x); }, 0.1f, { 1.0f, 0.5f, 0.0f });
                m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
                glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
            }
            if (m_showfunction[3])
            {
                GetRidgeRegressionPolynomial();
                PreDrawFunction([this](float x) { return this->ridgeRegressionPolynomial(x); }, 0.1f, { 0.0f, 1.0f, 1.0f });
                m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
                glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
            }
        }         
    }

    void Homework1::OnImguiRender()
    {

        ImGui::Text("Press SPACE to clear points");

        ImGui::SliderFloat("Point Size", &m_pointSize, 2.0f, 30.0f);

        ImGui::SliderFloat("Line Width", &m_lineWidth, 2.0f, 30.0f);

        ImGui::SliderFloat("sigma of gauss", &m_sigma, 0.1f, 10.0f);

        ImGui::SliderFloat("lambda of ridge regression", &m_lambda, 0.0f, 10.0f);

        ImGui::SliderInt("highest power", &m_highestPower, 1, 10);

        ImGui::Checkbox("Polynomial Interpolation", &m_showfunction[0]);
        ImGui::Checkbox("Gauss Interpolation", &m_showfunction[1]);
        ImGui::Checkbox("Linear Regression", &m_showfunction[2]);
        ImGui::Checkbox("Ridge Regression", &m_showfunction[3]);

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
        m_functionShouldUpdate = true;
    }

    void Homework1::ClearPoint()
    {
        m_points.clear();
        m_indices.clear();
        m_functionShouldUpdate = true;
    }

    void Homework1::SortPoint()
    {
        std::sort(m_points.begin(), m_points.end(), [](VertexBuffer::point a, VertexBuffer::point b) {return a.position.x > b.position.x;});
    }

    void Homework1::PreDrawFunction(std::function<float(float)> func, float increments, glm::vec3 color)
    {
        if (m_functionShouldUpdate == false)
            return;
        m_FunctionPoints.clear();
        float x = -100.0f;
        while (x <= 100.0f)
        {
            m_FunctionPoints.push_back({ {x, func(x), 0}, color });
            x += increments;
        }
        m_functionShouldUpdate = true;
    }

    void Homework1::GetInterpolationPolynomial()
    {
        if (m_functionShouldUpdate == false)
            return;
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
        /*std::vector<double>::iterator it;
        for (it = m_polynomialCoefficient.begin(); it != m_polynomialCoefficient.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;*/
    }

    void Homework1::GetInterpolationGauss()
    {
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        if (size == 0)
        {
            m_polynomialCoefficient.clear();
            return;//avoid using empty matrix
        }
        if (size == 1)
        {
            m_gaussCoefficient.clear();
            m_gaussCoefficient.push_back(m_points[0].position.y);
            m_gaussCoefficient.push_back(0.0);
            return;
        }
        Eigen::MatrixXd matrix_A(size + 1, size + 1);
        Eigen::MatrixXd matrix_b(size + 1, 1);
        for (int i = 0; i < size; i++)
        {
            matrix_A.coeffRef(i, 0) = 1.0;
            for (int j = 1; j < size + 1; j++)
            {
                matrix_A.coeffRef(i, j) = exp(-pow(m_points[i].position.x - m_points[j-1].position.x, 2) / 2.0 / pow(m_sigma, 2));
            }
            matrix_b.coeffRef(i, 0) = m_points[i].position.y;
        }
        for (int i = 0; i < size + 1; i++)
        {
            matrix_A.coeffRef(size, i) = 1.0;
        }
        matrix_b.coeffRef(size, 0) = 1.0;
        Eigen::MatrixXd matrix_x(size + 1, 1);
        matrix_x = matrix_A.colPivHouseholderQr().solve(matrix_b);
        m_gaussCoefficient.clear();
        m_gaussCoefficient.insert(m_gaussCoefficient.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());

        /*std::cout << "A:\n" << matrix_A << std::endl << "b:\n" << matrix_b << std::endl << "x:\n" << matrix_x << std::endl;
        std::vector<double>::iterator it;
        for (it = m_gaussCoefficient.begin(); it != m_gaussCoefficient.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl <<std::endl;*/
    }

    void Homework1::GetApproximationPolynomial()
    {
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        Eigen::MatrixXd matrix_A(size, m_highestPower + 1);
        Eigen::MatrixXd matrix_b(size, 1);
        Eigen::MatrixXd matrix_x(m_highestPower + 1, 1);
        for (int i = 0; i < size; i++)
        {
            double t = 1.0;
            for (int j = 0; j < m_highestPower + 1; j++)
            {
                matrix_A.coeffRef(i, j) = t;
                t = t * m_points[i].position.x;
            }
            matrix_b.coeffRef(i, 0) = m_points[i].position.y;
        }
            
        matrix_x = matrix_A.colPivHouseholderQr().solve(matrix_b);//当方程组矛盾，会求最小二值解，无需手动干预

        m_approximationPolynomialCoefficient.clear();
        m_approximationPolynomialCoefficient.insert(m_approximationPolynomialCoefficient.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());

        /*std::vector<double>::iterator it;
        for (it = m_approximationPolynomialCoefficient.begin(); it != m_approximationPolynomialCoefficient.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;*/
    }

    void Homework1::GetRidgeRegressionPolynomial()
    {
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        Eigen::MatrixXd matrix_A(size, m_highestPower + 1);
        Eigen::MatrixXd matrix_b(size, 1);
        Eigen::MatrixXd matrix_x(m_highestPower + 1, 1);
        for (int i = 0; i < size; i++)
        {
            double t = 1.0;
            for (int j = 0; j < m_highestPower + 1; j++)
            {
                matrix_A.coeffRef(i, j) = t;
                t = t * m_points[i].position.x;
            }
            matrix_b.coeffRef(i, 0) = m_points[i].position.y;
        }

        //由于左侧矩阵对称正定，所以可以ldlt分解，比Householder Qr分解更好
         matrix_x = (matrix_A.transpose() * matrix_A + m_lambda * Eigen::MatrixXd::Identity(m_highestPower + 1, m_highestPower + 1)).ldlt().solve(matrix_A.transpose() * matrix_b);

         m_ridgeRegressionPolynomialCoefficient.clear();
         m_ridgeRegressionPolynomialCoefficient.insert(m_ridgeRegressionPolynomialCoefficient.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());

        /*std::vector<double>::iterator it;
        for (it = m_approximationPolynomialCoefficient.begin(); it != m_approximationPolynomialCoefficient.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;*/
    }

    float Homework1::interpolationPolynomial(float x)
    {
        //这里最好改用秦九昭算法，还没来得及写
        double t = 1.0, y = 0.0;
        int size = m_points.size();
        for (int i = 0; i < size; i++)
        {
            y += t * m_polynomialCoefficient[i];
            t = t * x;
        }
        return (float)y;
    }

    float Homework1::interpolationGauss(float x)
    {
        double y = m_gaussCoefficient[0];
        int size = m_points.size();
        for (int i = 0; i < size; i++)
        {
            y += exp(-pow(x - m_points[i].position.x, 2) / 2.0 / pow(m_sigma, 2)) * m_gaussCoefficient[i + 1];
        }
        return (float)y;
    }

    float Homework1::approximationPolynomial(float x)
    {
        //这里最好改用秦九昭算法，还没来得及写
        double t = 1.0, y = 0.0;
        int size = m_points.size();
        for (int i = 0; i < m_highestPower + 1; i++)
        {
            y += t * m_approximationPolynomialCoefficient[i];
            t = t * x;
        }
        return (float)y;
    }

    
    float Homework1::ridgeRegressionPolynomial(float x)
    {
        //这里最好改用秦九昭算法，还没来得及写
        double t = 1.0, y = 0.0;
        int size = m_points.size();
        for (int i = 0; i < m_highestPower + 1; i++)
        {
            y += t * m_ridgeRegressionPolynomialCoefficient[i];
            t = t * x;
        }
        return (float)y;
    }
}