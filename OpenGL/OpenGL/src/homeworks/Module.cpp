#include "Module.h"
#include "imgui/imgui.h"

namespace module {

	ModuleMenu::ModuleMenu(Module*& currentModulePointer)
		:m_CurrentModule(currentModulePointer)
	{

	}

	void ModuleMenu::OnImguiRender()
	{
		for (auto& module : m_Modules)
		{
			if (ImGui::Button(module.first.c_str()))
				m_CurrentModule = module.second();
		}
	}

	void ModuleMenu::OnRender()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}