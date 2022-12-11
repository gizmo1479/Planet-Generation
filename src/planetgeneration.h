#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "sphere.h"
#include "terraingenerator.h"
#include "SkyBox/skybox.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

class PlanetGeneration : public QOpenGLWidget
{
public:
    PlanetGeneration(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void updateView();

    glm::mat4 m_model   = glm::mat4(1);
    glm::mat4 m_view    = glm::mat4(1);
    glm::mat4 m_proj    = glm::mat4(1);
    glm::mat4 m_MVP     = glm::mat4(1);
    glm::vec3 m_eye;

    GLuint m_shader;        // Stores id of shader program
    bool initialised = false;

    // terrain generation/texture
    TerrainGenerator m_terrain;
    GLuint m_terrain_texture;

    // sphere stuff
    Sphere m_sphere;        // Stores sphere
    GLuint m_sphere_vao;    // Stores id of sphere VAO
    GLuint m_sphere_vbo;    // Stores id of sphere VBO
    void initSphere();
    void setSphereVBO();
    void delSphere();

    // misc helpers
    void rebuildCameraMatrices(int w, int h);
    void sendUniforms(GLuint *shader);

    /*** OUTLINE STUFF ***/
    GLuint m_outline_vbo;
    GLuint m_outline_vao;
    GLuint m_outline_shader;
    void paintOutline();
    bool outline = true;

    /*** skybox ***/
    Skybox m_skybox;

    float  m_angleX;
    float  m_angleY;
    float  m_zoom;
    QPoint m_prevMousePos;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;
};
