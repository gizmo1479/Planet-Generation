#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <array>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>


class Skybox
{
public:
    Skybox(std::array<std::string, 6> images, int texSlot);
    ~Skybox();
    void paint();

private:
    void createTex();
    void createData();
    void destroy();
    std::array<std::string, 6> m_images;
    GLuint m_vao, m_vbo, m_shader, m_tex;
    int m_texSlot;
};

#endif // SKYBOX_H
