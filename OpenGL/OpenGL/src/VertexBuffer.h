#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class VertexBuffer
{
private:
	unsigned int m_RendererID;
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	void ReData(const void* data, unsigned int size);

	struct vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 textureCoordinate;
		float textureID;

		inline vertex(glm::vec3 position, glm::vec3 color, glm::vec2 textureCoordinate, float textureID)
			: position(position), color(color), textureCoordinate(textureCoordinate), textureID(textureID)
		{}
		inline vertex()
			: position(0), color(0), textureCoordinate(0), textureID(0)
		{}
	};

	struct point
	{
		glm::vec3 position;
		glm::vec3 color;
	};
};