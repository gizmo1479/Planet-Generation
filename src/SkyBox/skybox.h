#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <array>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <QOpenGLWidget>

class Skybox
{
public:
    Skybox() {};
    Skybox(std::array<std::string, 6> images, int texSlot, glm::mat4 view,
           glm::mat4 proj);
    ~Skybox();
    void paint();
    void update(glm::mat4 view, glm::mat4 proj);
    GLuint m_skybox_shader;
private:
    void createTex();
    void createData();
    void destroy();
    std::vector<float> getVerts();

    std::array<std::string, 6> m_images;
    GLuint m_vao, m_vbo, m_tex;
    int m_texSlot;

    glm::mat4 view, proj;
};

#endif // SKYBOX_H
