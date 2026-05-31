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

    void loadSelectedItem(QListWidgetItem* item);
    void showMarkerAtX(double x);
};
