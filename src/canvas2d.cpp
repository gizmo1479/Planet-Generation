#include "canvas2d.h"
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>
#include <iostream>
#include "settings.h"


/**
 * @brief Initializes new 500x500 canvas
 */
void Canvas2D::init() {
    m_width = 500;
    m_height = 500;
    m_mask.resize(pow(2 * settings.brushRadius + 1, 2));
    if (settings.brushTerrain == TERRAIN_WATER) settings.brushColor  = {0, 0, 128, 255};
    else if (settings.brushTerrain == TERRAIN_FLATLANDS) settings.brushColor = {0, 255, 0, 255};

    fillMask();
//    m_smudge.resize(pow(2 * settings.brushRadius + 1, 2));
    clearCanvas();
}

/**
 * @brief Takes an (x,y) coordinate and width and returns the proper index into the array of Canvas data with the corresponding width
 */
int Canvas2D::posToIndex(int x, int y, int width) {
    return (y * width) + x;
}

/**
 * @brief Converts a float to an int
 */
uint8_t Canvas2D::floatToInt(float intensity) {
    return round(intensity * 255);
}

/**
 * @brief Returns the distance from the given index to the center of the mask
 */
float Canvas2D::distance(int index) {
    return sqrt(pow(abs((floor(index / (2 * settings.brushRadius + 1))) - settings.brushRadius), 2) + pow(abs((index % (2 * settings.brushRadius + 1)) - settings.brushRadius), 2));
}


std::uint8_t Canvas2D::rgbaToGray(const RGBA &pixel) {
    return (uint8_t) 0.299 * pixel.r + 0.587 * pixel.g + 0.114 * pixel.b;
}

void Canvas2D::filterGray() {
    for (int row = 0; row < m_height; ++row) {
        for (int col = 0; col < m_width; ++col) {
            size_t currentIndex = m_width * row + col;
            RGBA &currentPixel = m_data[currentIndex];
            uint8_t grayPix = rgbaToGray(currentPixel);
            currentPixel.r = grayPix;
            currentPixel.g = grayPix;
            currentPixel.b = grayPix;
        }
    }
}

void Canvas2D::filterBlur() {
    float weights = 0;
    std::vector<float> kernel (2 * settings.blurRadius + 1, 0);
    for (int i = 0; i < 2 * settings.blurRadius + 1; i++) {
        kernel[i] = 1 - fabs((float) i - settings.blurRadius) / settings.blurRadius;
        weights += kernel[i];
    }
    //normalize
    for (int i = 0; i < 2 * settings.blurRadius + 1; i++) {
        kernel[i] = (float)(kernel[i] / weights);
    }
    std::vector<RGBA> result (m_data.capacity(), RGBA{0, 0, 0, 0});
    convolve2D(m_data, 1, kernel.capacity(), kernel, result);
    replaceImage(result);
    convolve2D(m_data, kernel.capacity(), 1, kernel, result);
    replaceImage(result);
    displayImage();
}

uint8_t clamper(float val) {
    if (val > 255)
        return (uint8_t) 255;
    else if (val < 0)
        return (uint8_t) 0;
    else
        return (uint8_t) val;
}

void Canvas2D::filterEdgeDetect() {
    filterGray();
    //Make Gx
    std::vector<float> kernel = {1, 2, 1}; //Gxv
    std::vector<RGBA_f> Gx (m_data.capacity(), RGBA_f{0, 0, 0, 255});
    convolve2D(m_data, 1, kernel.capacity(), kernel, Gx);
    kernel = {-1, 0, 1}; //Gxh
    convolve2D(Gx, kernel.capacity(), 1, kernel, Gx);
    //Make Gy
    //Kernel is already Gyv
    std::vector<RGBA_f> Gy (m_data.capacity(), RGBA_f{0, 0, 0, 255});
    convolve2D(m_data, 1, kernel.capacity(), kernel, Gy);
    kernel = {1, 2, 1}; //Gyh
    convolve2D(Gy, kernel.capacity(), 1, kernel, Gy);
    //Combine Gx and Gy into G, place into m_data
    for (int i = 0; i < m_data.capacity(); i++) {
        uint8_t red = clamper(settings.edgeDetectSensitivity * (sqrt(pow(Gx[i].r, 2) + pow(Gy[i].r, 2))));
        uint8_t green = clamper(settings.edgeDetectSensitivity * (sqrt(pow(Gx[i].g, 2) + pow(Gy[i].g, 2))));
        uint8_t blue = clamper(settings.edgeDetectSensitivity * (sqrt(pow(Gx[i].b, 2) + pow(Gy[i].b, 2))));
        m_data[i] = RGBA {red, green, blue, 255};
    }
    displayImage();
}

