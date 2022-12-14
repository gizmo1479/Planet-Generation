#pragma once
#ifndef CANVAS2D_H
#define CANVAS2D_H

#include <QLabel>
#include <QMouseEvent>
#include <array>
#include <glm/glm.hpp>
#include "planetgeneration.h"
#include "rgba.h"

class Canvas2D : public QLabel {
    Q_OBJECT
public:
    int m_width = 500;
    int m_height = 500;
    std::vector<RGBA> m_data;
    PlanetGeneration *planet;
    QImage img;
    void init();
    void clearCanvas();
    bool loadImageFromFile(const QString &file);
    void displayImage();
    void resize(int w, int h);

    // This will be called when the settings have changed
    void settingsChanged();

    // Filter TODO: implement
    void filterImage();

private:
    std::vector<float> m_mask;
    std::vector<RGBA> m_smudge;
    int cur_brush; //brush type that current mask was generated using
    int cur_radius; //radius that current mask was generated using
    bool m_isDown;

    void mouseDown(int x, int y);
    void mouseDragged(int x, int y);
    void mouseUp(int x, int y);

    // These are functions overriden from QWidget that we've provided
    // to prevent you from having to interact with Qt's mouse events.
    // These will pass the mouse coordinates to the above mouse functions
    // that you will have to fill in.
    virtual void mousePressEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDown(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseMoveEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDragged(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseReleaseEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseUp(static_cast<int>(x), static_cast<int>(y));
    }

    // TODO: add any member variables or functions you need
    int posToIndex(int x, int y, int width);
    uint8_t floatToInt(float intensity);
    void maskToCanvas(int x, int y);
    float distance(int index);
    void fillMask();
    void fillSmudge(int x, int y);
    std::vector<RGBA> convolve2D(std::vector<RGBA> &data, int k_width, int k_height, const std::vector<float> &kernel);
    void convolve2D(auto data, int k_width, int k_height, const std::vector<float> &kernel, auto &result);
    RGBA getPixelReflected(std::vector<RGBA> &data, int width, int height, int x, int y);
    std::uint8_t rgbaToGray(const RGBA &pixel);
    void filterGray();
    void replaceImage(std::vector<RGBA> filtered);
    void filterEdgeDetect();
    void filterBlur();
    void filterScale();
    RGBA h_prime(std::vector<RGBA> &data, int k, double a, int size, int shift, bool dir);
    double g(double x, double radius);
};

#endif // CANVAS2D_H
