#pragma once

#include <vector>
#include "glm/glm.hpp"

class TerrainGenerator
{
public:
    TerrainGenerator();
    ~TerrainGenerator();
    int getResolution() { return m_resolution; };
    std::vector<float> generateTerrain();

private:
    // Member variables for terrain generation. You will not need to use these directly.
    std::vector<glm::vec2> m_randVecLookup;
    int m_resolution;
    int m_lookupSize;

    // Samples the (infinite) random vector grid at (row, col)
    glm::vec2 sampleRandomVector(int row, int col);

    // Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
    // Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
    glm::vec3 getPosition(int row, int col);

    // Takes a normalized (x, y) position, in range [0,1)
    // Returns a height value, z, by sampling a noise function
    float getHeight(float x, float y);
    float getHeight2(float x, float y);

    // Computes the normal of a vertex by averaging neighbors
//    glm::vec3 getNormal(int row, int col);

    // Computes the intensity of Perlin noise at some point
    float computePerlin(float x, float y);
};
