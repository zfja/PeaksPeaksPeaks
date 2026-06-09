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

/**
 * @brief Saved peak positions and widths for one spectrum file.
 */
struct PeakMeasurement {
    double p1_x = 0.0;      ///< Center x of peak 1 (nm).
    double p1_width = 0.0;  ///< Half-width Δx of peak 1; full width shown in UI is @c 2 * p1_width.
    double p2_x = 0.0;      ///< Center x of peak 2 (nm).
    double p2_width = 0.0;  ///< Half-width Δx of peak 2.
    int final_state = 0;    ///< @ref PeaksPeaksPeaks::selectionState when saved (0 = two peaks, 3 = one peak).
};

QT_BEGIN_NAMESPACE
namespace Ui {
class PeaksPeaksPeaks;
}
QT_END_NAMESPACE

/**
 * @brief Qt main window: load spectra, smooth curves, and measure up to two peaks interactively.
 *
 * Raw and smoothed series are drawn on a chart. The user places markers with the mouse
 * and adjusts peak widths with clicks and arrow keys. Measurements persist in @c data.csv
 * next to the application.
 */
class PeaksPeaksPeaks : public QMainWindow
{
    Q_OBJECT

public:
    explicit PeaksPeaksPeaks(QWidget *parent = nullptr);
    ~PeaksPeaksPeaks() override;

protected:
    /**
     * @brief Handles list navigation and chart interaction (keyboard and mouse).
     *
     * List: Up/Down changes the selected spectrum.
     * Chart: mouse moves markers or width lines depending on @c selectionState;
     * Left/Right nudges width in states 11 and 22; Enter/Escape advance or cancel steps.
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::PeaksPeaksPeaks *ui;

    double p1_deltaX = 0.0;  ///< Half-width Δx for peak 1 while editing.
    double p2_deltaX = 0.0;  ///< Half-width Δx for peak 2 while editing.
    double arrowStep = 0.7;  ///< Width change per Left/Right key press (nm).
    /**
     * Interactive workflow state:
     * - @c 1  — place peak 1 center
     * - @c 11 — set peak 1 width
     * - @c 2  — place peak 2 center
     * - @c 22 — set peak 2 width
     * - @c 0  — finished (two peaks)
     * - @c 3  — finished (single peak only)
     */
    int selectionState = 1;

    QColor chart_color1 = QColor("#28a5e8"); ///< Pen color for the raw data series.
    QColor chart_color2 = QColor("#e82862"); ///< Pen color for the smoothed series.
    QString x_title = "wavelength [nm]";
    QString y_title = "intensity";
    QString basePath;                        ///< Directory containing spectra and @c data.csv.

    QLineSeries* fitSeries = nullptr;             ///< Smoothed curve (second series).
    QChart* currentChart = nullptr;
    QGraphicsEllipseItem* markerItem = nullptr;   ///< Peak 1 center marker.
    QGraphicsRectItem* markerItem2 = nullptr;     ///< Peak 2 center marker (square).

    QGraphicsLineItem* p1_lineLeft = nullptr;
    QGraphicsLineItem* p1_lineRight = nullptr;
    QGraphicsLineItem* p2_lineLeft = nullptr;
    QGraphicsLineItem* p2_lineRight = nullptr;

    std::string currentLoadedFile;
    std::map<std::string, PeakMeasurement> measurements;
    QString csvFilePath;

    /** Repositions the vertical width lines for peak @p peakNum (1 or 2). */
    void updateWidthLines(int peakNum);

    /** Saves the current file's measurement, loads @p item's spectrum, and redraws the chart. */
    void loadSelectedItem(QListWidgetItem* item);

    /** Snaps the active marker (per @c selectionState) to the nearest point on @c fitSeries. */
    void showMarkerAtX(double x);

    void loadCSV();
    void saveCSV();
    void saveCurrentMeasurement();
    void restoreMeasurement(const std::string& fileName);
    void setMarkerToX(QGraphicsItem* marker, double x);
    void updateInfoPanel();
};
