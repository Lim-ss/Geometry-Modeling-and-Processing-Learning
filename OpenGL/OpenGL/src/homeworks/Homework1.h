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

	class Homework1 : public Module
	{
	public:
		Homework1();
		~Homework1();

		void OnUpdate(double deltaTime) override;
		void OnRender() override;
		void OnImguiRender() override;
		void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) override;
		void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;
		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) override;

		void AddPoint(glm::vec3 position, glm::vec3 color);
		void ClearPoint();
		void SortPoint();
		void PreDrawFunction(std::function<float(float)> func, float increments, glm::vec3 color);

		void GetInterpolationPolynomial();
		float interpolationPolynomial(float);
		void GetInterpolationGauss();
		float interpolationGauss(float);
		void GetApproximationPolynomial();
		float approximationPolynomial(float);
		void GetRidgeRegressionPolynomial();
		float ridgeRegressionPolynomial(float);
		

		
	private:
		std::vector<double> m_polynomialCoefficient;//a0,a1,a2...a(n-1),attention: use double to match Eigen::MatrixXd
		std::vector<double> m_gaussCoefficient;//b0,b1,b2...b(n)
		std::vector<double> m_approximationPolynomialCoefficient;//a0,a1,a2...a(m)
		std::vector<double> m_ridgeRegressionPolynomialCoefficient;//a0,a1,a2...a(m)

		std::vector<VertexBuffer::point> m_points;
		std::vector<unsigned int> m_indices;
		std::vector<VertexBuffer::point> m_FunctionPoints;

		bool m_functionShouldUpdate;
		bool m_showfunction[4];

		float m_sigma;
		int m_highestPower;
		float m_lambda;//岭回归正则项系数

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