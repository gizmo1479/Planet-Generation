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
    void paint(GLuint shader);
    void update(glm::mat4 view, glm::mat4 proj);

    GLuint vao, vbo, m_shader, tex;
    int texSlot;

    glm::mat4 view, proj;
private:
    void createTex();
    void createData();
    void destroy();
    std::vector<float> getVerts();

    std::array<std::string, 6> m_images;
};

#endif // SKYBOX_H
