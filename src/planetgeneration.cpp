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

void PlanetGeneration::setSphereVBO() {
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_sphere.generateShape().size(), m_sphere.generateShape().data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlanetGeneration::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteVertexArrays(1, &m_sphere_vao);

    this->doneCurrent();
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
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    glClearColor(0, 0, 0, 1);

    // Shader setup
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");

    // Initialise sphere data and VBO/VAO
    m_sphere = Sphere();
    m_sphere.updateParams(20, 20);

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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    initialised = true;
}

void PlanetGeneration::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    if (initialised) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(m_sphere_vao);

        glUseProgram(m_shader);

        GLint modelMatrix = glGetUniformLocation(m_shader, "modelMatrix");
        glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, &m_model[0][0]);

        GLint viewMatrix = glGetUniformLocation(m_shader, "viewMatrix");
        glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, &m_view[0][0]);

        GLint projectionMatrix = glGetUniformLocation(m_shader, "projectionMatrix");
        glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, &m_proj[0][0]);

        auto camPosLoc = glGetUniformLocation(m_shader, "cameraPos");
        glUniform3fv(camPosLoc, 1, &m_eye[0]);

        glDrawArrays(GL_TRIANGLES, 0, m_sphere.generateShape().size() / 3);
        glBindVertexArray(0);
        glUseProgram(0);
    }
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

    update(); // asks for a PaintGL() call to occur
}

void PlanetGeneration::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void PlanetGeneration::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void PlanetGeneration::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void PlanetGeneration::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void PlanetGeneration::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

void PlanetGeneration::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    update(); // asks for a PaintGL() call to occur
}
