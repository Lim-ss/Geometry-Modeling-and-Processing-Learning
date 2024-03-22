#include "OpenSubdivTest.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Eigen/Core"
#include "Eigen/Geometry"
#include "Eigen/Dense"

#include "assimp/scene.h"

#include "opensubdiv/far/topologyDescriptor.h"
#include "opensubdiv/far/primvarRefiner.h"

namespace module {

    OpenSubdivTest::OpenSubdivTest()
        :
        m_Proj(glm::mat4(1.0f)),
        m_View(glm::mat4(1.0f)),
        m_Model(glm::mat4(1.0f)),
        m_MVP(glm::mat4(1.0f)),
        m_IO(ImGui::GetIO()),
        m_scale(0.0f)
    {
        m_Mesh = std::make_unique<HE3::Mesh>("res/mesh/Nefertiti_face.obj");

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        m_VAO1 = std::make_unique<VertexArray>();
        m_VAO2 = std::make_unique<VertexArray>();
        m_VBO = std::make_unique<VertexBuffer>(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
        VertexBufferLayout layout;
        layout.Vacate(sizeof(HE3::Vertex::edgeIndex));//edge
        layout.Push<float>(3);//position
        layout.Push<float>(3);//color
        layout.Push<float>(3);//normal
        layout.Push<float>(2);//texCoord
        m_VAO1->AddBuffer(*m_VBO, layout);
        m_VAO2->AddBuffer(*m_VBO, layout);
        m_IBO1 = std::make_unique<IndexBuffer>(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
        m_IBO2 = std::make_unique<IndexBuffer>(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());
        m_Shader = std::make_unique<Shader>("res/shaders/MeshSubdiv.shader");
        m_Shader->Bind();
        m_Camera = std::make_unique<Camera>(m_View);

        glEnable(GL_POLYGON_OFFSET_FILL); // 启用填充面的多边形偏移
        glPolygonOffset(0.5f, 0.1f); // 设置多边形偏移因子和单位

    }

    OpenSubdivTest::~OpenSubdivTest()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    void OpenSubdivTest::OnUpdate(double deltaTime)
    {
        m_Camera->CameraUpdate(deltaTime);
        m_Model = glm::scale(glm::mat4(1.0f), glm::vec3(pow(10.0f, m_scale)));//调整模型大小
    }

    void OpenSubdivTest::OnRender()
    {
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

        /* Render here */
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        Renderer renderer;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*m_Mesh->UpdateIndices();
        m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
        m_IBO1->ReData(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
        m_IBO2->ReData(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());*/

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_Shader->SetUniform1i("u_Mode", 3);//纯色
        renderer.DrawTriangle(*m_VAO1, *m_IBO1, *m_Shader, m_Mesh->m_Indices1.size());
        m_Shader->SetUniform1i("u_Mode", 2);//黑色
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        renderer.DrawLine(*m_VAO2, *m_IBO2, *m_Shader, m_Mesh->m_Indices2.size());


    }

    void OpenSubdivTest::OnImguiRender()
    {
        if (ImGui::Button("catmullClark()"))
        {
            catmullClark();
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO1->ReData(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
            m_IBO2->ReData(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());
        }
        ImGui::InputText("index", m_input1, IM_ARRAYSIZE(m_input1));
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
        if (ImGui::Button("load model 1"))
        {
            m_Mesh->Load("res/mesh/Nefertiti_face.obj");
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO1->ReData(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
            m_IBO2->ReData(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());
        }
        if (ImGui::Button("load model 2"))
        {
            m_Mesh->Load("res/mesh/Balls.obj");
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO1->ReData(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
            m_IBO2->ReData(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());
        }
        if (ImGui::Button("load model 3"))
        {
            m_Mesh->Load("res/mesh/polygonTest.obj");
            m_VBO->ReData(m_Mesh->m_Vertices.data(), sizeof(HE3::Vertex) * m_Mesh->m_Vertices.size());
            m_IBO1->ReData(m_Mesh->m_Indices1.data(), m_Mesh->m_Indices1.size());
            m_IBO2->ReData(m_Mesh->m_Indices2.data(), m_Mesh->m_Indices2.size());
        }
        if (ImGui::Button("print"))
        {
            printf("face:%d,edges:%d,vertex:%d\n", m_Mesh->m_Faces.size(), m_Mesh->m_Edges.size(), m_Mesh->m_Vertices.size());
        }
        if (ImGui::Button("printIndices1"))
        {
            m_Mesh->PrintIndices1();
        }
        if (ImGui::Button("printEdges"))
        {
            m_Mesh->PrintHalfEdges();
        }
        if (ImGui::Button("printFaces"))
        {
            m_Mesh->PrintFaces();
        }
        if (ImGui::Button("printVertices"))
        {
            m_Mesh->PrintVertices();
        }
        ImGui::SliderFloat("model scale", &m_scale, -2.0f, 2.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_IO.Framerate, m_IO.Framerate);

    }

    void OpenSubdivTest::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Camera::CursorPosCallback(window, xpos, ypos);
    }

    void OpenSubdivTest::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Camera::KeyCallback(window, key, scancode, action, mods);
    }

    void OpenSubdivTest::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Camera::MouseButtonCallback(window, button, action, mods);
    }

    void OpenSubdivTest::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Camera::ScrollCallback(window, xoffset, yoffset);
    }

    void OpenSubdivTest::catmullClark()
    {

        // Populate a topology descriptor with our raw data

        typedef OpenSubdiv::Far::TopologyDescriptor Descriptor;

        OpenSubdiv::Sdc::SchemeType type = OpenSubdiv::Sdc::SCHEME_CATMARK;

        OpenSubdiv::Sdc::Options options;
        options.SetVtxBoundaryInterpolation(OpenSubdiv::Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);

        Descriptor desc;
        desc.numVertices = m_Mesh->m_Vertices.size();
        desc.numFaces = m_Mesh->m_Faces.size();
        desc.numVertsPerFace = m_Mesh->m_Vertsperface.data();
        desc.vertIndicesPerFace = m_Mesh->m_Indices3.data();

        // Instantiate a Far::TopologyRefiner from the descriptor

        OpenSubdiv::Far::TopologyRefiner* refiner = OpenSubdiv::Far::TopologyRefinerFactory<Descriptor>::Create(desc,
            OpenSubdiv::Far::TopologyRefinerFactory<Descriptor>::Options(type, options));

        int maxlevel = 1;

        // Uniformly refine the topology up to 'maxlevel'
        refiner->RefineUniform(OpenSubdiv::Far::TopologyRefiner::UniformOptions(maxlevel));

        // Allocate a buffer for vertex primvar data. The buffer length is set to
        // be the sum of all children vertices up to the highest level of refinement.
        std::vector<Vertex> vbuffer(refiner->GetNumVerticesTotal());
        Vertex* verts = &vbuffer[0];

        // Initialize coarse mesh positions
        int nCoarseVerts = m_Mesh->m_Vertices.size();
        for (int i = 0; i < nCoarseVerts; ++i) {
            verts[i].SetPosition(m_Mesh->m_Vertices[i].position.x, m_Mesh->m_Vertices[i].position.y, m_Mesh->m_Vertices[i].position.z);
        }

        // Interpolate vertex primvar data
        OpenSubdiv::Far::PrimvarRefiner primvarRefiner(*refiner);

        Vertex* src = verts;
        for (int level = 1; level <= maxlevel; ++level) {
            Vertex* dst = src + refiner->GetLevel(level - 1).GetNumVertices();
            primvarRefiner.Interpolate(level, src, dst);
            src = dst;
        }

        { 
            // Output the highest level refined -----------

            OpenSubdiv::Far::TopologyLevel const& refLastLevel = refiner->GetLevel(maxlevel);

            int nverts = refLastLevel.GetNumVertices();
            int nfaces = refLastLevel.GetNumFaces();

            // Print vertex positions
            int firstOfLastVerts = refiner->GetNumVerticesTotal() - nverts;

            aiVector3D* Vertices = new aiVector3D[nverts];
            aiFace* Faces = new aiFace[nfaces];
            aiMesh mesh;
            mesh.mNumVertices = nverts;
            mesh.mNumFaces = nfaces;
            mesh.mVertices = Vertices;
            mesh.mFaces = Faces;

            for (int vert = 0; vert < nverts; ++vert) 
            {
                float const* pos = verts[firstOfLastVerts + vert].GetPosition();
                Vertices[vert] = aiVector3D((ai_real)pos[0], (ai_real)pos[1], (ai_real)pos[2]);
            }

            for (int face = 0; face < nfaces; ++face) 
            {
                OpenSubdiv::Far::ConstIndexArray fverts = refLastLevel.GetFaceVertices(face);

                // all refined Catmark faces should be quads
                assert(fverts.size() == 4);

                aiFace _face;
                _face.mNumIndices = fverts.size();
                _face.mIndices = new unsigned int[fverts.size()];
                for (int vert = 0; vert < fverts.size(); ++vert)
                {
                    _face.mIndices[vert] = fverts[vert];
                }
                Faces[face] = _face;
            }

            m_Mesh->Load(&mesh);
        }

        delete refiner;
    }
}