double Canvas2D::g(double x, double radius) {
    if ((x < -radius) || (x > radius) )
        return 0;
    else
        return (1 - fabs(x)/radius) / radius;
}

RGBA Canvas2D::h_prime(std::vector<RGBA> &data, int k, double a, int width, int shift, bool dir) {
    double r_sum = 0, g_sum = 0, b_sum = 0, weights_sum = 0;
    float center = (k/a) + ((1-a)/(2*a));
    double radius = (a > 1) ? 1 : 1/a;
    int left = ceil(center - radius);
    int right = floor(center + radius);
    //clamp left and right
    if (left < 0)
        left = 0;
    if (right > data.size())
        right = data.size();

    for (int i = right; i >= left; i--) {
        auto t_filt = g(i - center, radius);
        int indx;
        if (dir)
            indx = (i * width) + shift;
        else
            indx = i + (width * shift);
        if (indx >= 0 && indx < data.size()) {
            r_sum += t_filt * data[indx].r;
            g_sum += t_filt * data[indx].g;
            b_sum += t_filt * data[indx].b;
            weights_sum += t_filt;
        }
    }
    return RGBA{(uint8_t) (r_sum/weights_sum + 0.5f), (uint8_t) (g_sum/weights_sum + 0.5f), (uint8_t) (b_sum/weights_sum + 0.5f), 255};
}

void Canvas2D::filterScale() {
    //x direction first
    int scaled_width = m_width * settings.scaleX;
    std::vector<RGBA> scaled_x (scaled_width * m_height, RGBA{0, 0, 0, 255});
    if (settings.scaleX == 1) {
        scaled_x = m_data;
    } else {
        for (int r = 0; r < m_height; r++) {
            for (int c = 0; c < scaled_width; c++) {
                int indx = (r * scaled_width) + c;
                scaled_x[indx] = h_prime(m_data, c, settings.scaleX, m_width, r, false);
            }
        }
    }
    //y direction second
    int scaled_height = m_height * settings.scaleY;
    std::vector<RGBA> scaled_img (scaled_width * scaled_height, RGBA{0,0,0,255});
    if (settings.scaleY == 1) {
        scaled_img = scaled_x;
    } else {
        for (int c = 0; c < scaled_width; c++) {
            for (int r = 0; r < scaled_height; r++) {
                int indx = (r * scaled_width) + c;
                scaled_img[indx] = h_prime(scaled_x, r, settings.scaleY, scaled_width, c, true);
            }
        }
    }
    //resize canvas and replace image data
    m_width = scaled_width;
    m_height = scaled_height;
    m_data.resize(m_width * m_height);
    replaceImage(scaled_img);
    displayImage();
}

void Canvas2D::replaceImage(std::vector<RGBA> filtered) {
    for (int i = 0; i < filtered.capacity(); i++) {
        m_data[i] = filtered[i];
    }
}

RGBA Canvas2D::getPixelReflected(std::vector<RGBA> &data, int width, int height, int x, int y) {
    int newX = (x < 0) ? abs(x) : width - 1 - (x % (width - 1));
    int newY = (y < 0) ? abs(y) : height - 1 - (y % (height - 1));
    return data[width * newY + newX];
}

void Canvas2D::convolve2D(auto data, int k_width, int k_height, const std::vector<float> &kernel, auto &result) {
    int h_radius = (k_width - 1) / 2;
    int v_radius = (k_height - 1) / 2;

    for (int r = 0; r < m_height; r++) {
        for (int c = 0; c < m_width; c++) {
            size_t centerIndex = r * m_width + c;
            float redAcc = 0, greenAcc = 0, blueAcc = 0;

            for (int m = k_height - 1; m >= 0; m--) {
                for (int n = k_width - 1; n >= 0; n--) {
                    float red, green, blue, indX;
                    int offR = r - v_radius + m;
                    int offC = c - h_radius + n;
                    if (offR >= m_height || offR < 0 || offC >= m_width || offC < 0) {
                        int newX = (offC < 0) ? abs(offC) : m_width - 1 - (offC % (m_width - 1));
                        int newY = (offR < 0) ? abs(offR) : m_height - 1 - (offR % (m_height - 1));
                        indX = m_width * newY + newX;
                        red = data[indX].r;
                        green = data[indX].g;
                        blue = data[indX].b;
                    } else {
                        indX = centerIndex + ((m - v_radius) * m_width) + (n - h_radius);
                        red = data[indX].r;
                        green = data[indX].g;
                        blue = data[indX].b;
                    }
                    float weight = kernel[std::max(m, n)];
                    redAcc += weight * (float) red;
                    greenAcc += weight * (float) green;
                    blueAcc += weight * (float) blue;
                }
            }
            if (settings.filterType == FILTER_BLUR) {
                result[centerIndex].r = (uint8_t)redAcc;
                result[centerIndex].g = (uint8_t)greenAcc;
                result[centerIndex].b = (uint8_t)blueAcc;
                result[centerIndex].a = 255;
            } else if (settings.filterType == FILTER_EDGE_DETECT) {
                result[centerIndex].r = redAcc;
                result[centerIndex].g = greenAcc;
                result[centerIndex].b = blueAcc;
                result[centerIndex].a = 255;
            }
        }
    }
}

