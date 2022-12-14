#include "sphere.h"
#include <algorithm>

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    if (m_param1 < 2) m_param1 = 2;
    if (m_param2 < 3) m_param2 = 3;
    setVertexData();
}

// TODO: not sure if this works for the UV coords...
std::vector<float> Sphere::generateShapeScale(float scale) {
    auto d = std::vector<float>();
//    std::transform(d.begin(), d.end(), d.begin(), [&](float c) {return c * scale;});
    for (int i = 0; i < m_vertexData.size(); i += 5) {
        d.push_back(m_vertexData[i] * scale);
        d.push_back(m_vertexData[i + 1] * scale);
        d.push_back(m_vertexData[i + 2] * scale);
        d.push_back(m_vertexData[i + 3]);
        d.push_back(m_vertexData[i + 4]);
    }

    return d;
}

glm::vec2 Sphere::uv(glm::vec3 point) {
    float u = -1;
    float v = -1;

    if ((point.y == 0.0f) || (point.y == 1.0f)) {
        u = 0.5f;
    } else {
        float meep = atan2(point.z, point.x);
        if (meep < 0) {
            u = -meep / (2.f * M_PI);
        } else {
            u = 1 - (meep / (2.f * M_PI));
        }
    }

    float meep2 = glm::asin(point.y / m_radius);
    v = (meep2 / M_PI) + 0.5f;

    return glm::vec2(u, v);
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    insertVec3(m_vertexData, topLeft);
    insertVec2(m_vertexData, uv(topLeft));

    insertVec3(m_vertexData, bottomLeft);
    insertVec2(m_vertexData, uv(bottomLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec2(m_vertexData, uv(bottomRight));

    insertVec3(m_vertexData, topLeft);
    insertVec2(m_vertexData, uv(topLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec2(m_vertexData, uv(bottomRight));

    insertVec3(m_vertexData, topRight);
    insertVec2(m_vertexData, uv(topRight));
}

glm::vec3 getXYZ(float theta, float phi, float r) {
    // x = r * sin(phi) * sin(theta)
    // y = r * cos(phi)
    // z = r * sin(phi) * cos(theta)
    return glm::vec3{ r * glm::sin(phi) * sin(theta),
                      r * glm::cos(phi),
                      r * glm::sin(phi) * glm::cos(theta) };
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float r = m_radius;
    // phiStep is how tall each tile needs to be (height angle)
    float phiStep = glm::radians(180.f / m_param1);

    for (int i = 0; i < m_param1; i++) {
        float currentPhi = i * phiStep;
        float nextPhi = (i + 1) * phiStep;

        glm::vec3 topLeft = getXYZ(currentTheta, currentPhi, r);
        glm::vec3 topRight = getXYZ(nextTheta, currentPhi, r);
        glm::vec3 bottomLeft = getXYZ(currentTheta, nextPhi, r);
        glm::vec3 bottomRight = getXYZ(nextTheta, nextPhi, r);

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeSphere() {
    float thetaStep = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
     makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

// Inserts a glm::vec2 into a vector of floats.
void Sphere::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}
