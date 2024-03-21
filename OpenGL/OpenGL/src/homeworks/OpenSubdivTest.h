#pragma once

#include "Module.h"

#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "Camera.h"
#include "HalfEdge3.h"

#include "GLFW/glfw3.h"

#include "imgui/imgui.h"

#include <memory>
#include <unordered_set>

#include "obj-loader/OBJ-Loader.h"

//opensubdiv需要的接口
static struct Vertex {

	// Minimal required interface ----------------------
	Vertex() {
		_position[0] = 0;
		_position[1] = 0;
		_position[2] = 0;
	}

	Vertex(Vertex const& src) {
		_position[0] = src._position[0];
		_position[1] = src._position[1];
		_position[2] = src._position[2];
	}

	void Clear(void* = 0) {
		_position[0] = _position[1] = _position[2] = 0.0f;
	}

	void AddWithWeight(Vertex const& src, float weight) {
		_position[0] += weight * src._position[0];
		_position[1] += weight * src._position[1];
		_position[2] += weight * src._position[2];
	}

	// Public interface ------------------------------------
	void SetPosition(float x, float y, float z) {
		_position[0] = x;
		_position[1] = y;
		_position[2] = z;
	}

	const float* GetPosition() const {
		return _position;
	}

private:
	float _position[3];
};

namespace module {

	class OpenSubdivTest : public Module
	{
	public:
		OpenSubdivTest();
		~OpenSubdivTest();

		void OnUpdate(double deltaTime) override;
		void OnRender() override;
		void OnImguiRender() override;
		void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) override;
		void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;
		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) override;
		void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) override;

		void catmullClark();
	private:

		glm::mat4 m_Proj;
		glm::mat4 m_View;
		glm::mat4 m_Model;
		glm::mat4 m_MVP;

		//use smart pointer to keep the API consistent with before
		std::unique_ptr<VertexArray> m_VAO1;//面
		std::unique_ptr<VertexArray> m_VAO2;//线框
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO1;
		std::unique_ptr<IndexBuffer> m_IBO2;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Camera> m_Camera;
		std::unique_ptr<HE3::Mesh> m_Mesh;

		float m_scale;
		char m_input1[10] = "";

		ImGuiIO& m_IO;
	};
}