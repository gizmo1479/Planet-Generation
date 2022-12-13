#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <iostream>

void MainWindow::initialize() {
    settings.loadSettingsOrDefaults();

    planetgeneration = new PlanetGeneration;

    QHBoxLayout *hLayout = new QHBoxLayout; // horizontal alignment
    QVBoxLayout *vLayout = new QVBoxLayout(); // vertical alignment
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addLayout(vLayout);
    planetgeneration->setMinimumWidth(500);
    hLayout->addWidget(planetgeneration, 1);
    setupCanvas2D();
    hLayout->addWidget(planetgeneration->m_canvas, 2);
    this->setLayout(hLayout);

    // Create labels in sidebox
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    QLabel *tesselation_label = new QLabel(); // Parameters label
    tesselation_label->setText("Tesselation");
    tesselation_label->setFont(font);
    QLabel *shaders_label = new QLabel(); // shaders label
    shaders_label->setText("Shaders");
    shaders_label->setFont(font);
    QLabel *brushes_label = new QLabel(); // Filters label
    brushes_label->setText("Terrain Brush");
    brushes_label->setFont(font);
    QLabel *param1_label = new QLabel(); // Parameter 1 label
    param1_label->setText("Parameter 1:");
    QLabel *param2_label = new QLabel(); // Parameter 2 label
    param2_label->setText("Parameter 2:");

    // Creates the boxes containing the parameter sliders and number boxes
    QGroupBox *p1Layout = new QGroupBox(); // horizonal slider 1 alignment
    QHBoxLayout *l1 = new QHBoxLayout();
    QGroupBox *p2Layout = new QGroupBox(); // horizonal slider 2 alignment
    QHBoxLayout *l2 = new QHBoxLayout();

    // Create slider controls to control parameters
    p1Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 1 slider
    p1Slider->setTickInterval(1);
    p1Slider->setMinimum(1);
    p1Slider->setMaximum(50);
    p1Slider->setValue(1);

    p1Box = new QSpinBox();
    p1Box->setMinimum(1);
    p1Box->setMaximum(50);
    p1Box->setSingleStep(1);
    p1Box->setValue(1);

    p2Slider = new QSlider(Qt::Orientation::Horizontal); // Parameter 2 slider
    p2Slider->setTickInterval(1);
    p2Slider->setMinimum(1);
    p2Slider->setMaximum(50);
    p2Slider->setValue(1);

    p2Box = new QSpinBox();
    p2Box->setMinimum(1);
    p2Box->setMaximum(50);
    p2Box->setSingleStep(1);
    p2Box->setValue(1);

    // Adds the slider and number box to the parameter layouts
    l1->addWidget(p1Slider);
    l1->addWidget(p1Box);
    p1Layout->setLayout(l1);

    l2->addWidget(p2Slider);
    l2->addWidget(p2Box);
    p2Layout->setLayout(l2);

    vLayout->addWidget(tesselation_label);

    vLayout->addWidget(param1_label);
    vLayout->addWidget(p1Layout);
    vLayout->addWidget(param2_label);
    vLayout->addWidget(p2Layout);

    vLayout->addWidget(shaders_label);
    QGroupBox *shaderGroup = new QGroupBox();
    QVBoxLayout *shaderLayout = new QVBoxLayout();
    addCheckBox(shaderLayout, "Skybox", settings.skybox, [this] { settings.skybox = !settings.skybox; });
    addCheckBox(shaderLayout, "Outlines", settings.outlines, [this] { settings.outlines = !settings.outlines; });
    addRadioButton(shaderLayout, "Phong", settings.shaderType == SHADER_PHONG, [this] { setShaderType(SHADER_PHONG); });
    addRadioButton(shaderLayout, "Toon Shader", settings.shaderType == SHADER_TOON, [this] { setShaderType(SHADER_TOON); });
    shaderGroup->setLayout(shaderLayout);
    vLayout->addWidget(shaderGroup);

    vLayout->addWidget(brushes_label);
    QGroupBox *brushesGroup = new QGroupBox();
    QVBoxLayout *brushesLayout = new QVBoxLayout();
    addPushButton(brushesLayout, "Clear Map", &MainWindow::onClearButtonClick);
    addPushButton(brushesLayout, "Render Map", &::MainWindow::onRenderButtonClick);
    addSpinBox(brushesLayout, "radius", 1, 100, 1, settings.brushRadius, [this](int value){ setIntVal(settings.brushRadius, value); });
    addRadioButton(brushesLayout, "Water", settings.brushTerrain == TERRAIN_WATER, [this]{ setTerrainType(TERRAIN_WATER); });
    addRadioButton(brushesLayout, "Flatlands", settings.brushTerrain == TERRAIN_FLATLANDS, [this]{ setTerrainType(TERRAIN_FLATLANDS); });
    addPushButton(brushesLayout, "Save Map", &MainWindow::onSaveButtonClick);
    addPushButton(brushesLayout, "Load Map", &MainWindow::onUploadButtonClick);
    brushesGroup->setLayout(brushesLayout);
    vLayout->addWidget(brushesGroup);
    connectUIElements();

    // Set default values of 5 for tesselation parameters
    onValChangeP1(5);
    onValChangeP2(5);
}

/**
 * @brief Sets up Canvas2D
 */
void MainWindow::setupCanvas2D() {
    planetgeneration->m_canvas = new Canvas2D();
    planetgeneration->m_canvas->init();

    if (!settings.imagePath.isEmpty()) {
        planetgeneration->m_canvas->loadImageFromFile(settings.imagePath);
    }
}

void MainWindow::finish() {
    planetgeneration->finish();
    delete(planetgeneration);
}

void MainWindow::connectUIElements() {
    connectParam1();
    connectParam2();
}

