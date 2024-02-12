#pragma once

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

struct ShaderProgramSource
{
	std::string VectexSource;
	std::string FragmentSource;
};

class Shader
{
private:
	unsigned int m_RendererID;
	std::string m_Filepath;
	std::unordered_map<std::string, unsigned int> m_UniformLocationCache;

public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	//set uniforms
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1iv(const std::string& name, int size, int* array);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

private:
	int GetUniformLocation(const std::string& name);
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& framentShader);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	
};