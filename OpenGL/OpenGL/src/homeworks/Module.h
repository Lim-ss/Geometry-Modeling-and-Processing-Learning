#pragma once

#include <vector>
#include <string>
#include <functional>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

namespace module {

	class Module
	{
	public:
		Module() {}
		virtual ~Module() {}

		virtual void OnUpdate(double deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImguiRender() {}
		virtual void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {}
		virtual void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {}
		virtual void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}
		virtual void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {}
	};

	class ModuleMenu : public Module
	{
	public:
		ModuleMenu(Module*& currentModulePointer);

		void OnRender() override;
		void OnImguiRender() override;

		template<typename T>
		void RegisterModule(const std::string& name)
		{
			std::cout << "Registering Module " << name << std::endl;
			m_Modules.push_back(std::make_pair(name, []() {return new T();}));
		}
	private:
		Module*& m_CurrentModule;
		std::vector<std::pair<std::string, std::function<Module* ()>>> m_Modules;
	};
}//namespace module