#pragma once

#include <QMainWindow>
#include <QColor>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QEvent>
#include <QChart>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

struct PeakMeasurement {
    double p1_x = 0.0;
    double p1_width = 0.0;
    double p2_x = 0.0;
    double p2_width = 0.0;
    int final_state = 0;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class PeaksPeaksPeaks;
}
QT_END_NAMESPACE

class PeaksPeaksPeaks : public QMainWindow
{
    Q_OBJECT

public:
    explicit PeaksPeaksPeaks(QWidget *parent = nullptr);
    ~PeaksPeaksPeaks() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::PeaksPeaksPeaks *ui;

    double p1_deltaX = 0.0; 
    double p2_deltaX = 0.0;
    double arrowStep = 0.7; 
    int selectionState = 1;

    QColor chart_color1 = QColor("#28a5e8");
    QColor chart_color2 = QColor("#e82862");
    QString x_title = "wavelength [nm]";
    QString y_title = "intensity";
    QString basePath;

    QLineSeries* fitSeries = nullptr;
    QChart* currentChart = nullptr;
    QGraphicsEllipseItem* markerItem = nullptr;
    QGraphicsRectItem* markerItem2 = nullptr; 

    QGraphicsLineItem* p1_lineLeft = nullptr;
    QGraphicsLineItem* p1_lineRight = nullptr;
    QGraphicsLineItem* p2_lineLeft = nullptr;
    QGraphicsLineItem* p2_lineRight = nullptr;

    std::string currentLoadedFile = "";
    std::map<std::string, PeakMeasurement> measurements; 
    QString csvFilePath;

    void updateWidthLines(int peakNum);
    void loadSelectedItem(QListWidgetItem* item);
    void showMarkerAtX(double x);
    void loadCSV();
    void saveCSV(); 
    void saveCurrentMeasurement(); 
    void restoreMeasurement(const std::string& fileName); 
    void setMarkerToX(QGraphicsItem* marker, double x); 
    void updateInfoPanel();
};
