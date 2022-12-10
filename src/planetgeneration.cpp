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

    // If you must use this function, do not edit anything above this
    rebuildCameraMatrices(this->width(), this->height());
}

void PlanetGeneration::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteVertexArrays(1, &m_sphere_vao);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // TODO: do outline stuff
    glGenBuffers(1, &m_outline_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_outline_vbo);
    auto sphereScale = m_sphere.generateShapeScale(1.05); // TODO: change if needed
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*sphereScale.size(), sphereScale.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_outline_vao);
    glBindVertexArray(m_outline_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
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

    // Students: anything requiring OpenGL calls when the program starts should be done here
    glClearColor(.9, .9, .9, 1);

    // Shader setup
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_outline_shader =
            ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/outline.frag");

    // Initialise sphere data and VBO/VAO
    m_sphere = Sphere();
    m_sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    initSphere();

    initialised = true;
}

void PlanetGeneration::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    if (initialised) {
        if (settings.outlines) {
            paintOutline();
            return;
        }

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0x00);

        glUseProgram(m_shader);
        glBindVertexArray(m_sphere_vao);
        sendUniforms();

        glDrawArrays(GL_TRIANGLES, 0, m_sphere.generateShape().size() / 3);

        glBindVertexArray(0);
        glUseProgram(0);
    }
}

void PlanetGeneration::paintOutline() {
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(m_shader);
    glBindVertexArray(m_sphere_vao);
    sendUniforms();

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glDrawArrays(GL_TRIANGLES, 0, m_sphere.generateShape().size() / 3);
    glBindVertexArray(0);
    glUseProgram(0);

    // do outline
    glUseProgram(m_outline_shader);
    glBindVertexArray(m_outline_vao);
    sendUniforms();

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST); // TODO: is this necessary?
    glDrawArrays(GL_TRIANGLES, 0, m_sphere.generateShape().size() / 3);
    glBindVertexArray(0);
    glUseProgram(0);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glEnable(GL_DEPTH_TEST);
}

void PlanetGeneration::rebuildCameraMatrices(int w, int h)
{
  // Update view matrix by rotating eye vector based on x and y angles

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

  update();
}


void PlanetGeneration::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_proj = glm::perspective(glm::radians(45.0), 1.0 * w / h, 0.01, 100.0);
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


void PlanetGeneration::sendUniforms() {

    GLint modelMatrix = glGetUniformLocation(m_shader, "modelMatrix");
    glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, &m_model[0][0]);

    GLint viewMatrix = glGetUniformLocation(m_shader, "viewMatrix");
    glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, &m_view[0][0]);

    GLint projectionMatrix = glGetUniformLocation(m_shader, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, &m_proj[0][0]);

    auto camPosLoc = glGetUniformLocation(m_shader, "cameraPos");
    glUniform3fv(camPosLoc, 1, &m_eye[0]);

    GLint shader = glGetUniformLocation(m_shader, "shader");
    glUniform1i(shader, settings.shader);
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

