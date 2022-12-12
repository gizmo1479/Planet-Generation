#include "skybox.h"
#include "utils/shaderloader.h"

#include <QImage>

Skybox::Skybox(std::array<std::string, 6> images, int texSlot, glm::mat4 view,
               glm::mat4 proj) {
    m_images = images;
    m_texSlot = texSlot;
    this->createData();
    this->createTex();
    this->view = glm::mat4(glm::mat3(view));
    this->proj = proj;
}

Skybox::~Skybox() {
    this->destroy();
}

void Skybox::update(glm::mat4 view, glm::mat4 proj) {
    this->proj = proj;
    this->view = glm::mat4(glm::mat3(view));
}

void Skybox::paint() {
    glDepthFunc(GL_LEQUAL); // change depth func so depth test passes when vals are == to depth buffs contents
    glUseProgram(m_skybox_shader);

    // send projection, view mat, and sampler
//    std::cout << "passing skybox uniforms\n";
    GLuint tLoc = glGetUniformLocation(m_skybox_shader, "skybox");
//    std::cout << "tLoc: " << tLoc << "\n";
    glUniform1i(tLoc, m_texSlot);
    GLint viewLoc = glGetUniformLocation(m_skybox_shader, "view");
//    std::cout << "viewLoc: " << viewLoc << "\n";
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    GLint projLoc = glGetUniformLocation(m_skybox_shader, "proj");
//    std::cout << "projLoc: " << projLoc << "\n";
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);

    // do skybox cube
    glBindVertexArray(m_vao);
    glActiveTexture(GL_TEXTURE0 + m_texSlot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glDepthFunc(GL_LESS); // set back to default
}

void Skybox::createData() {
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    auto v = getVerts();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v.size(), v.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Skybox::createTex() {
    glGenTextures(1, &m_tex);
    glActiveTexture(GL_TEXTURE0 + m_texSlot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

    for (int i = 0; i < 6; i++) {
        QString f = QString::fromStdString(m_images[i]);
        QImage m = QImage(f);
        m = m.convertToFormat(QImage::Format_RGBA8888).mirrored();

       glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGBA, m.width(), m.height(), 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, m.bits());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Skybox::destroy() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
//    glDeleteProgram(m_skybox_shader);
    glDeleteTextures(1, &m_tex);
}

std::vector<float> Skybox::getVerts() {
    std::vector<float> v = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    return v;
}
