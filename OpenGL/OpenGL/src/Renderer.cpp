#include "Renderer.h"
#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "):" << function <<
            " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

void Renderer::Clear() const
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::DrawTriangle(const VertexArray& va, const IndexBuffer& ib, const Shader& shader,int count) const
{
    shader.Bind();//step.1
    va.Bind();//step.2
    ib.Bind();//step.3
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);//draw with index(indices)

}

void Renderer::DrawPoint(const VertexArray& va, const IndexBuffer& ib, const Shader& shader, int count) const
{
    shader.Bind();//step.1
    va.Bind();//step.2
    ib.Bind();//step.3
    glDrawElements(GL_POINTS, count, GL_UNSIGNED_INT, nullptr);//draw with index(indices)

}
