#pragma once

#include "Module.h"

#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "Camera.h"

#include "GLFW/glfw3.h"

#include "imgui/imgui.h"

#include "Eigen/Core"
#include "Eigen/Geometry"

#include <memory>

namespace module {

	class Homework5 : public Module
	{
	public:
		Homework5();
		~Homework5();

		void OnUpdate(double deltaTime) override;
		void OnRender() override;
		void OnImguiRender() override;
		void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) override;
		void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;
		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) override;

		void AddPoint(glm::vec3 position, glm::vec3 color);
		void ClearPoint();
		void Link();
		void UnLink();

		void Chaiukin2();

	private:
		std::vector<VertexBuffer::point> m_pointsA;
		std::vector<VertexBuffer::point> m_pointsB;

		glm::mat4 m_Proj;
		glm::mat4 m_View;
		glm::mat4 m_Model;
		glm::mat4 m_MVP;

		float m_pointSize;
		float m_lineWidth;

		bool m_link;
		enum CurrentBuffer
		{
			A = 0,
			B = 1
		}m_currentBuffer;

		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<Shader> m_Shader;

		ImGuiIO& m_IO;
	};
}