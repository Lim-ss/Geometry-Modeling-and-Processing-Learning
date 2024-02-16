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

	class Homework4 : public Module
	{
	public:
		Homework4();
		~Homework4();

		void OnUpdate(double deltaTime) override;
		void OnRender() override;
		void OnImguiRender() override;
		void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) override;
		void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;
		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) override;

		enum ParameterMethod
		{
			uniform = 0,
			chord_length = 1,
			centripetal = 2
		};

		void AddPoint(glm::vec3 position, glm::vec3 color);
		void ClearPoint();
		void SortPoint();

		void PreDrawFunction(glm::vec3 color);
		void PreDrawCurve(glm::vec3 color);

		void GenerateParameter();
		void GetFittingFunction();
		void GetFittingCurve();
		float fittingFunction(int i, float x);
		glm::vec3 fittingCurve(int i, float t);



	private:
		std::vector<double> m_m;//各点的一阶导
		std::vector<double> m_h;//h(i) = x(i + 1) - x(i)
		std::vector<double> m_h2;//h(i)^2
		std::vector<double> m_h3;//h(i)^3

		std::vector<VertexBuffer::point> m_points;
		std::vector<unsigned int> m_indices;
		std::vector<VertexBuffer::point> m_FunctionPoints;
		std::vector<float> m_parameter;

		bool m_functionShouldUpdate;
		
		char m_inputX[32] = "";
		char m_inputY[32] = "";

		glm::mat4 m_Proj;
		glm::mat4 m_View;
		glm::mat4 m_Model;
		glm::mat4 m_MVP;

		float m_pointSize;
		float m_lineWidth;
		//use smart pointer to keep the API consistent with before
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Texture> m_Texture1;
		std::unique_ptr<Texture> m_Texture2;
		std::unique_ptr<Camera> m_Camera;

		ImGuiIO& m_IO;
	};
}