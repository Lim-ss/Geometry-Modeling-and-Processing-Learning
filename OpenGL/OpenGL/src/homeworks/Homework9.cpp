#include "Homework9.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Eigen/Core"
#include "Eigen/Geometry"
#include "Eigen/Dense"


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
        m_Mesh = std::make_unique<HE2::Mesh>("res/mesh/Nefertiti_face.obj");

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        m_VAO = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_Mesh->m_Vertices.data(), sizeof(HE2::Vertex) * m_Mesh->m_Vertices.size());
        VertexBufferLayout layout;
        layout.Vacate(sizeof(HE2::Vertex::edgeIndex));//edge
        layout.Push<float>(3);//position
        layout.Push<float>(3);//color
        layout.Push<float>(3);//normal
        layout.Push<float>(2);//texCoord
        layout.Vacate(sizeof(Eigen::Matrix4f));//QuadricErrorMetrix;
        m_VAO->AddBuffer(*m_VBO, layout);
        m_IBO = std::make_unique<IndexBuffer>(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());
        m_Shader = std::make_unique<Shader>("res/shaders/Homework9.shader");
        m_Shader->Bind();
        m_Camera = std::make_unique<Camera>(m_View);

        glEnable(GL_POLYGON_OFFSET_FILL); // 启用填充面的多边形偏移
        glPolygonOffset(0.5f, 0.1f); // 设置多边形偏移因子和单位

        InitQEM();
    }

    Homework9::~Homework9()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_POLYGON_OFFSET_FILL);
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
        if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            //窗口没有最小化
            glfwGetWindowSize(window, &width, &height);
            m_Proj = glm::perspective(glm::radians(m_Camera->fov), (float)width / (float)height, 0.1f, 1000.0f);

        }
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
        //m_Mesh->UpdateNormals();
        m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE2::Vertex) * m_Mesh->m_Vertices.size());
        m_IBO->ReData(m_Mesh->m_Indices.data(), m_Mesh->m_Indices.size());

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_Shader->SetUniform1i("u_Mode", 3);
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Mesh->m_Indices.size());
        m_Shader->SetUniform1i("u_Mode", 2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Mesh->m_Indices.size());
    }

    void Homework9::OnImguiRender()
    {

        ImGui::Text("Press ESC to disable the cursor");

        ImGui::Checkbox("Wireframe Mode", &m_WireframeMode);

        ImGui::SliderFloat("model scale", &m_scale, -2.0f, 2.0f);

        ImGui::InputText("index", m_input1, IM_ARRAYSIZE(m_input1));
        // 在ImGui中创建按钮，用于确定坐标
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
        if (ImGui::Button("delete face"))
        {
            int index = atoi(m_input1);
            if (index < m_Mesh->m_Faces.size())
            {
                printf("delete face:%d\n", index);
                m_Mesh->DeleteFace(index);
            }
        }
        if (ImGui::Button("delete face"))
        {
            int index = atoi(m_input1);
            if (index < m_Mesh->m_Faces.size())
            {
                printf("delete face:%d\n", index);
                m_Mesh->DeleteFace(index);
            }
        }
        if (ImGui::Button("edge contract"))
        {
            int index = atoi(m_input1);
            if (index < m_Mesh->m_Edges.size())
            {
                if (m_Mesh->EdgeContract(index) == 1)
                {
                    printf("edge contract:%d\n", index);
                }
                else
                {
                    printf("failed to EdgeContract\n");
                }
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
        if (ImGui::Button("print QEM"))
        {
            m_Mesh->PrintQEM();
        }
        if (ImGui::Button("print error"))
        {
            m_Mesh->PrintQuadricError();
        }
        if (ImGui::Button("calculate error"))
        {
            printf("calculate error\n");
            CalculateQuadricError();
        }
        if (ImGui::Button("mesh simplify"))
        {
            int t = atoi(m_input2);
            printf("mesh simplifying ... \nwait ...\n");
            int schedule = 0;
            std::cout << "\033[32m"; // 设置文本颜色为绿色
            printf("[>         ]0%%\n");//0%-10%
            for (int i = 0;i < t;i++)
            {
                CalculateQuadricError();
                MeshSimplify();

                //进度条
                if ((float)i / t > (schedule + 1) * 0.1f)
                {
                    std::cout << "\033[F";//删除上一行控制台输出
                    printf("[");
                    for (int j = 0;j < schedule + 1;j++)
                    {
                        printf("=");
                    }
                    printf(">");
                    for (int j = 0;j < 8 - schedule;j++)
                    {
                        printf(" ");
                    }
                    printf("]%d0%%\n", schedule + 1);
                    schedule++;
                }
            }
            std::cout << "\033[F";//删除上一行控制台输出
            printf("[==========]100%%\n");//100%
            std::cout << "\033[0m"; // 重置颜色属性
            printf("finish:%dtimes\n", t);
        }
        ImGui::SameLine();
        ImGui::InputText("simplify times", m_input2, IM_ARRAYSIZE(m_input2));

        

        /*if (ImGui::Button("erase face"))
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
        }*/

        if (ImGui::Button("load model 1"))
        {
            m_Mesh->Reload("res/mesh/Nefertiti_face.obj");
            InitQEM();
        }
        if (ImGui::Button("load model 2"))
        {
            m_Mesh->Reload("res/mesh/Bunny_head.obj");
            InitQEM();
        }
        if (ImGui::Button("load model 3"))
        {
            m_Mesh->Reload("res/mesh/Balls.obj");
            InitQEM();
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

    void Homework9::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Camera::MouseButtonCallback(window, button, action, mods);
    }

    void Homework9::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Camera::ScrollCallback(window, xoffset, yoffset);
    }

    void Homework9::InitQEM()
    {
        for (int i = 0;i < m_Mesh->m_Vertices.size();i++)
        {
            int start = m_Mesh->m_Vertices[i].edgeIndex;
            int t = m_Mesh->m_Edges[m_Mesh->m_Edges[start].oppositeEdgeIndex].nextEdgeIndex;
            glm::vec3 a_b_c = m_Mesh->NormalOfFace(m_Mesh->m_Edges[start].faceIndex);
            float a = a_b_c.x;
            float b = a_b_c.y;
            float c = a_b_c.z;
            float d = -(m_Mesh->m_Vertices[i].position.x * a + m_Mesh->m_Vertices[i].position.y * b + m_Mesh->m_Vertices[i].position.z * c);
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.setZero();

            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 0) += a * a;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 1) += a * b;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 2) += a * c;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 3) += a * d;

            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 0) += a * b;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 1) += b * b;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 2) += b * c;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 3) += b * d;

            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 0) += a * c;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 1) += b * c;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 2) += c * c;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 3) += c * d;

            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 0) += a * d;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 1) += b * d;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 2) += c * d;
            m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 3) += d * d;
            while (t != start)
            {
                glm::vec3 a_b_c = m_Mesh->NormalOfFace(m_Mesh->m_Edges[t].faceIndex);
                float a = a_b_c.x;
                float b = a_b_c.y;
                float c = a_b_c.z;
                float d = -(m_Mesh->m_Vertices[i].position.x * a + m_Mesh->m_Vertices[i].position.y * b + m_Mesh->m_Vertices[i].position.z * c);
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 0) += a * a;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 1) += a * b;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 2) += a * c;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(0, 3) += a * d;

                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 0) += a * b;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 1) += b * b;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 2) += b * c;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(1, 3) += b * d;

                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 0) += a * c;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 1) += b * c;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 2) += c * c;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(2, 3) += c * d;

                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 0) += a * d;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 1) += b * d;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 2) += c * d;
                m_Mesh->m_Vertices[i].QuadricErrorMetrix.coeffRef(3, 3) += d * d;

                t = m_Mesh->m_Edges[m_Mesh->m_Edges[t].oppositeEdgeIndex].nextEdgeIndex;
            }
        }
    }

    void Homework9::CalculateQuadricError()
    {
        for (int i = 0;i < m_Mesh->m_Edges.size();i++)
        {
            //v1->v2
            int v1 = m_Mesh->m_Edges[m_Mesh->m_Edges[i].oppositeEdgeIndex].vertexIndex;
            int v2 = m_Mesh->m_Edges[i].vertexIndex;
            Eigen::Matrix4f& Q1 = m_Mesh->m_Vertices[v1].QuadricErrorMetrix;
            Eigen::Matrix4f& Q2 = m_Mesh->m_Vertices[v2].QuadricErrorMetrix;
            Eigen::Matrix4f Q = Q1 + Q2;
            Eigen::Matrix4f Q_ = Q;
            Q_.coeffRef(3, 0) = 0.0f;
            Q_.coeffRef(3, 1) = 0.0f;
            Q_.coeffRef(3, 2) = 0.0f;
            Q_.coeffRef(3, 3) = 1.0f;
            if (Q_.determinant() != 0.0f)
            {
                //可逆
                Eigen::Vector4f v = Q_.inverse() * Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
                m_Mesh->m_Edges[i].QuadricError = v.transpose() * Q * v;
                m_Mesh->m_Edges[i].bestPosition = glm::vec3(v.x(), v.y(), v.z());
                //std::cout << Q << std::endl;
                //printf("e:%d:  yes   error:%f\n", i, m_Mesh->m_Edges[i].QuadricError);
            }
            else
            {
                glm::vec3 v_pos = 0.5f * m_Mesh->m_Vertices[v1].position + 0.5f * m_Mesh->m_Vertices[v2].position;
                Eigen::Vector4f v = Eigen::Vector4f(v_pos.x, v_pos.y, v_pos.z, 1.0f);
                m_Mesh->m_Edges[i].QuadricError = v.transpose() * Q * v;
                m_Mesh->m_Edges[i].bestPosition = v_pos;
                //printf("e:%d:  no    error:%f\n", i, m_Mesh->m_Edges[i].QuadricError);
            }
        }
    }

    void Homework9::MeshSimplify()
    {
        int minIndex = -1;
        float minValue = 10000;
        for (int i = 0;i < m_Mesh->m_Edges.size();i++)
        {
            if (m_Mesh->m_Edges[i].QuadricError < minValue && m_Mesh->EdgeContractable(i))
            {
                minIndex = i;
                minValue = m_Mesh->m_Edges[i].QuadricError;
            }
        }
        //minIndex为error最小的可塌缩边
        if (minIndex == -1)
        {
            printf("unable to simplify\n");
            return;
        }
        int v1 = m_Mesh->m_Edges[m_Mesh->m_Edges[minIndex].oppositeEdgeIndex].vertexIndex;
        int v2 = m_Mesh->m_Edges[minIndex].vertexIndex;

        //printf("v%d: %f,%f,%f\n",v1, m_Mesh->m_Vertices[v1].position.x, m_Mesh->m_Vertices[v1].position.y, m_Mesh->m_Vertices[v1].position.z);
        //printf("v%d: %f,%f,%f\n",v2, m_Mesh->m_Vertices[v2].position.x, m_Mesh->m_Vertices[v2].position.y, m_Mesh->m_Vertices[v2].position.z);

        m_Mesh->m_Vertices[v1].position = m_Mesh->m_Edges[minIndex].bestPosition;
        m_Mesh->m_Vertices[v2].position = m_Mesh->m_Edges[minIndex].bestPosition;

        //printf("best: %f,%f,%f\n\n", m_Mesh->m_Vertices[v1].position.x, m_Mesh->m_Vertices[v1].position.y, m_Mesh->m_Vertices[v1].position.z);

        Eigen::Matrix4f& Q1 = m_Mesh->m_Vertices[v1].QuadricErrorMetrix;
        Eigen::Matrix4f& Q2 = m_Mesh->m_Vertices[v2].QuadricErrorMetrix;
        Eigen::Matrix4f Q = Q1 + Q2;
        m_Mesh->m_Vertices[v1].QuadricErrorMetrix = Q;
        m_Mesh->m_Vertices[v2].QuadricErrorMetrix = Q;
        m_Mesh->EdgeContract(minIndex);
    }
}