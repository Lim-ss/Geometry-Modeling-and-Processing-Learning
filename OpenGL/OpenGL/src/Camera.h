#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "GLFW/glfw3.h"

enum CameraStyle
{
	fpv = 0,
	cad = 1,
};

class Camera {
public:
	Camera(glm::mat4& View);
	~Camera();

	void BindViewMatrix(glm::mat4& View);
	void CameraUpdate(float deltaTime);
	static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	static CameraStyle cameraStyle;
	static float yaw;
	static float pitch;
	static bool CursorDisabled;
	static glm::vec3 m_cameraPos;

	float fov;
private:

	static glm::vec3 m_cameraFront;
	static glm::vec3 m_cameraUp;
	glm::mat4& m_View;

	static bool m_dragging;
	static bool m_IntoDragging;
};