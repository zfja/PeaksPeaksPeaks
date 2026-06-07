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

    QColor chart_color1 = QColor("#28a5e8");
    QColor chart_color2 = QColor("#e82862");
    QString x_title = "wavelength [nm]";
    QString y_title = "intensity";
    QString basePath;

    QLineSeries* fitSeries = nullptr;
    QChart* currentChart = nullptr;
    QGraphicsEllipseItem* markerItem = nullptr;
    QGraphicsRectItem* markerItem2 = nullptr; 
    int selectionState = 1;

    QGraphicsLineItem* p1_lineLeft = nullptr;
    QGraphicsLineItem* p1_lineRight = nullptr;
    QGraphicsLineItem* p2_lineLeft = nullptr;
    QGraphicsLineItem* p2_lineRight = nullptr;

    double p1_deltaX = 0.0; // Aktualna odległość kresek od środka (w jendostkach osi X)
    double p2_deltaX = 0.0;
    double arrowStep = 0.5; // Krok poszerzania strzałkami (możesz zmienić np. na 0.1)

    void updateWidthLines(int peakNum);

    void loadSelectedItem(QListWidgetItem* item);
    void showMarkerAtX(double x);
};
