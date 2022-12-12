#include "planetgeneration.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "settings.h"
#include <utils/shaderloader.h>

PlanetGeneration::PlanetGeneration(QWidget *parent)
    : QOpenGLWidget(parent),
      m_angleX(6),
      m_angleY(0),
      m_zoom(2)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    rebuildCameraMatrices(this->width(), this->height());
}

void PlanetGeneration::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    delSphere();

    glDeleteTextures(1, &m_terrain_texture);

    glDeleteProgram(m_shader);
    glDeleteProgram(m_outline_shader);

    this->doneCurrent();
}

void PlanetGeneration::initSphere() {
    // initialise sphere vbo and vao
    glGenBuffers(1, &m_sphere_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    std::vector<GLfloat> sphere_data = m_sphere.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sphere_data.size(), sphere_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_sphere_vao);
    glBindVertexArray(m_sphere_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // initialise outline stuff
    glGenBuffers(1, &m_outline_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_outline_vbo);
    auto sphereScale = m_sphere.generateShapeScale(1.05); // TODO: change if needed
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*sphereScale.size(), sphereScale.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_outline_vao);
    glBindVertexArray(m_outline_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlanetGeneration::setSphereVBO() {
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    auto v = m_sphere.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*v.size(), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_outline_vbo);
    auto v2 = m_sphere.generateShapeScale(1.05);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*v2.size(), v2.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlanetGeneration::delSphere() {
    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteBuffers(1, &m_outline_vbo);
    glDeleteVertexArrays(1, &m_sphere_vao);
    glDeleteVertexArrays(1, &m_outline_vao);
}

void PlanetGeneration::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL to enable stencil testing
    glEnable(GL_STENCIL_TEST);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    glClearColor(.9, .9, .9, 1);

    // Shader setup
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_outline_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/outline.frag");


    // Initialise sphere data and VBO/VAO
    m_sphere = Sphere();
    m_sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    initSphere();

    // Initialise terrain default data and texture
    std::vector<GLfloat> heights = m_terrain.generateTerrain();
    int res = m_terrain.getResolution();

    /** Creds to chatgpt for help with the following code :D **/
    // Generate a texture object and bind it to texture slot 0
    glGenTextures(1, &m_terrain_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_terrain_texture);

    // Set the texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the displacement map texture data to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, res, res, 0, GL_RED, GL_UNSIGNED_BYTE, heights.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(m_shader);
    GLuint texture = glGetUniformLocation(m_shader, "height_map");
    glUniform1i(texture, 0);
    glUseProgram(0);

    // initialise camera matrices
    rebuildCameraMatrices(width(), height());

    // make skybox
    auto back = "/home/gizmo1479/Pictures/skybox2/back.jpg";
    auto top = "/home/gizmo1479/Pictures/skybox2/top.jpg";
    auto left = "/home/gizmo1479/Pictures/skybox2/left.jpg";
    auto right = "/home/gizmo1479/Pictures/skybox2/right.jpg";
    auto bottom = "/home/gizmo1479/Pictures/skybox2/bottom.jpg";
    auto front = "/home/gizmo1479/Pictures/skybox2/front.jpg";
    std::array<std::string, 6> images = {left, left, left, left, left, left};
    m_skybox = Skybox(images, 2, m_view, m_proj); // TODO: update tex slot if needed
    m_skybox_shader = ShaderLoader::createShaderProgram(":resources/shaders/skybox.vert",
                                                        ":resources/shaders/skybox.frag");
    initialised = true;
}

void PlanetGeneration::paintGL() {
    if (initialised) {
        if (settings.outlines) {
            paintOutline();
            return;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(m_shader);
        glBindVertexArray(m_sphere_vao);
        sendUniforms(&m_shader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_terrain_texture);

        glDrawArrays(GL_TRIANGLES, 0, m_sphere.generateShape().size() / 3);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        // now draw the skybox where the sphere isnt
        m_skybox.update(m_view, m_proj);
        m_skybox.paint(m_skybox_shader);

        //paintSkybox();
    }
}


void PlanetGeneration::paintOutline() {
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(m_shader);
    glBindVertexArray(m_sphere_vao);
    sendUniforms(&m_shader);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_terrain_texture);
    glDrawArrays(GL_TRIANGLES, 0, m_sphere.generateShape().size() / 3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // do outline
    glUseProgram(m_outline_shader);
    glBindVertexArray(m_outline_vao);
    sendUniforms(&m_outline_shader);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST); // TODO: is this necessary?
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_terrain_texture);
    glDrawArrays(GL_TRIANGLES, 0, m_sphere.generateShape().size() / 3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glEnable(GL_DEPTH_TEST);
}

void PlanetGeneration::rebuildCameraMatrices(int w, int h)
{
  // Create a new view matrix
  m_view = glm::mat4(1);
  glm::mat4 rot = glm::rotate(glm::radians(-10 * m_angleX), glm::vec3(0,0,1));
  glm::vec3 eye = glm::vec3(2,0,0);
  eye = glm::vec3(rot * glm::vec4(eye, 1));
  rot = glm::rotate(glm::radians(-10 * m_angleY), glm::cross(glm::vec3(0,0,1),eye));
  eye = glm::vec3(rot * glm::vec4(eye, 1));

  eye = eye * m_zoom;
  m_eye = eye;
  m_view = glm::lookAt(eye,glm::vec3(0,0,0),glm::vec3(0,0,1));
  m_proj = glm::perspective(glm::radians(45.0), 1.0 * w / h, 0.01,100.0);
  m_MVP = m_proj * m_view * m_model;

  update();
}

void PlanetGeneration::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    m_proj = glm::perspective(glm::radians(45.0), 1.0 * w / h, 0.01, 100.0);
    m_MVP = m_proj * m_view * m_model;
}

