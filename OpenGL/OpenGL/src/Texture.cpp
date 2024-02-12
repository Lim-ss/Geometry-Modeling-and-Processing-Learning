#include "Texture.h"

#include "stb_image/stb_image.h"

#include <iostream>

Texture::Texture(const std::string& path)
	: m_RendererID(0), m_FilePath(path), m_LocationBuffer(nullptr),
	m_Width(0), m_Heigth(0), m_Channels(0)
{
	stbi_set_flip_vertically_on_load(1);
	m_LocationBuffer = stbi_load(path.c_str(), &m_Width, &m_Heigth, &m_Channels, 4);//4-byte alignment.As much as possible use 4 even if only 3 channels.Or you should use glPixelStorei(GL_UNPACK_ALIGNMENT, 1)

	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	
	if (m_LocationBuffer == nullptr)
		std::cout << "load texture failed" << std::endl;
	else
		std::cout << "load texture successful    channel:" << m_Channels << " width:" << m_Width << " height:" << m_Heigth << std::endl;
	
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//if is not 4-byte alignment and use GL_RGB

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Heigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocationBuffer));
	
	if (m_LocationBuffer)
		stbi_image_free(m_LocationBuffer);
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int slot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}