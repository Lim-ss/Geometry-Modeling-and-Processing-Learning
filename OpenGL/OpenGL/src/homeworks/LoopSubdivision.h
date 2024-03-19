#pragma once

#include "Module.h"

#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "Camera.h"
#include "HalfEdge.h"

#include "GLFW/glfw3.h"

#include "imgui/imgui.h"

#include <memory>
#include <unordered_set>

#include "obj-loader/OBJ-Loader.h"

namespace module {

	class LoopSubdivision : public Module
	{
	public:
		LoopSubdivision();
		~LoopSubdivision();

		void OnUpdate(double deltaTime) override;
		void OnRender() override;
		void OnImguiRender() override;
		void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) override;
		void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;
		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) override;
		void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) override;

		void LoopSubdiv();
	private:

		glm::mat4 m_Proj;
		glm::mat4 m_View;
		glm::mat4 m_Model;
		glm::mat4 m_MVP;

		//use smart pointer to keep the API consistent with before
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Camera> m_Camera;
		std::unique_ptr<HE::Mesh> m_Mesh;

		float m_scale;

		ImGuiIO& m_IO;
	};
}