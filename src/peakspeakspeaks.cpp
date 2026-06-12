// PeaksPeaksPeaks — Copyright (c) 2026 Zofia Tryznowska
#include "peakspeakspeaks.h"
#include "./ui_peakspeakspeaks.h"
#include "FileManager.h"
#include "SpectrumLoader.h"
#include "MathEngine.h"

#include <QApplication>
#include <QChart>
#include <QChartView>
#include <QColorDialog>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFont>
#include <QFrame>
#include <QGraphicsEllipseItem>
#include <QLabel>
#include <QLegendMarker>
#include <QLineEdit>
#include <QLineSeries>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMargins>
#include <QMouseEvent>
#include <QPen>
#include <QPushButton>
#include <QResizeEvent>
#include <QScatterSeries>
#include <QSettings>
#include <QShowEvent>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QValueAxis>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <QPainter>
#include <QPixmap>

PeaksPeaksPeaks::PeaksPeaksPeaks(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::PeaksPeaksPeaks)
{
    ui->setupUi(this);

    this->setStyleSheet(R"(
        QMainWindow, QDialog { background-color: #1e1f22; }

        QWidget { color: #e6e8eb; font-size: 14px; }

        QListWidget {
            background-color: #26282c;
            color: #e6e8eb;
            border: 1px solid #34373c;
            border-radius: 8px;
            padding: 6px;
            outline: 0;
        }
        QListWidget::item { padding: 6px 8px; border-radius: 6px; }
        QListWidget::item:hover { background-color: #313438; }
        QListWidget::item:selected { background-color: #6d7cff; color: #ffffff; }

        QPushButton {
            background-color: #383b41;
            color: #f0f2f4;
            border: 1px solid #565a61;
            border-radius: 4px;
            padding: 7px 14px;
        }
        QPushButton:hover { background-color: #43474e; border-color: #6d7cff; }
        QPushButton:pressed { background-color: #6d7cff; color: #ffffff; border-color: #6d7cff; }
        QPushButton:disabled { color: #6b7177; background-color: #2a2c30; border-color: #34373c; }

        QLabel { color: #e6e8eb; background: transparent; }

        QLineEdit, QSpinBox {
            background-color: #26282c;
            color: #e6e8eb;
            border: 1px solid #3a3d42;
            border-radius: 6px;
            padding: 5px 8px;
            selection-background-color: #6d7cff;
        }
        QLineEdit:focus, QSpinBox:focus { border-color: #6d7cff; }
        QSpinBox::up-button, QSpinBox::down-button {
            width: 16px; background-color: #2f3237; border: none;
        }
        QSpinBox::up-button:hover, QSpinBox::down-button:hover { background-color: #3a3d42; }

        QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
        QScrollBar::handle:vertical { background: #3a3d42; border-radius: 5px; min-height: 24px; }
        QScrollBar::handle:vertical:hover { background: #4a4d53; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

        #graphView { background-color: #ffffff; border: 1px solid #34373c; border-radius: 8px; }
    )");
    
    base_stylesheet_ = styleSheet();

    overlay_refresh_timer_ = new QTimer(this);
    overlay_refresh_timer_->setSingleShot(true);
    overlay_refresh_timer_->setInterval(16);
    connect(overlay_refresh_timer_, &QTimer::timeout, this, &PeaksPeaksPeaks::refresh_peak_overlays_from_data);

    ui->listWidget->setFocusPolicy(Qt::StrongFocus);
    ui->listWidget->installEventFilter(this);

    ui->graphView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphView->setFrameShape(QFrame::NoFrame);
    ui->graphView->setFocusPolicy(Qt::NoFocus);
    ui->graphView->setMouseTracking(true);
    ui->graphView->viewport()->setMouseTracking(true);
    ui->graphView->installEventFilter(this);
    ui->graphView->viewport()->installEventFilter(this);

    connect(ui->folderButton, &QPushButton::clicked, this, [=]()
    {
        QString dir = QFileDialog::getExistingDirectory(this, "Choose the folder with .txt spectra", base_path);
        if (!dir.isEmpty())
        {
            QSettings settings("PeaksPeaksPeaks", "PeaksPeaksPeaks");
            settings.setValue("dataFolder", dir);
            set_data_folder(dir);
        }
    });

    connect(ui->savePngButton, &QPushButton::clicked, this, [=]()
    {
        QListWidgetItem* current = ui->listWidget->currentItem();
        if (!ui->graphView->chart() || !current)
            return;

        QString temperature = current->text();

        QString png_dir = output_dir.isEmpty() ? base_path : output_dir;
        QDir().mkpath(png_dir);
        QString suggested = png_dir + "/" + (temperature.isEmpty() ? "chart" : temperature) + ".png";
        QString path = QFileDialog::getSaveFileName(this, "Save chart as PNG", suggested, "PNG image (*.png)");
        if (!path.isEmpty())
        {
            if (!path.endsWith(".png", Qt::CaseInsensitive))
                path += ".png";
            render_chart_png(temperature, path);
        }
    });

    connect(ui->settingsButton, &QPushButton::clicked, this, [=]()
    {
        QDialog settings(this);
        settings.setWindowTitle("Chart Settings");
        settings.setMinimumWidth(250);

        QVBoxLayout* layout = new QVBoxLayout(&settings);

        QPushButton* save_all_button = new QPushButton("Save all", &settings);
        layout->addWidget(save_all_button);
        connect(save_all_button, &QPushButton::clicked, this, [&]()
        {
            save_all_png();
        });

        layout->addSpacing(15);

        QPushButton* chose_color1 = new QPushButton("Change marker 1 colour", &settings);
        layout->addWidget(chose_color1);
        connect(chose_color1, &QPushButton::clicked, this, [&]()
        {
            QColor new_color = QColorDialog::getColor(chart_color1, &settings, "Choose colour");

            if (new_color.isValid())
            {
                chart_color1 = new_color;

                QSettings store("PeaksPeaksPeaks", "PeaksPeaksPeaks");
                store.setValue("chartColor1", chart_color1.name());

                if (ui->graphView->chart() && !ui->graphView->chart()->series().isEmpty())
                {
                    QLineSeries* series = qobject_cast<QLineSeries*>(ui->graphView->chart()->series().first());
                    if (series)
                    {
                        QPen pen = series->pen();
                        pen.setColor(chart_color1);
                        series->setPen(pen);
                    }
                }
            }
        });

        QPushButton* chose_color2 = new QPushButton("Change marker 2 colour", &settings);
        layout->addWidget(chose_color2);
        connect(chose_color2, &QPushButton::clicked, this, [&]()
        {
            QColor new_color = QColorDialog::getColor(chart_color2, &settings, "Choose colour");

            if (new_color.isValid())
            {
                chart_color2 = new_color;

                QSettings store("PeaksPeaksPeaks", "PeaksPeaksPeaks");
                store.setValue("chartColor2", chart_color2.name());

                if (ui->graphView->chart() && ui->graphView->chart()->series().size() > 1)
                {
                    QLineSeries* series = qobject_cast<QLineSeries*>(ui->graphView->chart()->series().last());

                    if (series)
                    {
                        QPen pen = series->pen();
                        pen.setColor(chart_color2);
                        series->setPen(pen);
                    }
                }
            }
        });

        layout->addSpacing(10);

        QLabel* label_x = new QLabel("X-axis Title:", &settings);
        layout->addWidget(label_x);
        QLineEdit* input_x = new QLineEdit(&settings);
        input_x->setText(x_title);
        layout->addWidget(input_x);

        QLabel* label_y = new QLabel("Y-axis Title:", &settings);
        layout->addWidget(label_y);
        QLineEdit* input_y = new QLineEdit(&settings);
        input_y->setText(y_title);
        layout->addWidget(input_y);

        layout->addSpacing(10);

        QLabel* label_window = new QLabel("Smoothing window:", &settings);
        layout->addWidget(label_window);
        QSpinBox* input_window = new QSpinBox(&settings);
        input_window->setRange(3, 151);
        input_window->setSingleStep(2);
        input_window->setValue(sg_window);
        layout->addWidget(input_window);

        QLabel* label_order = new QLabel("Polynomial order:", &settings);
        layout->addWidget(label_order);
        QSpinBox* input_order = new QSpinBox(&settings);
        input_order->setRange(2, 6);
        input_order->setSingleStep(2);
        input_order->setValue(sg_order);
        layout->addWidget(input_order);

        auto apply_smoothing = [this](int window, int order)
        {
            sg_window = window;
            sg_order = order;

            QSettings store("PeaksPeaksPeaks", "PeaksPeaksPeaks");
            store.setValue("sgWindow", sg_window);
            store.setValue("sgOrder", sg_order);

            if (QListWidgetItem* current = ui->listWidget->currentItem())
                load_item(current);
        };

        connect(input_window, &QSpinBox::valueChanged, this, [=](int value)
        {
            apply_smoothing(value, input_order->value());
        });

        connect(input_order, &QSpinBox::valueChanged, this, [=](int value)
        {
            apply_smoothing(input_window->value(), value);
        });

        layout->addSpacing(15);

        QPushButton* save_button = new QPushButton("Save Changes", &settings);
        save_button->setStyleSheet("font-weight: bold;");
        layout->addWidget(save_button);

        connect(save_button, &QPushButton::clicked, &settings, [&]()
        {
            x_title = input_x->text();
            y_title = input_y->text();

            QSettings store("PeaksPeaksPeaks", "PeaksPeaksPeaks");
            store.setValue("xTitle", x_title);
            store.setValue("yTitle", y_title);

            if (ui->graphView->chart())
            {
                QChart* c = ui->graphView->chart();

                if (!c->axes(Qt::Horizontal).isEmpty())
                    c->axes(Qt::Horizontal).first()->setTitleText(x_title);

                if (!c->axes(Qt::Vertical).isEmpty())
                    c->axes(Qt::Vertical).first()->setTitleText(y_title);
            }

            settings.accept();
        });

        settings.exec();
    });

    connect(ui->listWidget, &QListWidget::itemClicked, this, [=](QListWidgetItem* item)
    {
        load_item(item);
    });

    QSettings settings("PeaksPeaksPeaks", "PeaksPeaksPeaks");
    sg_window = settings.value("sgWindow", sg_window).toInt();
    sg_order = settings.value("sgOrder", sg_order).toInt();

    chart_color1 = QColor(settings.value("chartColor1", chart_color1.name()).toString());
    chart_color2 = QColor(settings.value("chartColor2", chart_color2.name()).toString());
    x_title = settings.value("xTitle", x_title).toString();
    y_title = settings.value("yTitle", y_title).toString();

    QString saved = settings.value("dataFolder").toString();
    QString folder = (!saved.isEmpty() && QDir(saved).exists())
                       ? saved
                       : QString::fromStdString(FileManager().get_path());
    set_data_folder(folder);
}

void PeaksPeaksPeaks::set_data_folder(const QString& folder)
{
    base_path = folder;
    output_dir = base_path + "/peakspeakspeaks";
    QDir().mkpath(output_dir);
    csv_path = output_dir + "/data.csv";
    current_file.clear();

    load_csv();

    FileManager manager;
    manager.load(folder.toStdString(), -8, -5);

    ui->listWidget->clear();

    for (const auto& f : manager.files)
    {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(f.display_name));
        item->setData(Qt::UserRole, QString::fromStdString(f.file_name));
        ui->listWidget->addItem(item);
    }

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
        load_item(ui->listWidget->item(0));
    }
    else
    {
        ui->graphView->setChart(new QChart());
    }
}

void PeaksPeaksPeaks::load_item(QListWidgetItem* item)
{
    if (!item)
        return;

    save_measurement();
    std::string file_name = item->data(Qt::UserRole).toString().toStdString();
    std::string full_path = base_path.toStdString() + "/" + file_name;

    current_file = file_name;

    SpectrumLoader loader;
    bool is_broken = true;

    try
    {
        loader.load(full_path);

        if (!loader.data.empty())
        {
            is_broken = false;

            QLineSeries* raw = new QLineSeries();
            QPen pen(chart_color1);
            pen.setWidth(2);
            raw->setPen(pen);
            raw->setName("raw data");

            for (const auto& point : loader.data)
                raw->append(point.first, point.second);

            QChart* new_chart = new QChart();
            new_chart->addSeries(raw);

            new_chart->setBackgroundBrush(Qt::white);
            new_chart->setBackgroundRoundness(0);
            new_chart->setMargins(QMargins(0, 0, 0, 0));

            QValueAxis* axis_x = new QValueAxis();
            axis_x->setTitleText(x_title);
            axis_x->setTickCount(10);
            axis_x->setGridLineVisible(false);
            new_chart->addAxis(axis_x, Qt::AlignBottom);
            raw->attachAxis(axis_x);

            QValueAxis* axis_y = new QValueAxis();
            axis_y->setTitleText(y_title);
            axis_y->setTitleFont(axis_x->titleFont());
            axis_y->setTickCount(8);
            axis_y->setGridLineVisible(false);
            new_chart->addAxis(axis_y, Qt::AlignLeft);
            raw->attachAxis(axis_y);

            MathEngine math;
            auto smoothed = math.smooth(loader.data, sg_window, sg_order);

            QLineSeries* smooth = new QLineSeries();
            smooth->setName("smooth data");
            smooth->setPen(QPen(chart_color2, 2));

            for (const auto& point : smoothed)
                smooth->append(point.first, point.second);

            new_chart->addSeries(smooth);
            smooth->attachAxis(axis_x);
            smooth->attachAxis(axis_y);

            smooth_series = smooth;
            chart = new_chart;

            new_chart->legend()->setVisible(true);
            new_chart->legend()->detachFromChart();
            new_chart->legend()->setBackgroundVisible(true);
            new_chart->legend()->setLabelColor(QColor("#2b2b2b"));

            QFont legend_font = new_chart->legend()->font();
            legend_font.setPixelSize(12);
            new_chart->legend()->setFont(legend_font);

            new_chart->legend()->setMinimumSize(120, 70);
            new_chart->legend()->resize(120, 60);
            update_chart_legend_layout();

            ui->graphView->setChart(new_chart);
            ui->graphView->setRenderHint(QPainter::Antialiasing);

            if (marker1) 
                delete marker1; marker1 = nullptr;
            if (marker2)
                delete marker2; marker2 = nullptr;
            if (p1_left)
                delete p1_left; p1_left = nullptr;
            if (p1_right)
                 delete p1_right; p1_right = nullptr;
            if (p2_left)
                delete p2_left; p2_left = nullptr;
            if (p2_right)
                delete p2_right; p2_right = nullptr;

            state = 1;
            p1_dx = 0.0;
            p2_dx = 0.0;
            p1_x = 0.0;
            p2_x = 0.0;

            marker1 = new QGraphicsEllipseItem();
            marker1->setRect(-4, -4, 7, 7);
            marker1->setBrush(Qt::black);
            marker1->setPen(QPen(Qt::black));
            marker1->setZValue(1000);
            marker1->setVisible(false);
            new_chart->scene()->addItem(marker1);

            marker2 = new QGraphicsRectItem();
            marker2->setRect(-4, -4, 7, 7);
            marker2->setBrush(Qt::black);
            marker2->setPen(QPen(Qt::black));
            marker2->setZValue(1000);
            marker2->setVisible(false);
            new_chart->scene()->addItem(marker2);

            QPen line_pen(Qt::darkGray);
            line_pen.setStyle(Qt::DashLine);
            line_pen.setWidth(2);

            p1_left = new QGraphicsLineItem(); 
            p1_left->setPen(line_pen); p1_left->setZValue(999); 
            p1_left->setVisible(false); 
            new_chart->scene()->addItem(p1_left);

            p1_right = new QGraphicsLineItem(); 
            p1_right->setPen(line_pen); 
            p1_right->setZValue(999); 
            p1_right->setVisible(false); 
            new_chart->scene()->addItem(p1_right);
            
            p2_left = new QGraphicsLineItem(); 
            p2_left->setPen(line_pen); 
            p2_left->setZValue(999); 
            p2_left->setVisible(false); 
            new_chart->scene()->addItem(p2_left);

            p2_right = new QGraphicsLineItem(); 
            p2_right->setPen(line_pen); 
            p2_right->setZValue(999); 
            p2_right->setVisible(false); 
            new_chart->scene()->addItem(p2_right);
            
            restore_measurement(current_file);
        }
        else
        {
            ui->graphView->setChart(new QChart());
        }
    }
    catch (const std::exception&)
    {
        ui->graphView->setChart(new QChart());
    }

    QFont font = item->font();
    font.setBold(is_broken);
    item->setFont(font);
    item->setForeground(is_broken ? QColor("#b61818") : Qt::white);
}

void PeaksPeaksPeaks::capture_design_layout()
{
    design_central_size_ = ui->centralwidget->size();
    design_geometries_.clear();

    for (QObject* child : ui->centralwidget->children())
    {
        if (QWidget* widget = qobject_cast<QWidget*>(child))
            design_geometries_.insert(widget, widget->geometry());
    }
}

void PeaksPeaksPeaks::update_chart_legend_layout()
{
    if (!chart || !chart->legend() || !design_geometries_.contains(ui->graphView))
        return;

    const QRect design_graph = design_geometries_.value(ui->graphView);
    if (design_graph.width() <= 0 || design_graph.height() <= 0)
        return;

    const double gx = static_cast<double>(ui->graphView->width()) / design_graph.width();
    const double gy = static_cast<double>(ui->graphView->height()) / design_graph.height();
    chart->legend()->setPos(440.0 * gx, 20.0 * gy);
}

void PeaksPeaksPeaks::schedule_peak_overlay_refresh()
{
    if (overlay_refresh_timer_)
        overlay_refresh_timer_->start();
}

void PeaksPeaksPeaks::refresh_peak_overlays_from_data()
{
    if (!chart || !smooth_series)
        return;

    if (marker1 && marker1->isVisible())
        set_marker(marker1, p1_x);

    if (marker2 && marker2->isVisible())
        set_marker(marker2, p2_x);

    if (p1_left && p1_left->isVisible())
        update_width_lines(1);

    if (p2_left && p2_left->isVisible())
        update_width_lines(2);

    update_info();
}

void PeaksPeaksPeaks::apply_scaled_layout()
{
    if (design_central_size_.isEmpty() || design_geometries_.isEmpty())
        return;

    const QSize current = ui->centralwidget->size();
    const double sx = static_cast<double>(current.width()) / design_central_size_.width();
    const double sy = static_cast<double>(current.height()) / design_central_size_.height();

    for (auto it = design_geometries_.constBegin(); it != design_geometries_.constEnd(); ++it)
    {
        QWidget* widget = it.key();
        const QRect& design = it.value();
        widget->setGeometry(
            qRound(design.x() * sx),
            qRound(design.y() * sy),
            qMax(1, qRound(design.width() * sx)),
            qMax(1, qRound(design.height() * sy)));
    }

    const int font_size = qMax(8, qRound(base_font_size_ * std::min(sx, sy)));
    QString scaled_stylesheet = base_stylesheet_;
    scaled_stylesheet.replace(
        QString("font-size: %1px").arg(base_font_size_),
        QString("font-size: %1px").arg(font_size));
    setStyleSheet(scaled_stylesheet);

    update_chart_legend_layout();

    if (chart && smooth_series
        && ((marker1 && marker1->isVisible()) || (marker2 && marker2->isVisible())
            || (p1_left && p1_left->isVisible()) || (p2_left && p2_left->isVisible())))
    {
        schedule_peak_overlay_refresh();
    }
}

void PeaksPeaksPeaks::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    apply_scaled_layout();
}

void PeaksPeaksPeaks::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (design_geometries_.isEmpty())
    {
        capture_design_layout();
        const int chrome = ui->menubar->height() + ui->statusbar->height();
        setMinimumSize(QSize(design_central_size_.width() / 2,
                             design_central_size_.height() / 2 + chrome));
        apply_scaled_layout();
    }
}

PeaksPeaksPeaks::~PeaksPeaksPeaks()
{
    delete marker1;
    delete marker2;
    delete ui;
    delete p1_left;
    delete p1_right;
    delete p2_left;
    delete p2_right;
}

void PeaksPeaksPeaks::snap_marker(double x)
{
    if (!chart || !smooth_series || !marker1 || !marker2)
        return;

    const auto points = smooth_series->points();
    if (points.isEmpty())
        return;

    QPointF best_point = points.first();
    double best_dist = std::abs(points.first().x() - x);

    for (const QPointF& p : points)
    {
        double dist = std::abs(p.x() - x);
        if (dist < best_dist)
        {
            best_dist = dist;
            best_point = p;
        }
    }

    QPointF scene_pos = chart->mapToPosition(best_point, smooth_series);
    
    if (state == 1) 
    {
        p1_x = best_point.x();
        marker1->setPos(scene_pos);
        marker1->setVisible(true);
        marker2->setVisible(false);
        if (p1_left) p1_left->setVisible(false);
        if (p1_right) p1_right->setVisible(false);
        if (p2_left) p2_left->setVisible(false);
        if (p2_right) p2_right->setVisible(false);
    } 
    else if (state == 2) 
    {
        p2_x = best_point.x();
        marker2->setPos(scene_pos);
        marker2->setVisible(true);
        if (p2_left) p2_left->setVisible(false);
        if (p2_right) p2_right->setVisible(false);
    }
    update_info();
}

bool PeaksPeaksPeaks::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        auto* key_event = static_cast<QKeyEvent*>(event);
        int key = key_event->key();

        if (obj == ui->listWidget && (key == Qt::Key_Up || key == Qt::Key_Down))
        {
            int current_row = ui->listWidget->currentRow();
            int next_row = (key == Qt::Key_Down) ? current_row + 1 : current_row - 1;
            
            if (next_row >= 0 && next_row < ui->listWidget->count()) 
            {
                ui->listWidget->setCurrentRow(next_row);
                load_item(ui->listWidget->item(next_row));
            }
            return true;
        }

        if (key == Qt::Key_S && (key_event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)))
        {
            ui->savePngButton->click();
            return true;
        }

        if (key == Qt::Key_Left || key == Qt::Key_Right) 
        {
            double change = (key == Qt::Key_Right) ? arrow_step : -arrow_step;
            if (state == 11) 
            {
                p1_dx = std::max(0.0, p1_dx + change);
                update_width_lines(1);
                return true;
            } 
            else if (state == 22) 
            {
                p2_dx = std::max(0.0, p2_dx + change);
                update_width_lines(2);
                return true;
            }
        }

        if (key == Qt::Key_Enter || key == Qt::Key_Return) 
        {
            if (state == 11) 
            {
                state = 2; 
                return true;
            } 
            else if (state == 22) 
            {
                state = 0; 
                return true;
            }
        }

        if (key == Qt::Key_R) 
        {
            if (key_event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) 
            {
                if (!current_file.empty()) 
                {
                    measurements.erase(current_file); 
                    save_csv(); 
                }
                state = 1;
                p1_dx = 0.0; p2_dx = 0.0;
                p1_x = 0.0; p2_x = 0.0;
                if (marker1) marker1->setVisible(false);
                if (marker2) marker2->setVisible(false);
                if (p1_left) p1_left->setVisible(false);
                if (p1_right) p1_right->setVisible(false);
                if (p2_left) p2_left->setVisible(false);
                if (p2_right) p2_right->setVisible(false);
                update_info();
                return true;
            }

            if (state == 11) 
            {
                state = 1; 
                p1_dx = 0.0;
                p1_left->setVisible(false); p1_right->setVisible(false);
                update_info();
                return true;
            } 
            else if (state == 3) 
            {
                state = 1; 
                p1_dx = 0.0;
                if (p1_left) p1_left->setVisible(false);
                if (p1_right) p1_right->setVisible(false);
                update_info();
                return true;
            } 
            else if (state == 22)
            {
                state = 2; 
                p2_left->setVisible(false); p2_right->setVisible(false);
                return true;
            } 
            else if (state == 0) 
            {
                state = 2;
                p2_left->setVisible(false); p2_right->setVisible(false);
                return true;
            } 
            else if (state == 2) 
            {
                state = 3;
                if (marker2) marker2->setVisible(false);
                return true;
            }
        }
    }

    if (obj == ui->graphView->viewport())
    {
        if (event->type() == QEvent::MouseMove)
        {
            if (!chart || !smooth_series) 
                return false;

            auto* mouse_event = static_cast<QMouseEvent*>(event);
            QPointF scene_pos = ui->graphView->mapToScene(mouse_event->pos());
            QPointF value_pos = chart->mapToValue(scene_pos, smooth_series);

            if (state == 1 || state == 2) 
                snap_marker(value_pos.x());

            else if (state == 11 || state == 22) 
            {
                QGraphicsItem* center = (state == 11) ? (QGraphicsItem*)marker1 : (QGraphicsItem*)marker2;
                
                if (center) 
                {
                    double center_x = (state == 11) ? p1_x : p2_x;
                    double delta = std::abs(value_pos.x() - center_x);

                    if (state == 11) 
                    {
                        p1_dx = delta;
                        update_width_lines(1);
                    } 
                    else 
                    {
                        p2_dx = delta;
                        update_width_lines(2);
                    }
                }
            }
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress)
        {
            auto* mouse_event = static_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::LeftButton)
            {
                if (state == 1) 
                {
                    if (!marker1 || !marker1->isVisible())
                        return true;
                    state = 11; 
                    p1_dx = 0.0;
                    p1_left->setVisible(true); p1_right->setVisible(true);
                    update_width_lines(1);
                    update_info();
                    return true;
                }
                else if (state == 11) 
                {
                    state = 2; 
                    return true;
                }
                else if (state == 2) 
                {
                    if (!marker2 || !marker2->isVisible())
                        return true;
                    state = 22; 
                    p2_dx = 0.0;
                    p2_left->setVisible(true); p2_right->setVisible(true);
                    update_width_lines(2);
                    update_info();
                    return true;
                }
                else if (state == 22) 
                {
                    state = 0; 
                    return true;
                }
                else if (state == 0 || state == 3) 
                {
                    return true;
                }
            }
        }
    }

    update_info();
    return QMainWindow::eventFilter(obj, event);
}

void PeaksPeaksPeaks::update_width_lines(int peak)
{
    if (!chart || !smooth_series) 
        return;

    QGraphicsItem* center = (peak == 1) ? (QGraphicsItem*)marker1 : (QGraphicsItem*)marker2;
    if (!center || !center->isVisible()) 
        return;

    const double center_x = (peak == 1) ? p1_x : p2_x;
    const double delta = (peak == 1) ? p1_dx : p2_dx;

    QPointF left_val(center_x - delta, 0);
    QPointF right_val(center_x + delta, 0);

    double left_x = chart->mapToPosition(left_val, smooth_series).x();
    double right_x = chart->mapToPosition(right_val, smooth_series).x();

    QRectF area = chart->plotArea();

    if (peak == 1) 
    {
        p1_left->setLine(left_x, area.top(), left_x, area.bottom());
        p1_right->setLine(right_x, area.top(), right_x, area.bottom());
    } 
    else 
    {
        p2_left->setLine(left_x, area.top(), left_x, area.bottom());
        p2_right->setLine(right_x, area.top(), right_x, area.bottom());
    }
    update_info();

}

void PeaksPeaksPeaks::render_chart_png(const QString& temperature, const QString& path)
{
    if (!ui->graphView->chart())
        return;

    QString title = temperature + " " + QChar(0x00B0) + "C";

    QPixmap chart_pix = ui->graphView->viewport()->grab();
    qreal dpr = chart_pix.devicePixelRatio();
    int title_height = 44;

    QPixmap output(chart_pix.width(), chart_pix.height() + int(title_height * dpr));
    output.setDevicePixelRatio(dpr);
    output.fill(Qt::white);

    qreal logical_w = chart_pix.width() / dpr;

    QPainter painter(&output);
    QFont title_font = painter.font();
    title_font.setPixelSize(18);
    title_font.setBold(true);
    painter.setFont(title_font);
    painter.setPen(Qt::black);
    painter.drawText(QRectF(0, 0, logical_w, title_height), Qt::AlignCenter, title);
    painter.drawPixmap(QPointF(0, title_height), chart_pix);
    painter.end();

    output.save(path, "PNG");
}

void PeaksPeaksPeaks::save_all_png()
{
    int count = ui->listWidget->count();
    if (count == 0)
        return;

    QString png_dir = output_dir.isEmpty() ? base_path : output_dir;
    QDir().mkpath(png_dir);

    int saved_row = ui->listWidget->currentRow();

    for (int row = 0; row < count; ++row)
    {
        QListWidgetItem* item = ui->listWidget->item(row);
        if (!item)
            continue;

        ui->listWidget->setCurrentRow(row);
        load_item(item);

        QApplication::processEvents();

        QString temperature = item->text();
        QString file = png_dir + "/" + (temperature.isEmpty() ? QString("chart_%1").arg(row) : temperature) + ".png";
        render_chart_png(temperature, file);
    }

    if (saved_row >= 0 && saved_row < count)
    {
        ui->listWidget->setCurrentRow(saved_row);
        load_item(ui->listWidget->item(saved_row));
    }
}

void PeaksPeaksPeaks::load_csv()
{
    measurements.clear();
    std::ifstream file(csv_path.toStdString());
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) 
    {
        std::stringstream ss(line);
        std::string fname, p1x_str, p1w_str, p2x_str, p2w_str, state_str;
        
        if (std::getline(ss, fname, ',') && std::getline(ss, p1x_str, ',') && std::getline(ss, p1w_str, ',') && std::getline(ss, p2x_str, ',') && std::getline(ss, p2w_str, ',')) 
        {
            PeakMeasurement m;
            m.p1_x = std::stod(p1x_str);
            m.p1_width = std::stod(p1w_str);
            m.p2_x = std::stod(p2x_str);
            m.p2_width = std::stod(p2w_str);
            
            if (std::getline(ss, state_str, ',')) 
                m.last_state = std::stoi(state_str);
            else
                m.last_state = 0;
            
            measurements[fname] = m;
        }
    }
}

void PeaksPeaksPeaks::save_csv()
{
    std::ofstream file(csv_path.toStdString());
    if (!file.is_open()) return;

    auto temperature_of = [](const std::string& file_name) -> double
    {
        const int start_index = -8;
        const int end_index = -5;
        int start = static_cast<int>(file_name.length()) + start_index;
        int length = end_index - start_index + 1;
        if (start >= 0 && start + length <= static_cast<int>(file_name.length()))
        {
            try { return std::stod(file_name.substr(start, length)); }
            catch (...) { return 0.0; }
        }
        return 0.0;
    };

    std::vector<std::string> keys;
    keys.reserve(measurements.size());
    for (const auto& pair : measurements)
        keys.push_back(pair.first);

    std::sort(keys.begin(), keys.end(), [&](const std::string& a, const std::string& b)
    {
        double ta = temperature_of(a);
        double tb = temperature_of(b);
        if (ta != tb)
            return ta > tb;
        return a < b;
    });

    for (const std::string& key : keys)
    {
        const PeakMeasurement& m = measurements[key];
        file << key << "," << m.p1_x << "," << m.p1_width << "," << m.p2_x << "," << m.p2_width << "," << m.last_state << "\n";
    }
}

void PeaksPeaksPeaks::save_measurement()
{
    if ((state == 0 || state == 3) && !current_file.empty() && chart && smooth_series && marker1) 
    {
        PeakMeasurement m;
        m.last_state = state;
        m.p1_x = p1_x;
        m.p1_width = p1_dx;
        
        if (state == 0 && marker2) 
        {
            m.p2_x = p2_x;
            m.p2_width = p2_dx;
        } 
        else 
        {
            m.p2_x = 0.0;
            m.p2_width = 0.0;
        }
        
        measurements[current_file] = m;
        save_csv(); 
    }
}

void PeaksPeaksPeaks::set_marker(QGraphicsItem* marker, double x)
{
    if (!chart || !smooth_series) 
        return;
    const auto points = smooth_series->points();
    if (points.isEmpty()) 
        return;

    QPointF best_point = points.first();
    double best_dist = std::abs(points.first().x() - x);

    for (const QPointF& p : points) 
    {
        double dist = std::abs(p.x() - x);
        if (dist < best_dist)
        {
            best_dist = dist;
            best_point = p;
        }
    }
    marker->setPos(chart->mapToPosition(best_point, smooth_series));
    marker->setVisible(true);

    if (marker == marker1)
        p1_x = best_point.x();
    else if (marker == marker2)
        p2_x = best_point.x();
}

void PeaksPeaksPeaks::restore_measurement(const std::string& file_name)
{
    if (measurements.count(file_name)) 
    {
        PeakMeasurement m = measurements[file_name];
        state = m.last_state; 

        p1_dx = m.p1_width;
        set_marker(marker1, m.p1_x);
        p1_left->setVisible(true); 
        p1_right->setVisible(true);
        update_width_lines(1);
        
        if (state == 0) 
        {
            p2_dx = m.p2_width;
            set_marker(marker2, m.p2_x);
            p2_left->setVisible(true); 
            p2_right->setVisible(true);
            update_width_lines(2);
        } 
        else 
        {
            p2_dx = 0.0;
            if(marker2) marker2->setVisible(false);
            if(p2_left) p2_left->setVisible(false);
            if(p2_right) p2_right->setVisible(false);
        }
    } 
    else 
    {
        state = 1; 
        p1_dx = 0.0;
        p2_dx = 0.0;
        p1_x = 0.0;
        p2_x = 0.0;
    }
    update_info();
}

void PeaksPeaksPeaks::update_info()
{
    double x1 = 0.0, w1 = 0.0;
    double x2 = 0.0, w2 = 0.0;

    if (state != 1 && marker1) 
    {
        x1 = p1_x;
        w1 = p1_dx * 2.0;
    } 
    else if (state == 1 && marker1 && marker1->isVisible()) 
    {
        x1 = p1_x;
        w1 = 0.0;
    }

    if ((state == 0 || state == 22) && marker2) 
    {
        x2 = p2_x;
        w2 = p2_dx * 2.0;
    } 
    else if (state == 2 && marker2 && marker2->isVisible()) 
    {
        x2 = p2_x;
        w2 = 0.0;
    }

    ui->label_p1_info->setText(QString("● %1 nm").arg(x1, 0, 'f', 2));
    ui->label_p1_width->setText(QString("%1 nm").arg(w1, 0, 'f', 1));
    ui->label_p2_info->setText(QString("■ %1 nm").arg(x2, 0, 'f', 2));
    ui->label_p2_width->setText(QString("%1 nm").arg(w2, 0, 'f', 1));
}
