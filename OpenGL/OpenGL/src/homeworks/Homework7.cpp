#include "Homework7.h"

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

    Homework7::Homework7()
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
        m_Shader = std::make_unique<Shader>("res/shaders/Homework6.shader");
        m_Shader->Bind();
        m_Camera = std::make_unique<Camera>(m_View);

        ShowCurvatureWithColor();
    }

    Homework7::~Homework7()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void Homework7::OnUpdate(double deltaTime)
    {
        m_Camera->CameraUpdate(deltaTime);
        m_Model = glm::scale(glm::mat4(1.0f), glm::vec3(pow(10.0f, m_scale)));//����ģ�ʹ�С
    }

    void Homework7::OnRender()
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
        m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Mesh->m_Indices.size());
    }

    void Homework7::OnImguiRender()
    {

        ImGui::Text("Press ESC to disable the cursor");

        ImGui::Checkbox("Wireframe Mode", &m_WireframeMode);

        ImGui::SliderFloat("model scale", &m_scale, -2.0f, 2.0f);

        if (ImGui::Button("Update Mesh uniform"))
        {
            MinimalSurfaceGrobalMethod();
            ShowCurvatureWithColor();
        }
        if (ImGui::Button("Update Mesh cot"))
        {
            MinimalSurfaceGrobalMethodCot();
            ShowCurvatureWithColor();
        }
        if (ImGui::Button("Parameterization"))
        {
            parameterization();
            ShowCurvatureWithColor();
        }

        if (ImGui::Button("load model 1"))
        {
            m_Mesh->Reload("res/mesh/Nefertiti_face.obj");
            ShowCurvatureWithColor();
        }
        if (ImGui::Button("load model 2"))
        {
            m_Mesh->Reload("res/mesh/Balls.obj");
            ShowCurvatureWithColor();
        }
        if (ImGui::Button("load model 3"))
        {
            m_Mesh->Reload("res/mesh/Bunny_head.obj");
            ShowCurvatureWithColor();
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void Homework7::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Camera::CursorPosCallback(window, xpos, ypos);
    }

    void Homework7::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Camera::KeyCallback(window, key, scancode, action, mods);
    }

    void Homework7::MinimalSurfaceGrobalMethod()
    {
        int size = m_Mesh->m_Vertices.size();
        Eigen::SparseMatrix<double> matrixA(size, size);
        Eigen::SparseMatrix<double> matrixb(size, 3);
        for (int i = 0;i < size;i++)
        {
            if (m_Mesh->IsBoundaryVertex(i))
            {
                matrixA.coeffRef(i, i) = 1.0f;
                matrixb.coeffRef(i, 0) = m_Mesh->m_Vertices[i].position.x;
                matrixb.coeffRef(i, 1) = m_Mesh->m_Vertices[i].position.y;
                matrixb.coeffRef(i, 2) = m_Mesh->m_Vertices[i].position.z;
                continue;
            }

            std::vector<int> neighbourVertexIndex;//��vj��index
            glm::vec3 vi = m_Mesh->m_Vertices[i].position;//Ŀ�궥������
            int firstEdgeIndex = m_Mesh->m_Vertices[i].edgeIndex;//���ڱȽ��Ƿ�������һȦ
            int EdgeIndex = firstEdgeIndex;//����

            neighbourVertexIndex.push_back(m_Mesh->m_Edges[EdgeIndex].vertexIndex);
            EdgeIndex = m_Mesh->m_Edges[m_Mesh->m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;//�л�����һ����
            while (EdgeIndex != firstEdgeIndex)
            {
                neighbourVertexIndex.push_back(m_Mesh->m_Edges[EdgeIndex].vertexIndex);
                EdgeIndex = m_Mesh->m_Edges[m_Mesh->m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;
            }

            matrixA.coeffRef(i, i) = neighbourVertexIndex.size();
            for (int j = 0;j < neighbourVertexIndex.size();j++)
            {
                matrixA.coeffRef(i, neighbourVertexIndex[j]) = -1.0;
            }
        }

        Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
        solver.analyzePattern(matrixA);
        solver.factorize(matrixA);
        if (solver.info() != Eigen::Success)
        {
            std::cout << "Sparse LU decomposition failed." << std::endl;
            return;
        }
        Eigen::MatrixXd matrixx = solver.solve(matrixb);

        for (int i = 0;i < size;i++)
        {
            //���¶���λ��
            m_Mesh->m_Vertices[i].position.x = matrixx.coeff(i, 0);
            m_Mesh->m_Vertices[i].position.y = matrixx.coeff(i, 1);
            m_Mesh->m_Vertices[i].position.z = matrixx.coeff(i, 2);
        }
    }

    void Homework7::MinimalSurfaceGrobalMethodCot()
    {
        int size = m_Mesh->m_Vertices.size();
        Eigen::SparseMatrix<double> matrixA(size, size);
        Eigen::SparseMatrix<double> matrixb(size, 3);
        for (int i = 0;i < size;i++)
        {
            if (m_Mesh->IsBoundaryVertex(i))
            {
                matrixA.coeffRef(i, i) = 1.0f;
                matrixb.coeffRef(i, 0) = m_Mesh->m_Vertices[i].position.x;
                matrixb.coeffRef(i, 1) = m_Mesh->m_Vertices[i].position.y;
                matrixb.coeffRef(i, 2) = m_Mesh->m_Vertices[i].position.z;
                continue;
            }

            std::vector<glm::vec3> vector;//��vi->vj������
            std::vector<int> neighbourVertexIndex;//��vj��index
            glm::vec3 vi = m_Mesh->m_Vertices[i].position;//Ŀ�궥������
            int firstEdgeIndex = m_Mesh->m_Vertices[i].edgeIndex;//���ڱȽ��Ƿ�������һȦ
            int EdgeIndex = firstEdgeIndex;//����

            vector.push_back(m_Mesh->m_Vertices[m_Mesh->m_Edges[EdgeIndex].vertexIndex].position - vi);
            neighbourVertexIndex.push_back(m_Mesh->m_Edges[EdgeIndex].vertexIndex);
            EdgeIndex = m_Mesh->m_Edges[m_Mesh->m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;//�л�����һ����
            while (EdgeIndex != firstEdgeIndex)
            {
                vector.push_back(m_Mesh->m_Vertices[m_Mesh->m_Edges[EdgeIndex].vertexIndex].position - vi);
                neighbourVertexIndex.push_back(m_Mesh->m_Edges[EdgeIndex].vertexIndex);
                EdgeIndex = m_Mesh->m_Edges[m_Mesh->m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;
            }

            std::vector<float> angle1;//�洢ÿ���߶�Ӧ����ǰһ�������ڵĽ�(������)
            std::vector<float> angle2;//�洢ÿ���߶�Ӧ�����һ�������ڵĽ�(������)

            int n = vector.size();
            glm::vec3 t1 = vector[n - 1] - vector[0];
            glm::vec3 t2 = vector[1] - vector[0];
            angle1.push_back(std::acos(glm::dot(vector[0], t1) / (glm::length(vector[0]) * glm::length(t1))));
            angle2.push_back(std::acos(glm::dot(vector[0], t2) / (glm::length(vector[0]) * glm::length(t2))));
            for (int j = 1;j <= n - 2;j++)
            {
                t1 = vector[j - 1] - vector[j];
                t2 = vector[j + 1] - vector[j];
                angle1.push_back(std::acos(glm::dot(vector[j], t1) / (glm::length(vector[j]) * glm::length(t1))));
                angle2.push_back(std::acos(glm::dot(vector[j], t2) / (glm::length(vector[j]) * glm::length(t2))));
            }
            t1 = vector[n - 2] - vector[n - 1];
            t2 = vector[0] - vector[n - 1];
            angle1.push_back(std::acos(glm::dot(vector[n - 1], t1) / (glm::length(vector[n - 1]) * glm::length(t1))));
            angle2.push_back(std::acos(glm::dot(vector[n - 1], t2) / (glm::length(vector[n - 1]) * glm::length(t2))));

            for (int j = 0;j < n;j++)
            {
                matrixA.coeffRef(i, i) += (1.0f / tan(angle1[j])) + (1.0f / tan(angle2[j]));
                matrixA.coeffRef(i, neighbourVertexIndex[j]) -= (1.0f / tan(angle1[j])) + (1.0f / tan(angle2[j]));
            }
        }

        Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
        solver.analyzePattern(matrixA);
        solver.factorize(matrixA);
        if (solver.info() != Eigen::Success)
        {
            std::cout << "Sparse LU decomposition failed." << std::endl;
            return;
        }
        Eigen::MatrixXd matrixx = solver.solve(matrixb);
        
        for (int i = 0;i < size;i++)
        {
            //���¶���λ��
            m_Mesh->m_Vertices[i].position.x = matrixx.coeff(i, 0);
            m_Mesh->m_Vertices[i].position.y = matrixx.coeff(i, 1);
            m_Mesh->m_Vertices[i].position.z = matrixx.coeff(i, 2);
        }
    }

    void Homework7::parameterization()//ʹ�þ���Ȩ
    {
        int size = m_Mesh->m_Vertices.size();
        Eigen::SparseMatrix<double> matrixA(size, size);
        Eigen::SparseMatrix<double> matrixb(size, 3);

        int starting = -1;
        for (int i = 0;i < size;i++)
        {
            if(m_Mesh->IsBoundaryVertex(i))
            {
                starting = i;//����ҵ�һ���߽��ϵĵ�
            }
        }
        if (starting == -1)
        {
            printf("find no boundary, parameterization failed\n");
            return;
        }

        std::vector<float> offset;//ÿ���߽�㵽���ľ���
        std::vector<float> index;//ÿ���߽���index
        float totalLength = 0;
        offset.push_back(0.0f);
        index.push_back(starting);
        int current = starting;
        int next = m_Mesh->m_Edges[m_Mesh->m_Vertices[current].edgeIndex].vertexIndex;
        while(next != starting)//����߽�ĳ��Ⱥ�ÿ���߽�㵽���ľ���
        {
            float length = glm::length(m_Mesh->m_Vertices[current].position - m_Mesh->m_Vertices[next].position);
            offset.push_back(totalLength + length);
            index.push_back(next);
            totalLength += length;
            current = next;
            next = m_Mesh->m_Edges[m_Mesh->m_Vertices[current].edgeIndex].vertexIndex;
        }
        totalLength += glm::length(m_Mesh->m_Vertices[current].position - m_Mesh->m_Vertices[next].position);
        for (int i = 0;i < offset.size();i++)
        {

            //���߽���޶���[0,1]*[0,1]�����α߽��ϣ����½�Ϊ��㣬��ʱ�벼��
            float ratio = offset[i] / totalLength;
            if (ratio < 0.25f)//���±�
            {
                matrixb.coeffRef(index[i], 0) = ratio * 4;
                matrixb.coeffRef(index[i], 1) = 0;
                matrixb.coeffRef(index[i], 2) = 0;
            }
            else if (ratio < 0.5f)//���ұ�
            {
                matrixb.coeffRef(index[i], 0) = 1.0f;
                matrixb.coeffRef(index[i], 1) = (ratio - 0.25f) * 4;
                matrixb.coeffRef(index[i], 2) = 0;
            }
            else if (ratio < 0.75f)//���ϱ�
            {
                matrixb.coeffRef(index[i], 0) = 1.0f - (ratio - 0.5f) * 4;
                matrixb.coeffRef(index[i], 1) = 1.0f;
                matrixb.coeffRef(index[i], 2) = 0;
            }
            else//�����
            {
                matrixb.coeffRef(index[i], 0) = 0;
                matrixb.coeffRef(index[i], 1) = 1.0f - (ratio - 0.75f) * 4;
                matrixb.coeffRef(index[i], 2) = 0;
            }

            matrixA.coeffRef(index[i], index[i]) = 1.0f;
        }

        for (int i = 0;i < size;i++)
        {
            if (m_Mesh->IsBoundaryVertex(i))
            {
                continue;
            }

            std::vector<int> neighbourVertexIndex;//��vj��index
            glm::vec3 vi = m_Mesh->m_Vertices[i].position;//Ŀ�궥������
            int firstEdgeIndex = m_Mesh->m_Vertices[i].edgeIndex;//���ڱȽ��Ƿ�������һȦ
            int EdgeIndex = firstEdgeIndex;//����

            neighbourVertexIndex.push_back(m_Mesh->m_Edges[EdgeIndex].vertexIndex);
            EdgeIndex = m_Mesh->m_Edges[m_Mesh->m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;//�л�����һ����
            while (EdgeIndex != firstEdgeIndex)
            {
                neighbourVertexIndex.push_back(m_Mesh->m_Edges[EdgeIndex].vertexIndex);
                EdgeIndex = m_Mesh->m_Edges[m_Mesh->m_Edges[EdgeIndex].oppositeEdgeIndex].nextEdgeIndex;
            }

            matrixA.coeffRef(i, i) = neighbourVertexIndex.size();
            for (int j = 0;j < neighbourVertexIndex.size();j++)
            {
                matrixA.coeffRef(i, neighbourVertexIndex[j]) = -1.0;
            }
        }

        Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
        solver.analyzePattern(matrixA);
        solver.factorize(matrixA);
        if (solver.info() != Eigen::Success)
        {
            std::cout << "Sparse LU decomposition failed." << std::endl;
            return;
        }
        Eigen::MatrixXd matrixx = solver.solve(matrixb);

        for (int i = 0;i < size;i++)
        {
            //���¶���λ��
            m_Mesh->m_Vertices[i].position.x = matrixx.coeff(i, 0);
            m_Mesh->m_Vertices[i].position.y = matrixx.coeff(i, 1);
            m_Mesh->m_Vertices[i].position.z = matrixx.coeff(i, 2);
        }
    }

    void Homework7::ShowCurvatureWithColor()
    {
        for (int i = 0;i < m_Mesh->m_Vertices.size();i++)
        {
            float t = pow(10.0f, m_scale) * glm::length(m_Mesh->Laplace_Operator(i));//ƽ�����ʵ�ģ������Ȩ
            //float t = pow(10.0f, m_scale) * glm::length(m_Mesh->Laplace_Beltrami_Operator(i));//ƽ�����ʵ�ģ��cotȨ
            glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
            float ratio = (t / 0.01f > 1.0f ? 1.0f : t / 0.01f);
            m_Mesh->m_Vertices[i].color = ratio * red + (1.0f - ratio) * white;
        }
    }
}