#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <QObject>
#include "rgba.h"

// Enumeration values for the Brush types from which the user can choose in the GUI.
enum BrushType {
    BRUSH_CONSTANT,
    BRUSH_LINEAR,
    BRUSH_QUADRATIC,
    BRUSH_SMUDGE,
    BRUSH_SPRAY,
    BRUSH_SPEED,
    BRUSH_FILL,
    BRUSH_CUSTOM,
    NUM_BRUSH_TYPES
};

// Enumeration values for the Filters that the user can select in the GUI.
enum FilterType {
    FILTER_EDGE_DETECT,
    FILTER_BLUR,
    FILTER_SCALE,
    FILTER_MEDIAN,
    FILTER_CHROMATIC,
    FILTER_MAPPING,
    FILTER_ROTATION,
    FILTER_BILATERAL,
    NUM_FILTER_TYPES
};

enum TerrainType {
    TERRAIN_WATER,
    TERRAIN_FLATLANDS,
    TERRAIN_MOUNTAINS
};

enum ShaderType {
    SHADER_PHONG,
    SHADER_TOON
};

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 1;
    int shapeParameter2 = 1;
    bool outlines;
    bool skybox;
    int shaderType;

    // Brush
    int brushType = BRUSH_CONSTANT;      // The user's selected brush @see BrushType
    int brushRadius;    // The brush radius
    int brushTerrain;
    RGBA brushColor;
    int brushDensity; // This is for spray brush (extra credit)
    bool fixAlphaBlending; // Fix alpha blending (extra credit)

    // Filter
    int filterType;                     // The selected filter @see FilterType
    float edgeDetectSensitivity;    // Edge detection sensitivity, from 0 to 1.
    int blurRadius;                 // Selected blur radius
    float scaleX;                   // Horizontal scale factor
    float scaleY;                   // Vertical scale factor
    int medianRadius;               // Median radius (extra credit)
    float rotationAngle;            // Rotation angle (extra credit)
    int bilateralRadius;            // Bilateral radius (extra credit)
    float lambda_1;                 // Chromatic aberration labmda 1 (extra credit)
    float lambda_2;                 // Chromatic aberration labmda 2 (extra credit)
    float lambda_3;                 // Chromatic aberration labmda 3 (extra credit)
    bool nonLinearMap;              // Use non-linear mapping function for tone mapping (extra credit)
    float gamma;                    // Gamma for tone mapping (extra credit)

    QString imagePath;

    void loadSettingsOrDefaults();
    void saveSettings();
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H
