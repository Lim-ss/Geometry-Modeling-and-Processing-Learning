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
        m_WireframeMode(false)
    {
        
        //用Assimp加载obj
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile("res/mesh/Nefertiti_face.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
        if (scene)
        {
            std::cout << "num of meshes:" << scene->mNumMeshes << std::endl;
            m_Mesh = scene->mMeshes[0];
            std::cout << "num of vertices:" << m_Mesh->mNumVertices << std::endl;

            for (unsigned int i = 0; i < m_Mesh->mNumVertices; i++) {
                aiVector3D vertex = m_Mesh->mVertices[i];
                printf("%f,%f,%f\n", vertex.x, vertex.y, vertex.z);
            }

            for (unsigned int i = 0; i < m_Mesh->mNumFaces; i++) {
                aiFace face = m_Mesh->mFaces[i];
                if (face.mNumIndices == 3)//其实可以不判断，因为加载时用了aiProcess_Triangulate参数
                {
                    m_Indices.push_back(face.mIndices[0]);
                    m_Indices.push_back(face.mIndices[1]);
                    m_Indices.push_back(face.mIndices[2]);
                    printf("%u,%u,%u\n", face.mIndices[0], face.mIndices[1], face.mIndices[2]);
                }
            }
        }
        //importer.FreeScene();
        
        /*
        if (m_ObjLoader.LoadFile("res/mesh/yuegui.obj") == false)
        {
            std::cout << "obj loader failed" << std::endl;
        }
        else
        {
            std::cout << m_ObjLoader.LoadedMeshes.size() << " mesh(s) be loaded" << std::endl;
            m_Mesh = m_ObjLoader.LoadedMeshes[0];
            m_Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));//调整模型大小
        }
        */

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        m_VAO = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_Mesh->mVertices, sizeof(aiVector3D) * m_Mesh->mNumVertices);
        VertexBufferLayout layout;
        layout.Push<float>(3);//position
        //layout.Push<float>(3);//normal
        //layout.Push<float>(2);//texCoord
        m_VAO->AddBuffer(*m_VBO, layout);
        m_IBO = std::make_unique<IndexBuffer>(m_Indices.data(), m_Indices.size());//ATTENTION! argument.2 is count,not size
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
        renderer.DrawTriangle(*m_VAO, *m_IBO, *m_Shader, m_Indices.size());
    }

    void Homework6::OnImguiRender()
    {

        ImGui::Text("Press ESC to disable the cursor");

        ImGui::Checkbox("Wireframe Mode", &m_WireframeMode);

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
}