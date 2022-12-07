#include "sphere.h"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    if (m_param1 < 2) m_param1 = 2;
    if (m_param2 < 3) m_param2 = 3;
    setVertexData();
}

glm::vec3 Sphere::getNormal(glm::vec3 vec) {
    // { dx, dy, dz } = { 2x, 2y, 2z }
    return glm::normalize(glm::vec3{ 2 * vec.x, 2 * vec.y, 2 * vec.z });
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, getNormal(topLeft));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, getNormal(bottomLeft));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, getNormal(bottomRight));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, getNormal(topLeft));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, getNormal(bottomRight));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, getNormal(topRight));
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
