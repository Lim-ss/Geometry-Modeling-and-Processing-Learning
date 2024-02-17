#include "Homework4.h"

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

    Homework4::Homework4()
        :
        m_points(),
        m_indices(),
        m_FunctionPoints(),
        m_pointOnCursor(-1),
        m_parameter(),
        m_functionShouldUpdate(false),
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

    Homework4::~Homework4()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void Homework4::OnUpdate(double deltaTime)
    {

    }

    void Homework4::OnRender()
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

        if (m_points.size() > 1)
        {
            if (m_mode == 0)
            {
                GetFittingFunction();
                PreDrawFunction({ 0.0f, 1.0f, 1.0f });
                m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
                glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
                //glDrawArrays(GL_POINTS, 0, m_FunctionPoints.size());
            }
            else if (m_mode == 1)
            {
                GenerateParameter();
                GetFittingCurve();
                PreDrawCurve({ 0.0f, 1.0f, 1.0f });
                m_VBO->ReData(m_FunctionPoints.data(), sizeof(VertexBuffer::point) * m_FunctionPoints.size());
                glDrawArrays(GL_LINE_STRIP, 0, m_FunctionPoints.size());
                //glDrawArrays(GL_POINTS, 0, m_FunctionPoints.size());
            }
        }
    }

    void Homework4::OnImguiRender()
    {

        ImGui::Text("Press DELETE to delete one point and SPACE to clear all points");
        //ImGui::SliderFloat("Point Size", &m_pointSize, 2.0f, 30.0f);
        //ImGui::SliderFloat("Line Width", &m_lineWidth, 2.0f, 30.0f);

        ImGui::Text("mode:");
        ImGui::RadioButton("function", &m_mode, 0);
        ImGui::RadioButton("curve", &m_mode, 1);

        if (m_mode == 1)
        {
            ImGui::Text("parameter method:");
            ImGui::RadioButton("uniform", &m_parameterMethod, 0);
            ImGui::RadioButton("chord_length", &m_parameterMethod, 1);
        }
        
        // 在ImGui中创建输入框，用于手动输入坐标

        ImGui::InputText("X", m_inputX, IM_ARRAYSIZE(m_inputX));
        ImGui::InputText("Y", m_inputY, IM_ARRAYSIZE(m_inputY));
        // 在ImGui中创建按钮，用于确定坐标
        if (ImGui::Button("Confirm"))
        {
            float x = atof(m_inputX);
            float y = atof(m_inputY);
            printf("Add Point: (%.2f, %.2f)\n", x, y);
            AddPoint({ x, y, 0.0f }, { 1.0f, 0.0f, 0.0f });
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework4::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        int size = m_points.size();

        if (m_pointOnCursor >= size)
            m_pointOnCursor = -1;

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float x = (xpos - ((float)width / 2.0f)) * 20.0f / float(width);
        float y = -(ypos - ((float)height / 2.0f)) * 20.0f / float(height);

        if (m_draggingPoint == true)
        {
            m_points[m_pointOnCursor].position = { x, y, 0.0f };
            return;
        }

        if (m_pointOnCursor != -1)
            m_points[m_pointOnCursor].color = { 1.0f, 0.0f, 0.0f };
        for (int i = 0;i < size;i++)
        {
            if (abs(m_points[i].position.x - x) < 0.1f && abs(m_points[i].position.y - y) < 0.1f)
            {
                m_points[i].color = { 1.0f, 1.0f, 0.0f };
                m_pointOnCursor = i;
                return;
            }
        }
        m_pointOnCursor = -1;
    }

    void Homework4::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            ClearPoint();
        }
        if (key == GLFW_KEY_DELETE && action == GLFW_PRESS)
        {
            if (m_points.size() != 0)
            {
                m_points.pop_back();
                m_indices.pop_back();
            }
        }
    }

    void Homework4::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            double xpos, ypos;
            int width, height;
            glfwGetCursorPos(window, &xpos, &ypos);
            glfwGetWindowSize(window, &width, &height);
            float x = (xpos - ((float)width / 2.0f)) * 20.0f / float(width);
            float y = -(ypos - ((float)height / 2.0f)) * 20.0f / float(height);

            if(m_pointOnCursor == -1)
                AddPoint({ x, y, 0.0f }, { 1.0f, 0.0f, 0.0f });
            else
            {
                //进入拖动点的状态
                m_draggingPoint = true;
            }
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            m_draggingPoint = false;
        }
    }

    void Homework4::AddPoint(glm::vec3 position, glm::vec3 color)
    {
        m_points.push_back({ position, color });
        m_indices.push_back(m_indices.size());
        m_functionShouldUpdate = true;
    }

    void Homework4::ClearPoint()
    {
        m_points.clear();
        m_indices.clear();
        m_functionShouldUpdate = true;
    }

    void Homework4::SortPoint()
    {
        std::sort(m_points.begin(), m_points.end(), [](VertexBuffer::point a, VertexBuffer::point b) {return a.position.x > b.position.x;});
    }

    void Homework4::PreDrawFunction(glm::vec3 color)
    {
        if (m_functionShouldUpdate == false)
            return;
        m_FunctionPoints.clear();

        float step = 0.1f;
        int size = m_points.size();
        for (int i = 0;i < size - 1;i++)//分别绘制size-1段函数
        {
            float x1 = m_points[i].position.x;
            float x2 = m_points[i+1].position.x;
            if (x1 < x2)
            {
                float x = x1;
                m_FunctionPoints.push_back({ {x1, fittingFunction(i,x1) ,0.0f}, color });
                while ((x = x + step) < x2)
                {
                    m_FunctionPoints.push_back({ {x, fittingFunction(i,x) ,0.0f}, color });
                }
                //终点x2不用画，避免与下一条方程的第一个点重合
            }
            else
            {
                float x = x1;
                m_FunctionPoints.push_back({ {x1, fittingFunction(i,x1) ,0.0f}, {color} });
                while ((x = x - step) > x2)
                {
                    m_FunctionPoints.push_back({ {x, fittingFunction(i,x) ,0.0f}, {color} });
                }
                //终点x2不用画，避免与下一条方程的第一个点重合
            }
        }
        //补回最后一个点
        m_FunctionPoints.push_back({ {m_points[size - 1].position.x, fittingFunction(size - 2, m_points[size - 1].position.x) ,0.0f}, color});
        m_functionShouldUpdate = true;//bug here
    }

    void Homework4::PreDrawCurve(glm::vec3 color)
    {
        if (m_functionShouldUpdate == false)
            return;
        m_FunctionPoints.clear();

        int size = m_points.size();
        float step = 0.1f;
        
        for (int i = 0;i < size - 1;i++)//分别绘制size-1段函数
        {
            float t1 = m_parameter[i];
            float t2 = m_parameter[i + 1];
            float t = t1;
            while (t < t2)
            {
                m_FunctionPoints.push_back({ fittingCurve(i, t), color });
                t = t + step;
            }
            //终点不用画，避免与下一条方程的第一个点重合
        }
        //补回最后一个点
        m_FunctionPoints.push_back({ fittingCurve(size - 2, m_parameter[size - 1]), color});
        m_functionShouldUpdate = true;//bug here
    }

    void Homework4::GenerateParameter()
    {
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        if (size < 2)
            return;

        if (m_parameterMethod == 0)
        {
            //均匀参数化
            m_parameter.clear();
            for (int i = 0;i < size;i++)
            {
                m_parameter.push_back((float)i);
            }
        }
        else if (m_parameterMethod == 1)
        {
            //弦长参数化
            m_parameter.clear();
            float total_length = 0.0f;
            for (int i = 0; i < size - 1; i++)
            {
                m_parameter.push_back(total_length);
                total_length += pow(pow(m_points[i].position.x - m_points[i + 1].position.x, 2) + pow(m_points[i].position.y - m_points[i + 1].position.y, 2), 0.5);
            }
            m_parameter.push_back(total_length);
        }
     }

    void Homework4::GetFittingFunction()
    {
        //三次样条函数，x只能递增，如果想画非函数曲线应该用GetFittingCurve()
        /*采用西工大 << 计算方法(第二版) >> p127 用节点处一阶导数表示的三次样条插值函数第二种边界条件的方法，其中为了方便代码编写，hi的定义与书上有点不同*/
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        if (size < 2)
            return;

        m_h.clear();
        m_h2.clear();
        m_h3.clear();
        m_m.clear();
        for (int i = 0;i < size - 1;i++)
        {
            m_h.push_back(m_points[i + 1].position.x - m_points[i].position.x);
            m_h2.push_back(pow(m_h[i], 2));
            m_h3.push_back(pow(m_h[i], 3));
        }

        /*
        Eigen::MatrixXd matrix_A(size, size);
        matrix_A = Eigen::MatrixXd::Zero(size, size);//eigen不会自动初始化元素为0
        Eigen::MatrixXd matrix_b(size, 1);
        Eigen::MatrixXd matrix_x(size, 1);
        matrix_A.coeffRef(0, 0) = 2;
        matrix_A.coeffRef(0, 1) = 1;
        matrix_b.coeffRef(0, 0) = 3 * (m_points[1].position.y - m_points[0].position.y) / m_h[0];
        for (int i = 1; i <= size - 2; i++)
        {
            matrix_A.coeffRef(i, i - 1) = m_h[i] / (m_h[i - 1] + m_h[i]);
            matrix_A.coeffRef(i, i) = 2;
            matrix_A.coeffRef(i, i + 1) = m_h[i - 1] / (m_h[i - 1] + m_h[i]);
            matrix_b.coeffRef(i, 0) = 3 * ((m_h[i - 1] / (m_h[i - 1] + m_h[i])) * ((m_points[i + 1].position.y - m_points[i].position.y) / m_h[i]) + (m_h[i] / (m_h[i - 1] + m_h[i])) * ((m_points[i].position.y - m_points[i - 1].position.y) / m_h[i - 1]));
        }
        matrix_A.coeffRef(size - 1, size - 2) = 1;
        matrix_A.coeffRef(size - 1, size - 1) = 2;
        matrix_b.coeffRef(size - 1, 0) = 3 * (m_points[size - 1].position.y - m_points[size - 2].position.y) / m_h[size - 2];
        
        std::cout << "matrix_A\n" << matrix_A << std::endl;
        //暂时用普通解法先测试，等会再改用追赶法
        matrix_x = matrix_A.colPivHouseholderQr().solve(matrix_b);
        m_m.insert(m_m.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());
        */

        //追赶法
        double* a = new double[size];//注意a[0]不使用，其实只有size-1
        double* b = new double[size];
        double* c = new double[size - 1];
        double* beta = new double[size];//注意beta[0]不使用，其实只有size - 1
        double* gamma = new double[size];
        double* delta = new double[size - 1];
        double* f = new double[size];
        double* y = new double[size];
        b[0] = 2;
        c[0] = 1;
        f[0] = 3 * (m_points[1].position.y - m_points[0].position.y) / m_h[0];
        for (int i = 1;i <= size - 2;i++)
        {
            a[i] = m_h[i] / (m_h[i - 1] + m_h[i]);
            b[i] = 2;
            c[i] = m_h[i - 1] / (m_h[i - 1] + m_h[i]);
            f[i] = 3 * ((m_h[i - 1] / (m_h[i - 1] + m_h[i])) * ((m_points[i + 1].position.y - m_points[i].position.y) / m_h[i]) + (m_h[i] / (m_h[i - 1] + m_h[i])) * ((m_points[i].position.y - m_points[i - 1].position.y) / m_h[i - 1]));
        }
        a[size - 1] = 1;
        b[size - 1] = 2;
        f[size - 1] = 3 * (m_points[size - 1].position.y - m_points[size - 2].position.y) / m_h[size - 2];

        //三角分解:Ax=f ---> (LU)x = f
        gamma[0] = b[0];
        delta[0] = c[0] / gamma[0];
        for (int i = 1;i <= size - 1;i++)
        {
            beta[i] = a[i];
            gamma[i] = b[i] - beta[i] * delta[i - 1];
            if (i != size - 1)
                delta[i] = c[i] / gamma[i];
        }
        //Ly = f
        y[0] = f[0] / gamma[0];
        for (int i = 1;i <= size - 1; i++)
        {
            y[i] = (f[i] - beta[i] * y[i - 1]) / gamma[i];
        }
        //Ux = y，这里m就是x
        m_m.resize(size);
        m_m[size - 1] = y[size - 1];
        for (int i = size - 2;i >= 0; i--)
        {
            m_m[i] = y[i] - delta[i] * m_m[i + 1];
        }

        delete[] a;
        delete[] b;
        delete[] c;
        delete[] beta;
        delete[] gamma;
        delete[] delta;
        delete[] f;
        delete[] y;

        /*
        std::vector<double>::iterator it;
        for (it = m_m.begin(); it != m_m.end(); ++it) {
             std::cout << *it << " ";
        }
        std::cout << std::endl;
        */
    }

    void Homework4::GetFittingCurve()
    {
        //三次样条曲线，和上面函数的区别是这个是参数曲线，可以画非函数的曲线
        if (m_functionShouldUpdate == false)
            return;
        int size = m_points.size();
        if (size < 2)
            return;

        m_h.clear();
        m_h2.clear();
        m_h3.clear();
        m_m.clear();
        for (int i = 0;i < size - 1;i++)
        {
            m_h.push_back(m_parameter[i + 1] - m_parameter[i]);
            m_h2.push_back(pow(m_h[i], 2));
            m_h3.push_back(pow(m_h[i], 3));
        }

        /*
        Eigen::MatrixXd matrix_A(size, size);
        matrix_A = Eigen::MatrixXd::Zero(size, size);//eigen不会自动初始化元素为0
        Eigen::MatrixXd matrix_b(size, 2);
        Eigen::MatrixXd matrix_x(size, 2);
        matrix_A.coeffRef(0, 0) = 2;
        matrix_A.coeffRef(0, 1) = 1;
        matrix_b.coeffRef(0, 0) = 3 * (m_points[1].position.x - m_points[0].position.x) / m_h[0];
        matrix_b.coeffRef(0, 1) = 3 * (m_points[1].position.y - m_points[0].position.y) / m_h[0];
        for (int i = 1; i <= size - 2; i++)
        {
            matrix_A.coeffRef(i, i - 1) = m_h[i] / (m_h[i - 1] + m_h[i]);
            matrix_A.coeffRef(i, i) = 2;
            matrix_A.coeffRef(i, i + 1) = m_h[i - 1] / (m_h[i - 1] + m_h[i]);
            matrix_b.coeffRef(i, 0) = 3 * ((m_h[i - 1] / (m_h[i - 1] + m_h[i])) * ((m_points[i + 1].position.x - m_points[i].position.x) / m_h[i]) + (m_h[i] / (m_h[i - 1] + m_h[i])) * ((m_points[i].position.x - m_points[i - 1].position.x) / m_h[i - 1]));
            matrix_b.coeffRef(i, 1) = 3 * ((m_h[i - 1] / (m_h[i - 1] + m_h[i])) * ((m_points[i + 1].position.y - m_points[i].position.y) / m_h[i]) + (m_h[i] / (m_h[i - 1] + m_h[i])) * ((m_points[i].position.y - m_points[i - 1].position.y) / m_h[i - 1]));
        }
        matrix_A.coeffRef(size - 1, size - 2) = 1;
        matrix_A.coeffRef(size - 1, size - 1) = 2;
        matrix_b.coeffRef(size - 1, 0) = 3 * (m_points[size - 1].position.x - m_points[size - 2].position.x) / m_h[size - 2];
        matrix_b.coeffRef(size - 1, 1) = 3 * (m_points[size - 1].position.y - m_points[size - 2].position.y) / m_h[size - 2];

        //std::cout << "matrix_A\n" << matrix_A << std::endl;
        //暂时用普通解法先测试，等会再改用追赶法
        matrix_x = matrix_A.colPivHouseholderQr().solve(matrix_b);
        m_m.insert(m_m.begin(), matrix_x.data(), matrix_x.data() + matrix_x.size());
        */


        //追赶法
        double* a = new double[size];//注意a[0]不使用，其实只有size-1
        double* b = new double[size];
        double* c = new double[size - 1];
        double* beta = new double[size];//注意beta[0]不使用，其实只有size - 1
        double* gamma = new double[size];
        double* delta = new double[size - 1];
        double* f1 = new double[size];
        double* f2 = new double[size];
        double* y1 = new double[size];
        double* y2 = new double[size];
        b[0] = 2;
        c[0] = 1;
        f1[0] = 3 * (m_points[1].position.x - m_points[0].position.x) / m_h[0];
        f2[0] = 3 * (m_points[1].position.y - m_points[0].position.y) / m_h[0];
        for (int i = 1;i <= size - 2;i++)
        {
            a[i] = m_h[i] / (m_h[i - 1] + m_h[i]);
            b[i] = 2;
            c[i] = m_h[i - 1] / (m_h[i - 1] + m_h[i]);
            f1[i] = 3 * ((m_h[i - 1] / (m_h[i - 1] + m_h[i])) * ((m_points[i + 1].position.x - m_points[i].position.x) / m_h[i]) + (m_h[i] / (m_h[i - 1] + m_h[i])) * ((m_points[i].position.x - m_points[i - 1].position.x) / m_h[i - 1]));
            f2[i] = 3 * ((m_h[i - 1] / (m_h[i - 1] + m_h[i])) * ((m_points[i + 1].position.y - m_points[i].position.y) / m_h[i]) + (m_h[i] / (m_h[i - 1] + m_h[i])) * ((m_points[i].position.y - m_points[i - 1].position.y) / m_h[i - 1]));
        }
        a[size - 1] = 1;
        b[size - 1] = 2;
        f1[size - 1] = 3 * (m_points[size - 1].position.x - m_points[size - 2].position.x) / m_h[size - 2];
        f2[size - 1] = 3 * (m_points[size - 1].position.y - m_points[size - 2].position.y) / m_h[size - 2];

        //三角分解:Ax=f ---> (LU)x = f
        gamma[0] = b[0];
        delta[0] = c[0] / gamma[0];
        for (int i = 1;i <= size - 1;i++)
        {
            beta[i] = a[i];
            gamma[i] = b[i] - beta[i] * delta[i - 1];
            if (i != size - 1)
                delta[i] = c[i] / gamma[i];
        }
        //Ly = f
        y1[0] = f1[0] / gamma[0];
        y2[0] = f2[0] / gamma[0];
        for (int i = 1;i <= size - 1; i++)
        {
            y1[i] = (f1[i] - beta[i] * y1[i - 1]) / gamma[i];
            y2[i] = (f2[i] - beta[i] * y2[i - 1]) / gamma[i];
        }
        //Ux = y，这里m就是x
        m_m.resize(2 * size);
        m_m[2 * (size - 1)] = y1[size - 1];
        m_m[2 * (size - 1) + 1] = y2[size - 1];
        for (int i = size - 2;i >= 0; i--)
        {
            m_m[2 * i] = y1[i] - delta[i] * m_m[2 * (i + 1)];
            m_m[2 * i + 1] = y2[i] - delta[i] * m_m[2 * (i + 1) + 1];
        }

        delete[] a;
        delete[] b;
        delete[] c;
        delete[] beta;
        delete[] gamma;
        delete[] delta;
        delete[] f1;
        delete[] y1;
        delete[] y2;
    }

    float Homework4::fittingFunction(int i, float x)
    {   
        // i 表示第几段函数，i=0,1,2,....,n-2
        float y1 = pow(x - m_points[i + 1].position.x, 2) * (m_h[i] + 2 * (x - m_points[i].position.x)) / m_h3[i] * m_points[i].position.y;
        float y2 = pow(x - m_points[i].position.x, 2) * (m_h[i] + 2 * (m_points[i + 1].position.x - x)) / m_h3[i] * m_points[i + 1].position.y;
        float y3 = pow(x - m_points[i + 1].position.x, 2) * (x - m_points[i].position.x) / m_h2[i] * m_m[i];
        float y4 = pow(x - m_points[i].position.x, 2) * (x - m_points[i + 1].position.x) / m_h2[i] * m_m[i + 1];

        if (std::isnan(y1) || std::isnan(y2) || std::isnan(y3) || std::isnan(y4))
            std::cout << "error:两点的x值过近,计算中出现了除0操作" << std::endl << "请按空格清除顶点" << std::endl;
        return y1 + y2 + y3 + y4;
    }

    glm::vec3 Homework4::fittingCurve(int i, float t)
    {
        // i 表示第几段函数，i=0,1,2,....,n-2
        float x1 = pow(t - m_parameter[i + 1], 2) * (m_h[i] + 2 * (t - m_parameter[i])) / m_h3[i] * m_points[i].position.x;
        float x2 = pow(t - m_parameter[i], 2) * (m_h[i] + 2 * (m_parameter[i + 1] - t)) / m_h3[i] * m_points[i + 1].position.x;
        float x3 = pow(t - m_parameter[i + 1], 2) * (t - m_parameter[i]) / m_h2[i] * m_m[2 * i];
        float x4 = pow(t - m_parameter[i], 2) * (t - m_parameter[i + 1]) / m_h2[i] * m_m[2 * (i + 1)];

        float y1 = pow(t - m_parameter[i + 1], 2) * (m_h[i] + 2 * (t - m_parameter[i])) / m_h3[i] * m_points[i].position.y;
        float y2 = pow(t - m_parameter[i], 2) * (m_h[i] + 2 * (m_parameter[i + 1] - t)) / m_h3[i] * m_points[i + 1].position.y;
        float y3 = pow(t - m_parameter[i + 1], 2) * (t - m_parameter[i]) / m_h2[i] * m_m[2 * i + 1];
        float y4 = pow(t - m_parameter[i], 2) * (t - m_parameter[i + 1]) / m_h2[i] * m_m[2 * (i + 1) + 1];

        if (std::isnan(x1) || std::isnan(x2) || std::isnan(x3) || std::isnan(x4) || std::isnan(y1) || std::isnan(y2) || std::isnan(y3) || std::isnan(y4))
            std::cout << "error:两点过近,计算中出现了除0操作" << std::endl << "请按空格清除顶点" << std::endl;
        return { (x1 + x2 + x3 + x4), (y1 + y2 + y3 + y4), 0.0f };
    }

}