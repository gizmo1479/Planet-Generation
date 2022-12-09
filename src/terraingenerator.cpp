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

// Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
// Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
glm::vec3 TerrainGenerator::getPosition(int row, int col) {
    // Normalizing the planar coordinates to a unit square
    // makes scaling independent of sampling resolution.
    float x = 1.0 * row / m_resolution;
    float y = 1.0 * col / m_resolution;
    float z = getHeight(x, y);
    return glm::vec3(x,y,z);
}

// Helper for computePerlin() and, possibly, getColor()
float interpolate(float A, float B, float alpha) {
    // Task 4: implement your easing/interpolation function below
    float eased = 3 * pow(alpha, 2) - 2 * pow(alpha, 3);

    return A + (eased * (B - A));
}

// Takes a normalized (x, y) position, in range [0,1)
// Returns a height value, z, by...
float TerrainGenerator::getHeight(float x, float y) {
    // TODO: implement
    return 0;
}

// Computes the normal of a vertex by averaging neighbors
glm::vec3 TerrainGenerator::getNormal(int row, int col) {
    // Task 9: Compute the average normal for the given input indices
    glm::vec3 V = getPosition(row, col);
    std::vector<glm::vec3> positions(8, glm::vec3{0,0,0});

    // store adjacent positions in the positions vector
    // n0 = (row, col + 1)
    positions[0] = getPosition(row, col + 1);
    // n1 = (row - 1, col + 1)
    positions[1] = getPosition(row - 1, col + 1);
    // n2 = (row - 1, col)
    positions[2] = getPosition(row - 1, col);
    // n3 = (row - 1, col - 1)
    positions[3] = getPosition(row - 1, col - 1);
    // n4 = (row, col - 1)
    positions[4] = getPosition(row, col - 1);
    // n5 = (row + 1, col - 1
    positions[5] = getPosition(row + 1, col - 1);
    // n6 = (row + 1, col)
    positions[6] = getPosition(row + 1, col);
    // n7 = (row + 1, col + 1)
    positions[7] = getPosition(row + 1, col + 1);

    glm::vec3 sum = glm::vec3{0,0,0};
    for (int i = 0; i < positions.size(); i++) {
        // for ith position in the positions vector, calculate normal of
        // triangle made from ith and (i+1)th position
        if (i == 7) {
            sum += glm::normalize(glm::cross(positions[i] - V, positions[0] - V));
        } else {
            sum += glm::normalize(glm::cross(positions[i] - V, positions[i + 1] - V));
        }
    }

    return glm::normalize(sum);
}

// Computes color of vertex using normal and, optionally, position
glm::vec3 TerrainGenerator::getColor(glm::vec3 normal, glm::vec3 position) {
    // Task 10: compute color as a function of the normal and position
    // 10.1
//    if (position.z >= 0) {
//        return glm::vec3{1,1,1};
//    } else {
//        return glm::vec3{0.5,0.5,0.5};
//    }

    // 10.2
//    float verticalness = glm::dot(normal, glm::vec3{0,0,1});
//    if (verticalness >= 0.9) {
//        return glm::vec3{1,1,1};
//    } else {
//        return glm::vec3{0.5,0.5,0.5};
//    }

    // 10.3
    float verticalness = glm::dot(normal, glm::vec3{0,0,1});
    if ((verticalness >= 0.9) && (position.z >= -0)) {
        return glm::vec3{1,1,1};
    } else {
        return glm::vec3{0.5,0.5,0.5};
    }
}

// Computes the intensity of Perlin noise at some point
float TerrainGenerator::computePerlin(float x, float y) {
    // Task 1: get grid indices (as ints)
    int x_int = floor(x);
    int y_int = floor(y);
    glm::vec2 grid1 = glm::vec2{ x_int, y_int };
    glm::vec2 grid2 = glm::vec2{ x_int + 1, y_int };
    glm::vec2 grid3 = glm::vec2{ x_int, y_int + 1 };
    glm::vec2 grid4 = glm::vec2{ x_int + 1, y_int + 1 };

    // Task 2: compute offset vectors
    glm::vec2 int_point = glm::vec2{ x, y };
    glm::vec2 offset1 = int_point - grid1;
    glm::vec2 offset2 = int_point - grid2;
    glm::vec2 offset3 = int_point - grid3;
    glm::vec2 offset4 = int_point - grid4;

    // Task 3: compute the dot product between offset and grid vectors
    glm::vec2 random1 = sampleRandomVector(grid1.x, grid1.y);
    glm::vec2 random2 = sampleRandomVector(grid2.x, grid2.y);
    glm::vec2 random3 = sampleRandomVector(grid3.x, grid3.y);
    glm::vec2 random4 = sampleRandomVector(grid4.x, grid4.y);

    float dot1 = glm::dot(random1, offset1);
    float dot2 = glm::dot(random2, offset2);
    float dot3 = glm::dot(random3, offset3);
    float dot4 = glm::dot(random4, offset4);

    // Task 5: use your interpolation function to produce the final value
    float dx = x - x_int;
    float dy = y - y_int;
    float G = interpolate(dot1, dot2, dx);
    float H = interpolate(dot3, dot4, dx);

    return interpolate(G, H, dy);
}

// Helper for generateTerrain()
void addPointToVector(glm::vec3 point, std::vector<float>& vector) {
    vector.push_back(point.x);
    vector.push_back(point.y);
    vector.push_back(point.z);
}

// Generates the geometry of the output triangle mesh
std::vector<float> TerrainGenerator::generateTerrain(std::vector<glm::vec4> canvas) {
    std::vector<float> verts;
    verts.reserve(m_resolution * m_resolution * 6);

    for(int x = 0; x < m_resolution; x++) {
        for(int y = 0; y < m_resolution; y++) {
            int x1 = x;
            int y1 = y;

            int x2 = x + 1;
            int y2 = y + 1;

            glm::vec3 p1 = getPosition(x1,y1);
            glm::vec3 p2 = getPosition(x2,y1);
            glm::vec3 p3 = getPosition(x2,y2);
            glm::vec3 p4 = getPosition(x1,y2);

            glm::vec3 n1 = getNormal(x1,y1);
            glm::vec3 n2 = getNormal(x2,y1);
            glm::vec3 n3 = getNormal(x2,y2);
            glm::vec3 n4 = getNormal(x1,y2);

            // tris 1
            // x1y1z1
            // x2y1z2
            // x2y2z3
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p2, verts);
            addPointToVector(n2, verts);
            addPointToVector(getColor(n2, p2), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            // tris 2
            // x1y1z1
            // x2y2z3
            // x1y2z4
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            addPointToVector(p4, verts);
            addPointToVector(n4, verts);
            addPointToVector(getColor(n4, p4), verts);
        }
    }
    return verts;
}







// std::vector<RGBA> m_canvas contains RGBA values representing what user has drawn on canvas
// given m_canvas, for every RGBA value in the canvas:
//      compare to our list of RGBA values for specific textures and get corresponding texture (if none match get the default --> water)
//      depending on texture, get a height offset within a range defined by texture (randomly?)
//      using height, do getPosition
//      using position, do getNormal
//      add to the verts thing in generateTerrain













