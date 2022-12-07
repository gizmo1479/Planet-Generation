#pragma once

#include <vector>
#include <glm/glm.hpp>

class TerrainGenerator
{
public:
    virtual void addPointToVector(glm::vec3 point, std::vector<float>& vector) {
        vector.push_back(point.x);
        vector.push_back(point.y);
        vector.push_back(point.z);
    }


};