void PlanetGeneration::sceneChanged() {
    update(); // asks for a PaintGL() call to occur
}

void PlanetGeneration::settingsChanged() {
    if (initialised) {
        m_sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
        setSphereVBO();
    }

    update(); // asks for a PaintGL() call to occur
}

void PlanetGeneration::sendUniforms(GLuint *shader) {
    GLint modelMatrix = glGetUniformLocation(*shader, "modelMatrix");
    glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, &m_model[0][0]);

    GLint MVPMatrix = glGetUniformLocation(*shader, "MVPMatrix");
    glUniformMatrix4fv(MVPMatrix, 1, GL_FALSE, &m_MVP[0][0]);

    auto camPosLoc = glGetUniformLocation(*shader, "cameraPos");
    glUniform3fv(camPosLoc, 1, &m_eye[0]);

    GLint shaderType = glGetUniformLocation(*shader, "shaderType");
    glUniform1i(shaderType, settings.shaderType);
}

/******** EVENT HANDLING **********/

void PlanetGeneration::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void PlanetGeneration::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void PlanetGeneration::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void PlanetGeneration::mousePressEvent(QMouseEvent *event) {
    m_prevMousePos = event->pos();
    m_mouseDown = true;
}

void PlanetGeneration::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        m_angleX += 10 * (event->position().x() - m_prevMousePos.x()) / (float) width();
        m_angleY += 10 * (event->position().y() - m_prevMousePos.y()) / (float) height();
        m_prevMousePos = event->pos();
        rebuildCameraMatrices(width(), height());
    }
}

void PlanetGeneration::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    update(); // asks for a PaintGL() call to occur
}

void PlanetGeneration::wheelEvent(QWheelEvent *event) {
    m_zoom -= event->angleDelta().y() / 100.f;
    rebuildCameraMatrices(width(), height());
}

void PlanetGeneration::updateView() {
    m_view =
            glm::translate(glm::vec3(0.0, 0.0, -4.0 + m_zoom)) *
            glm::rotate(m_angleX, glm::vec3(0.0, 1.0, 0.0)) *
            glm::rotate(m_angleY, glm::vec3(1.0, 0.0, 0.0));
    update();
}

