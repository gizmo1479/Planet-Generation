#include "terraingenerator.h"

#include <cmath>
#include "glm/glm.hpp"

// Constructor
TerrainGenerator::TerrainGenerator()
{
  // Define resolution of terrain generation
  m_resolution = 100;

  // Generate random vector lookup table
  m_lookupSize = 1024;
  m_randVecLookup.reserve(m_lookupSize);

  // Initialize random number generator
  std::srand(1230);

  // Populate random vector lookup table
  for (int i = 0; i < m_lookupSize; i++)
  {
    m_randVecLookup.push_back(glm::vec2(std::rand() * 2.0 / RAND_MAX - 1.0,
                                        std::rand() * 2.0 / RAND_MAX - 1.0));
    }
}

// Destructor
TerrainGenerator::~TerrainGenerator()
{
    m_randVecLookup.clear();
}

// Samples the (infinite) random vector grid at (row, col)
glm::vec2 TerrainGenerator::sampleRandomVector(int row, int col)
{
    std::hash<int> intHash;
    int index = intHash(row * 41 + col * 43) % m_lookupSize;
    return m_randVecLookup.at(index);
}

// Helper for computePerlin() and, possibly, getColor()
float interpolate(float A, float B, float alpha) {
    float eased = 3 * pow(alpha, 2) - 2 * pow(alpha, 3);

    return A + (eased * (B - A));
}

// Computes the intensity of Perlin noise at some point
float TerrainGenerator::computePerlin(float x, float y) {
    int x_int = floor(x);
    int y_int = floor(y);
    glm::vec2 grid1 = glm::vec2{ x_int, y_int };
    glm::vec2 grid2 = glm::vec2{ x_int + 1, y_int };
    glm::vec2 grid3 = glm::vec2{ x_int, y_int + 1 };
    glm::vec2 grid4 = glm::vec2{ x_int + 1, y_int + 1 };

    glm::vec2 int_point = glm::vec2{ x, y };
    glm::vec2 offset1 = int_point - grid1;
    glm::vec2 offset2 = int_point - grid2;
    glm::vec2 offset3 = int_point - grid3;
    glm::vec2 offset4 = int_point - grid4;

    glm::vec2 random1 = sampleRandomVector(grid1.x, grid1.y);
    glm::vec2 random2 = sampleRandomVector(grid2.x, grid2.y);
    glm::vec2 random3 = sampleRandomVector(grid3.x, grid3.y);
    glm::vec2 random4 = sampleRandomVector(grid4.x, grid4.y);

    float dot1 = glm::dot(random1, offset1);
    float dot2 = glm::dot(random2, offset2);
    float dot3 = glm::dot(random3, offset3);
    float dot4 = glm::dot(random4, offset4);

    float dx = x - x_int;
    float dy = y - y_int;
    float G = interpolate(dot1, dot2, dx);
    float H = interpolate(dot3, dot4, dx);

    return interpolate(G, H, dy);
}

// Takes a normalized (x, y) position, in range [0,1)
// Returns a height value, z, by using Perlin noise
float TerrainGenerator::getHeight(float x, float y) {
    //    float amp = 1;
    //    float x_mod = 0.5;
    //    float y_mod = 1.5;
    //    float z = amp * computePerlin(x_mod * x * 5, y_mod * y * 5) / 2;

    float z1 = 0.5 * computePerlin(0.5 * x * 5, 0.5 * y * 5) / 2;
//    float z2 = 0.25 * computePerlin(1 * x * 5, 1 * y * 5) / 2;
//    float z3 = 0.125 * computePerlin(2 * x * 5, 2 * y * 5) / 2;
//    float z4 = 0.0625 * computePerlin(4 * x * 5, 4 * y * 5) / 2;

    return z1; // + z2 + z3 + z4;
}

// Generates the height map for generating the terrain
std::vector<float> TerrainGenerator::generateTerrain() {
    std::vector<float> heights;
    heights.reserve(m_resolution * m_resolution);

    for(int x = 0; x < m_resolution; x++) {
        for(int y = 0; y < m_resolution; y++) {
            float h = getHeight(x,y);
            heights.push_back(h);
        }
    }
    return heights;
}
