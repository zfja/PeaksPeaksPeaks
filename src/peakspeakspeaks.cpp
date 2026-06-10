#include "peakspeakspeaks.h"
#include "./ui_peakspeakspeaks.h"
#include "FileManager.h"
#include "SpectrumLoader.h"
#include "MathEngine.h"

#include <iostream>
#include <QListWidget>
#include <QListWidgetItem>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QChartView>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QFont>
#include <QPen>
#include <QMargins>
#include <QScatterSeries>
#include <QMouseEvent>
#include <QEvent>
#include <cmath>
#include <algorithm>
#include <QLegendMarker>
#include <QGraphicsEllipseItem>

PeaksPeaksPeaks::PeaksPeaksPeaks(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::PeaksPeaksPeaks)
{
    ui->setupUi(this);

    this->setStyleSheet(R"(
        QMainWindow { background-color: #2b2b2b; }
        QListWidget { background-color: #3c3f41; color: #e0e0e0; border: 1px solid #555555; padding: 5px; font-size: 14px; }
        QListWidget::item { padding: 2px; }
        QListWidget::item:selected { background-color: #2980b9; color: #ffffff; border-radius: 3px; }
        QListWidget::item:hover { background-color: #4f5356; }
        #graphView { background-color: #ffffff; border: 2px solid #555555; }
    )");

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

    FileManager manager;
    std::string path = manager.get_path();
    base_path = QString::fromStdString(path);

    csv_path = base_path + "/data.csv";
    load_csv();

    manager.load(path, -8, -5);

    ui->listWidget->clear();

    for (const auto& f : manager.files)
    {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(f.display_name));
        item->setData(Qt::UserRole, QString::fromStdString(f.file_name));
        ui->listWidget->addItem(item);
    }

    connect(ui->settingsButton, &QPushButton::clicked, this, [=]()
    {
        QDialog settings(this);
        settings.setWindowTitle("Chart Settings");
        settings.setMinimumWidth(250);

        QVBoxLayout* layout = new QVBoxLayout(&settings);

        QPushButton* chose_color1 = new QPushButton("Change marker 1 colour", &settings);
        layout->addWidget(chose_color1);
        connect(chose_color1, &QPushButton::clicked, this, [&]()
        {
            QColor new_color = QColorDialog::getColor(chart_color1, &settings, "Choose colour");

            if (new_color.isValid())
            {
                chart_color1 = new_color;

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

        layout->addSpacing(15);

        QPushButton* save_button = new QPushButton("Save Changes", &settings);
        save_button->setStyleSheet("font-weight: bold;");
        layout->addWidget(save_button);

        connect(save_button, &QPushButton::clicked, &settings, [&]()
        {
            x_title = input_x->text();
            y_title = input_y->text();

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

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
        load_item(ui->listWidget->item(0));
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
            axis_y->setTickCount(8);
            axis_y->setGridLineVisible(false);
            new_chart->addAxis(axis_y, Qt::AlignLeft);
            raw->attachAxis(axis_y);

            MathEngine math;
            auto smoothed = math.smooth(loader.data);

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
            new_chart->legend()->setPos(440, 20);

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
        marker1->setPos(scene_pos);
        marker1->setVisible(true);
        marker2->setVisible(false);
    } 
    else if (state == 2) 
    {
        marker2->setPos(scene_pos);
        marker2->setVisible(true);
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

        if (key == Qt::Key_Escape) 
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
                if (marker1) marker1->setVisible(false);
                if (marker2) marker2->setVisible(false);
                if (p1_left) p1_left->setVisible(false);
                if (p1_right) p1_right->setVisible(false);
                if (p2_left) p2_left->setVisible(false);
                if (p2_right) p2_right->setVisible(false);
                return true;
            }

            if (state == 11) 
            {
                state = 1; 
                p1_left->setVisible(false); p1_right->setVisible(false);
                return true;
            } 
            else if (state == 3) 
            {
                state = 1; 
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
                    QPointF center_val = chart->mapToValue(center->pos(), smooth_series);
                    double delta = std::abs(value_pos.x() - center_val.x());

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
                    state = 1;
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

    QPointF center_val = chart->mapToValue(center->pos(), smooth_series);
    double delta = (peak == 1) ? p1_dx : p2_dx;

    QPointF left_val(center_val.x() - delta, 0);
    QPointF right_val(center_val.x() + delta, 0);

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

    for (const auto& pair : measurements) 
        file << pair.first << "," << pair.second.p1_x << "," << pair.second.p1_width << "," << pair.second.p2_x << "," << pair.second.p2_width << "," << pair.second.last_state << "\n";
}

void PeaksPeaksPeaks::save_measurement()
{
    if ((state == 0 || state == 3) && !current_file.empty() && chart && smooth_series && marker1) 
    {
        PeakMeasurement m;
        m.last_state = state;
        m.p1_x = chart->mapToValue(marker1->pos(), smooth_series).x();
        m.p1_width = p1_dx;
        
        if (state == 0 && marker2) 
        {
            m.p2_x = chart->mapToValue(marker2->pos(), smooth_series).x();
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
    }
    update_info();
}

void PeaksPeaksPeaks::update_info()
{
    double x1 = 0.0, w1 = 0.0;
    double x2 = 0.0, w2 = 0.0;

    if (state != 1 && chart && smooth_series && marker1) 
    {
        x1 = chart->mapToValue(marker1->pos(), smooth_series).x();
        w1 = p1_dx * 2.0;
    } 
    else if (state == 1 && marker1 && marker1->isVisible()) 
    {
        x1 = chart->mapToValue(marker1->pos(), smooth_series).x();
        w1 = 0.0;
    }

    if ((state == 0 || state == 22) && chart && smooth_series && marker2) 
    {
        x2 = chart->mapToValue(marker2->pos(), smooth_series).x();
        w2 = p2_dx * 2.0;
    } 
    else if (state == 2 && marker2 && marker2->isVisible()) 
    {
        x2 = chart->mapToValue(marker2->pos(), smooth_series).x();
        w2 = 0.0;
    }

    ui->label_p1_info->setText(QString("● %1 nm").arg(x1, 0, 'f', 2));
    ui->label_p1_width->setText(QString("%1 nm").arg(w1, 0, 'f', 1));
    ui->label_p2_info->setText(QString("■ %1 nm").arg(x2, 0, 'f', 2));
    ui->label_p2_width->setText(QString("%1 nm").arg(w2, 0, 'f', 1));
}