void MainWindow::connectParam1() {
    connect(p1Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP1);
    connect(p1Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP1);
}

void MainWindow::connectParam2() {
    connect(p2Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP2);
    connect(p2Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP2);
}


void MainWindow::onValChangeP1(int newValue) {
    p1Slider->setValue(newValue);
    p1Box->setValue(newValue);
    settings.shapeParameter1 = p1Slider->value();
    planetgeneration->settingsChanged();
}

void MainWindow::onValChangeP2(int newValue) {
    p2Slider->setValue(newValue);
    p2Box->setValue(newValue);
    settings.shapeParameter2 = p2Slider->value();
    planetgeneration->settingsChanged();
}

// ------ FUNCTIONS FOR ADDING UI COMPONENTS ------

void MainWindow::addHeading(QBoxLayout *layout, QString text) {
    QFont font;
    font.setPointSize(16);
    font.setBold(true);

    QLabel *label = new QLabel(text);
    label->setFont(font);
    layout->addWidget(label);
}

void MainWindow::addLabel(QBoxLayout *layout, QString text) {
    layout->addWidget(new QLabel(text));
}

void MainWindow::addRadioButton(QBoxLayout *layout, QString text, bool value, auto function) {
    QRadioButton *button = new QRadioButton(text);
    button->setChecked(value);
    layout->addWidget(button);
    connect(button, &QRadioButton::clicked, this, function);
}

void MainWindow::addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function) {
    QSpinBox *box = new QSpinBox();
    box->setMinimum(min);
    box->setMaximum(max);
    box->setSingleStep(step);
    box->setValue(val);
    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, text);
    subLayout->addWidget(box);
    layout->addLayout(subLayout);
    connect(box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, function);
}

void MainWindow::addDoubleSpinBox(QBoxLayout *layout, QString text, double min, double max, double step, double val, int decimal, auto function) {
    QDoubleSpinBox *box = new QDoubleSpinBox();
    box->setMinimum(min);
    box->setMaximum(max);
    box->setSingleStep(step);
    box->setValue(val);
    box->setDecimals(decimal);
    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, text);
    subLayout->addWidget(box);
    layout->addLayout(subLayout);
    connect(box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, function);
}

void MainWindow::addPushButton(QBoxLayout *layout, QString text, auto function) {
    QPushButton *button = new QPushButton(text);
    button->setMinimumWidth(140);
    layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, function);
}

void MainWindow::addCheckBox(QBoxLayout *layout, QString text, bool val, auto function) {
    QCheckBox *box = new QCheckBox(text);
    box->setChecked(val);
    layout->addWidget(box);
    connect(box, &QCheckBox::clicked, this, function);
}



// ------ FUNCTIONS FOR UPDATING SETTINGS ------

void MainWindow::setBrushType(int type) {
    settings.brushType = type;
    planetgeneration->m_canvas->settingsChanged();
}

void MainWindow::setFilterType(int type) {
    settings.filterType = type;
    planetgeneration->m_canvas->settingsChanged();
}

void MainWindow::setShaderType(int type) {
    settings.shaderType = type;
}

void MainWindow::setTerrainType(int type) {
    settings.brushTerrain = type;
   planetgeneration->m_canvas->settingsChanged();
}

void MainWindow::setUIntVal(std::uint8_t &setValue, int newValue) {
    setValue = newValue;
    planetgeneration->m_canvas->settingsChanged();
}

void MainWindow::setIntVal(int &setValue, int newValue) {
    setValue = newValue;
    planetgeneration->m_canvas->settingsChanged();
}

void MainWindow::setFloatVal(float &setValue, float newValue) {
    setValue = newValue;
    planetgeneration->m_canvas->settingsChanged();
}

void MainWindow::setBoolVal(bool &setValue, bool newValue) {
    setValue = newValue;
    planetgeneration->m_canvas->settingsChanged();
}


// ------ PUSH BUTTON FUNCTIONS ------
void MainWindow::onRenderButtonClick() {
    planetgeneration->paintCanvas();
}
void MainWindow::onClearButtonClick() {
    planetgeneration->m_canvas->clearCanvas();
}

void MainWindow::onFilterButtonClick() {
    planetgeneration->m_canvas->filterImage();
}

void MainWindow::onRevertButtonClick() {
    planetgeneration->m_canvas->loadImageFromFile(settings.imagePath);
}

void MainWindow::onUploadButtonClick() {
    // Get new image path selected by user
    QString file = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.jpeg)"));
    if (file.isEmpty()) { return; }
    settings.imagePath = file;

    // Display new image
    planetgeneration->m_canvas->loadImageFromFile(settings.imagePath);

    planetgeneration->m_canvas->settingsChanged();
}

void MainWindow::onSaveButtonClick() {
    // Save drawn map
    QByteArray* img = new QByteArray(reinterpret_cast<const char*>(planetgeneration->m_canvas->m_data.data()), 4*planetgeneration->m_canvas->m_data.size());
    QImage now = QImage((const uchar*)img->data(), 500, 500, QImage::Format_RGBX8888);
    QString dir = QFileDialog::getSaveFileName(this, tr("Save File"), "/home", tr("Images (*.png *.jpg)"));
    if (dir.isNull()) {
        std::cerr << "Error: failed to save image to \"" << dir.toStdString() << "\"" << std::endl;
    } else {
        bool success = now.save(dir);
        if (!success) {
            success = now.save(dir, "PNG");
        }
        if (success) {
            std::cout << "Saved rendered image to \"" << dir.toStdString() << "\"" << std::endl;
        } else {
            std::cerr << "Error: failed to save image to \"" << dir.toStdString() << "\"" << std::endl;
        }
    }
}
