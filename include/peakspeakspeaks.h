// PeaksPeaksPeaks — Copyright (c) 2026 Zofia Tryznowska
#pragma once

#include <QChart>
#include <QColor>
#include <QEvent>
#include <QKeyEvent>
#include <QLineSeries>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QResizeEvent>
#include <QScatterSeries>
#include <QShowEvent>
#include <QValueAxis>

#include <fstream>
#include <map>
#include <sstream>
#include <string>

/**
 * @brief Saved peak positions and widths for one spectrum file.
 */
struct PeakMeasurement {
    double p1_x = 0.0;      ///< Center x of peak 1 (nm).
    double p1_width = 0.0;  ///< Half-width of peak 1; full width in the UI is 2 * p1_width.
    double p2_x = 0.0;      ///< Center x of peak 2 (nm).
    double p2_width = 0.0;  ///< Half-width of peak 2.
    int last_state = 0;     ///< Workflow state when saved (0 = two peaks, 3 = one peak).
};

QT_BEGIN_NAMESPACE
class QTimer;
namespace Ui {
class PeaksPeaksPeaks;
}
QT_END_NAMESPACE

/**
 * @brief Main window: loads spectra, draws raw and smoothed curves, measures up to two peaks.
 *
 * The user picks peak centers with the mouse and sets their widths with clicks
 * or arrow keys. Measurements are stored in @c data.csv inside the @c peakspeakspeaks
 * subfolder of the selected data directory, and charts can be exported to PNG.
 *
 * @author Zofia Tryznowska
 */
class PeaksPeaksPeaks : public QMainWindow
{
    Q_OBJECT

public:
    explicit PeaksPeaksPeaks(QWidget *parent = nullptr);
    ~PeaksPeaksPeaks() override;

protected:
    /**
     * @brief Handles list navigation and mouse/keyboard peak-picking on the chart.
     *
     * Up/Down switch spectra; moving the mouse places peak centers and sets widths;
     * a click advances the workflow; Left/Right adjust the active width; Enter confirms.
     * @c R steps one stage back, @c Ctrl/Cmd+R clears the current file's measurement,
     * and @c Ctrl/Cmd+S exports the current chart to PNG (same as the Save PNG button).
     */
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    Ui::PeaksPeaksPeaks *ui;

    double p1_dx = 0.0;      ///< Half-width of peak 1 while editing.
    double p2_dx = 0.0;      ///< Half-width of peak 2 while editing.
    double p1_x = 0.0;       ///< Peak 1 center wavelength (nm); source of truth for overlays.
    double p2_x = 0.0;       ///< Peak 2 center wavelength (nm).
    double arrow_step = 0.7; ///< Width change per left/right key press (nm).

    int sg_window = 51;      ///< Savitzky–Golay window length (points), set in Settings.
    int sg_order = 2;        ///< Savitzky–Golay polynomial degree, set in Settings.
    /**
     * Picking workflow:
     * 1  -> place peak 1 center, 11 -> set peak 1 width,
     * 2  -> place peak 2 center, 22 -> set peak 2 width,
     * 0  -> done (two peaks),    3  -> done (single peak).
     */
    int state = 1;

    QColor chart_color1 = QColor("#28a5e8"); ///< Pen color of the raw series.
    QColor chart_color2 = QColor("#e82862"); ///< Pen color of the smoothed series.
    QString x_title = "wavelength [nm]";
    QString y_title = "intensity";
    QString png_name_prefix;                 ///< Prepended to every exported PNG file name.
    QString base_path;                       ///< Folder with the spectra (.txt files).
    QString output_dir;                      ///< Output subfolder ("peakspeakspeaks") for data.csv and PNG exports.

    QLineSeries* smooth_series = nullptr;    ///< Smoothed curve, used for all snapping.
    QChart* chart = nullptr;
    QGraphicsEllipseItem* marker1 = nullptr; ///< Peak 1 center marker.
    QGraphicsRectItem* marker2 = nullptr;    ///< Peak 2 center marker.

    QGraphicsLineItem* p1_left = nullptr;
    QGraphicsLineItem* p1_right = nullptr;
    QGraphicsLineItem* p2_left = nullptr;
    QGraphicsLineItem* p2_right = nullptr;

    std::string current_file;
    std::map<std::string, PeakMeasurement> measurements;
    QString csv_path;

    /// Repositions the width lines of peak @p peak (1 or 2).
    void update_width_lines(int peak);

    /// Sets @p folder as the data directory, creates its @c peakspeakspeaks output
    /// subfolder, and reloads the file list and saved measurements.
    void set_data_folder(const QString& folder);

    /// Saves the open measurement, then loads and draws @p item.
    void load_item(QListWidgetItem* item);

    /// Snaps the active marker (chosen by @c state) to the nearest point of the smoothed curve.
    void snap_marker(double x);

    /// Grabs the current chart, adds a temperature title, and writes it to @p path as PNG.
    void render_chart_png(const QString& temperature, const QString& path);

    /// Exports a PNG for every spectrum in the list into the output folder.
    void save_all_png();

    /// Loads saved measurements from @c data.csv in the output folder.
    void load_csv();

    /// Writes all measurements to @c data.csv, rows sorted by temperature descending.
    void save_csv();

    void save_measurement();
    void restore_measurement(const std::string& file_name);
    void set_marker(QGraphicsItem* marker, double x);
    void update_info();
    QString prefixed_png_name(const QString& name) const;

    void capture_design_layout();
    void apply_scaled_layout();
    void schedule_peak_overlay_refresh();
    void refresh_peak_overlays_from_data();
    void update_chart_legend_layout();

    QTimer* overlay_refresh_timer_ = nullptr;
    QSize design_central_size_;
    QHash<QWidget*, QRect> design_geometries_;
    int base_font_size_ = 14;
    QString base_stylesheet_;
};
