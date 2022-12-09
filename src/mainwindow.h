#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include "planetgeneration.h"
#include "canvas2d.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void setupCanvas2D();
    Canvas2D *m_canvas;

    void addHeading(QBoxLayout *layout, QString text);
    void addLabel(QBoxLayout *layout, QString text);
    void addRadioButton(QBoxLayout *layout, QString text, bool value, auto function);
    void addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function);
    void addDoubleSpinBox(QBoxLayout *layout, QString text, double min, double max, double step, double val, int decimal, auto function);
    void addPushButton(QBoxLayout *layout, QString text, auto function);
    void addCheckBox(QBoxLayout *layout, QString text, bool value, auto function);
    void connectUIElements();
    void connectParam1();
    void connectParam2();
    void connectUploadFile();

    PlanetGeneration *planetgeneration;
    QPushButton *uploadFile;
    QSlider *p1Slider;
    QSlider *p2Slider;
    QSpinBox *p1Box;
    QSpinBox *p2Box;
    QSpinBox *radiusBox;

private slots:
    void setBrushType(int type);
    void setFilterType(int type);
    void setTerrainType(int type);
    void setShaderType(int type);

    void setUIntVal(std::uint8_t &setValue, int newValue);
    void setIntVal(int &setValue, int newValue);
    void setFloatVal(float &setValue, float newValue);
    void setBoolVal(bool &setValue, bool newValue);

    void onClearButtonClick();
    void onFilterButtonClick();
    void onRevertButtonClick();
    void onUploadButtonClick();
    void onSaveButtonClick();

    void onValChangeP1(int newValue);
    void onValChangeP2(int newValue);
};
