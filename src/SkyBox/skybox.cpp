#include "skybox.h"

Skybox::Skybox(std::array<std::string, 6> images, int texSlot) {
    m_images = images;
    m_texSlot = texSlot;
    this->createData();
    this->createTex();
}

Skybox::~Skybox() {
    this->destroy();
}

void Skybox::createData() {
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);


}

void Skybox::createTex() {
    glGenTextures(1, &m_tex);
    glActiveTexture(GL_TEXTURE0 + m_texSlot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

    // TODO: load in the damn images fuuuuuuck


    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // TODO: create shader program :(

}

void Skybox::destroy() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteProgram(m_shader);
    glDeleteTextures(1, &m_tex);
}