/**
 * @brief Fills a vector equal to the size of the mask for later smudge usage using the pixel data around coordinate (x, y) on the canvas
 */
void Canvas2D::fillSmudge(int x, int y) {
    for (int n = 0; n < 2 * settings.brushRadius + 1; n++)  {
            for (int m = 0; m < 2 * settings.brushRadius + 1; m++) {
                if ((y - settings.brushRadius + n >= 0) && (y - settings.brushRadius + n < m_height) && (x - settings.brushRadius + m >= 0) && (x - settings.brushRadius + m < m_width)) {
                    int canvas_index = posToIndex(x - settings.brushRadius + m, y - settings.brushRadius + n, m_width);
                    m_smudge[posToIndex(m, n, 2 * settings.brushRadius + 1)] = m_data[canvas_index];
                } else {
                    m_smudge[posToIndex(m, n, 2 * settings.brushRadius + 1)] = RGBA{0, 0, 0, 0};
            }
        }
    }
}

/**
 * @brief Fills the mask with the appropriate opacity values
 */
void Canvas2D::fillMask(){
    cur_brush = BRUSH_CONSTANT;
    settings.brushType = BRUSH_CONSTANT;
    cur_radius = settings.brushRadius;
    for (int i = 0; i < m_mask.size(); i++) {
        float dist = distance(i);
        float opacity;
        switch (cur_brush) {
        case BRUSH_CONSTANT:
            if (dist <= settings.brushRadius) {
                opacity = 1;
            } else {
                opacity = 0;
            }
            break;
        case BRUSH_LINEAR:
            if (dist <= settings.brushRadius) {
                opacity = fmax(0, 1 - (dist / settings.brushRadius));
            } else {
                opacity = 0;
            }
            break;
        case BRUSH_QUADRATIC:
            if (dist <= settings.brushRadius) {
                opacity = (pow(dist, 2) / pow(settings.brushRadius, 2)) - (2 * dist / settings.brushRadius) + 1;
            } else {
                opacity = 0;
            }
            break;
        case BRUSH_SMUDGE:
            if (dist <= settings.brushRadius) {
                opacity = fmax(0, 1 - (dist / settings.brushRadius));
            } else {
                opacity = 0;
            }
            break;
        //default case to get rid of warning on assigning garbage opacity;
        default:
            if (dist <= settings.brushRadius) {
                opacity = 1;
            } else {
                opacity = 0;
            }
            break;
        }
        m_mask[i] = opacity;
    }
}

/**
 * @brief Takes an (x,y) coordinate and paints the current mask onto the surrounding area on the Canvas
 */
void Canvas2D::maskToCanvas(int x, int y) {
    for (int n = 0; n < 2 * settings.brushRadius + 1; n++)  {
        for (int m = 0; m < 2 * settings.brushRadius + 1; m++) {
            if ((y - settings.brushRadius + n >= 0) && (y - settings.brushRadius + n < m_height) && (x - settings.brushRadius + m >= 0) && (x - settings.brushRadius + m < m_width)) {
                int canvas_index = posToIndex(x - settings.brushRadius + m, y - settings.brushRadius + n, m_width);
                int mask_index = posToIndex(m, n, 2 * settings.brushRadius + 1);
                float opacity = m_mask[mask_index];
                RGBA new_color;

                if (settings.brushType != BRUSH_SMUDGE) {
                    float alpha = ((float) settings.brushColor.a) / 255;
                    new_color = {(uint8_t) (((opacity * alpha) * settings.brushColor.r  + ((1 - (opacity * alpha)) * m_data[canvas_index].r))),
                                 (uint8_t) (((opacity * alpha) * settings.brushColor.g  + ((1 - (opacity * alpha)) * m_data[canvas_index].g))),
                                 (uint8_t) (((opacity * alpha) * settings.brushColor.b  + ((1 - (opacity * alpha)) * m_data[canvas_index].b))),
                                 255};
                } else {
                    RGBA old_color = m_smudge[mask_index];
                    new_color = {(uint8_t) ((opacity * old_color.r  + ((1 - opacity) * m_data[canvas_index].r)) + 0.5f),
                                 (uint8_t) ((opacity * old_color.g + ((1 - opacity) * m_data[canvas_index].g)) + 0.5f),
                                 (uint8_t) ((opacity * old_color.b + ((1 - opacity) * m_data[canvas_index].b)) + 0.5f),
                                 255};
                    m_smudge[mask_index] = new_color;
                }

                m_data[canvas_index] = new_color;
            } else if (settings.brushType == BRUSH_SMUDGE) {
                m_smudge[posToIndex(m, n, 2 * settings.brushRadius + 1)] = RGBA{0, 0, 0, 0};
            }
        }
    }
}

/**
 * @brief Canvas2D::clearCanvas sets all canvas pixels to blank white
 */
void Canvas2D::clearCanvas() {
    m_data.assign(m_width * m_height, RGBA{0, 0, 128, 255});
    settings.imagePath = "";
    displayImage();
}

/**
 * @brief Stores the image specified from the input file in this class's
 * `std::vector<RGBA> m_image`.
 * Also saves the image width and height to canvas width and height respectively.
 * @param file: file path to an image
 * @return True if successfully loads image, False otherwise.
 */
bool Canvas2D::loadImageFromFile(const QString &file) {
    QImage myImage;
    if (!myImage.load(file)) {
        std::cout<<"Failed to load in image"<<std::endl;
        return false;
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    m_width = myImage.width();
    m_height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    m_data.clear();
    m_data.reserve(m_width * m_height);
    for (int i = 0; i < arr.size() / 4.f; i++){
        m_data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
    displayImage();
    return true;
}

/**
 * @brief Get Canvas2D's image data and display this to the GUI
 */
void Canvas2D::displayImage() {
    QByteArray* img = new QByteArray(reinterpret_cast<const char*>(m_data.data()), 4*m_data.size());
    m_img = QImage((const uchar*)img->data(), m_width, m_height, QImage::Format_RGBX8888);
    setPixmap(QPixmap::fromImage(m_img));
    setFixedSize(m_width, m_height);
    update();
}

/**
 * @brief Canvas2D::resize resizes canvas to new width and height
 * @param w
 * @param h
 */
void Canvas2D::resize(int w, int h) {
    m_width = w;
    m_height = h;
    m_data.resize(w * h);
    displayImage();
}

/**
 * @brief Called when the filter button is pressed in the UI
 */
void Canvas2D::filterImage() {
    switch (settings.filterType) {
    case FILTER_BLUR:
        filterBlur();
        break;
    case FILTER_EDGE_DETECT:
        filterEdgeDetect();
        break;
    case FILTER_SCALE:
        filterScale();
        break;
    }
}

/**
 * @brief Called when any of the parameters in the UI are modified.
 */
void Canvas2D::settingsChanged() {
    // this saves your UI settings locally to load next time you run the program
    settings.saveSettings();

    // TODO: fill in what you need to do when brush or filter parameters change
    if (settings.brushTerrain == TERRAIN_WATER) settings.brushColor  = {0, 0, 128, 255};
    else if (settings.brushTerrain == TERRAIN_FLATLANDS) settings.brushColor = {0, 255, 0, 255};
    if (settings.brushRadius != cur_radius) {
        m_smudge.resize(pow(2 * settings.brushRadius + 1, 2));
        m_mask.resize(pow(2 * settings.brushRadius + 1, 2));
        fillMask();
    } else if (settings.brushType != cur_brush) {
        fillMask();
    }
}

/**
 * @brief These functions are called when the mouse is clicked and dragged on the canvas
 */
void Canvas2D::mouseDown(int x, int y) {
    // Brush TODO
    m_isDown = true;
    if (0 <= x && x <= m_width && 0 <= y && y <= m_height) {
        if (settings.brushType != BRUSH_SMUDGE) {
            maskToCanvas(x, y);
            displayImage();
        } else {
            fillSmudge(x, y);
        }

    }
}

void Canvas2D::mouseDragged(int x, int y) {
    // Brush TODO
    if (m_isDown) {
        if (0 <= x && x <= m_width && 0 <= y && y <= m_height) {
            maskToCanvas(x, y);
            displayImage();
        }
    }
}

void Canvas2D::mouseUp(int x, int y) {
    // Brush TODO
    m_isDown = false;
}
