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
    basePath = QString::fromStdString(path);

    csvFilePath = basePath + "/data.csv";
    loadCSV();

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
        QDialog settingsDialog(this);
        settingsDialog.setWindowTitle("Chart Settings");
        settingsDialog.setMinimumWidth(250);

        QVBoxLayout* layout = new QVBoxLayout(&settingsDialog);

        QPushButton* chose_color1 = new QPushButton("Change marker 1 colour", &settingsDialog);
        layout->addWidget(chose_color1);
        connect(chose_color1, &QPushButton::clicked, this, [&]()
        {
            QColor new_color = QColorDialog::getColor(chart_color1, &settingsDialog, "Choose colour");

            if (new_color.isValid())
            {
                chart_color1 = new_color;

                if (ui->graphView->chart() && !ui->graphView->chart()->series().isEmpty())
                {
                    QLineSeries* currentSeries = qobject_cast<QLineSeries*>(ui->graphView->chart()->series().first());
                    if (currentSeries)
                    {
                        QPen currentPen = currentSeries->pen();
                        currentPen.setColor(chart_color1);
                        currentSeries->setPen(currentPen);
                    }
                }
            }
        });

        QPushButton* chose_color2 = new QPushButton("Change marker 2 colour", &settingsDialog);
        layout->addWidget(chose_color2);
        connect(chose_color2, &QPushButton::clicked, this, [&]()
        {
            QColor new_color = QColorDialog::getColor(chart_color2, &settingsDialog, "Choose colour");

            if (new_color.isValid())
            {
                chart_color2 = new_color;

                if (ui->graphView->chart() && ui->graphView->chart()->series().size() > 1)
                {
                    QLineSeries* currentSeries = qobject_cast<QLineSeries*>(ui->graphView->chart()->series().last());

                    if (currentSeries)
                    {
                        QPen currentPen = currentSeries->pen();
                        currentPen.setColor(chart_color2);
                        currentSeries->setPen(currentPen);
                    }
                }
            }
        });

        layout->addSpacing(10);

        QLabel* label_x = new QLabel("X-axis Title:", &settingsDialog);
        layout->addWidget(label_x);
        QLineEdit* input_x = new QLineEdit(&settingsDialog);
        input_x->setText(x_title);
        layout->addWidget(input_x);

        QLabel* label_y = new QLabel("Y-axis Title:", &settingsDialog);
        layout->addWidget(label_y);
        QLineEdit* input_y = new QLineEdit(&settingsDialog);
        input_y->setText(y_title);
        layout->addWidget(input_y);

        layout->addSpacing(15);

        QPushButton* save_button = new QPushButton("Save Changes", &settingsDialog);
        save_button->setStyleSheet("font-weight: bold;");
        layout->addWidget(save_button);

        connect(save_button, &QPushButton::clicked, &settingsDialog, [&]()
        {
            x_title = input_x->text();
            y_title = input_y->text();

            if (ui->graphView->chart())
            {
                QChart* current_chart = ui->graphView->chart();

                if (!current_chart->axes(Qt::Horizontal).isEmpty())
                    current_chart->axes(Qt::Horizontal).first()->setTitleText(x_title);

                if (!current_chart->axes(Qt::Vertical).isEmpty())
                    current_chart->axes(Qt::Vertical).first()->setTitleText(y_title);
            }

            settingsDialog.accept();
        });

        settingsDialog.exec();
    });

    connect(ui->listWidget, &QListWidget::itemClicked, this, [=](QListWidgetItem* item)
    {
        loadSelectedItem(item);
    });

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
        loadSelectedItem(ui->listWidget->item(0));
    }
}

void PeaksPeaksPeaks::loadSelectedItem(QListWidgetItem* item)
{
    if (!item)
        return;

    saveCurrentMeasurement();
    std::string file_name = item->data(Qt::UserRole).toString().toStdString();
    std::string full_path = basePath.toStdString() + "/" + file_name;

    currentLoadedFile = file_name;

    SpectrumLoader loader;
    bool isbroken = true;

    try
    {
        loader.load(full_path);

        if (!loader.data.empty())
        {
            isbroken = false;

            QLineSeries* series = new QLineSeries();
            QPen pen(chart_color1);
            pen.setWidth(2);
            series->setPen(pen);
            series->setName("raw data");

            for (const auto& point : loader.data)
                series->append(point.first, point.second);

            QChart* chart = new QChart();
            chart->addSeries(series);

            chart->setBackgroundBrush(Qt::white);
            chart->setBackgroundRoundness(0);
            chart->setMargins(QMargins(0, 0, 0, 0));

            QValueAxis* axisX = new QValueAxis();
            axisX->setTitleText(x_title);
            axisX->setTickCount(10);
            axisX->setGridLineVisible(false);
            chart->addAxis(axisX, Qt::AlignBottom);
            series->attachAxis(axisX);

            QValueAxis* axisY = new QValueAxis();
            axisY->setTitleText(y_title);
            axisY->setTickCount(8);
            axisY->setGridLineVisible(false);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            MathEngine math;
            auto smoothed_data = math.smooth(loader.data);

            QLineSeries* smooth_series = new QLineSeries();
            smooth_series->setName("smooth data");
            smooth_series->setPen(QPen(chart_color2, 2));

            for (const auto& point : smoothed_data)
                smooth_series->append(point.first, point.second);

            chart->addSeries(smooth_series);
            smooth_series->attachAxis(axisX);
            smooth_series->attachAxis(axisY);

            fitSeries = smooth_series;
            currentChart = chart;

            chart->legend()->setVisible(true);
            chart->legend()->detachFromChart();
            chart->legend()->setBackgroundVisible(true);
            chart->legend()->setLabelColor(QColor("#2b2b2b"));

            QFont legend_font = chart->legend()->font();
            legend_font.setPixelSize(12);
            chart->legend()->setFont(legend_font);

            chart->legend()->setMinimumSize(120, 70);
            chart->legend()->resize(120, 60);
            chart->legend()->setPos(440, 20);

            ui->graphView->setChart(chart);
            ui->graphView->setRenderHint(QPainter::Antialiasing);

            if (markerItem) { delete markerItem; markerItem = nullptr; }
            if (markerItem2) { delete markerItem2; markerItem2 = nullptr; }

            if (p1_lineLeft) { delete p1_lineLeft; p1_lineLeft = nullptr; }
            if (p1_lineRight) { delete p1_lineRight; p1_lineRight = nullptr; }
            if (p2_lineLeft) { delete p2_lineLeft; p2_lineLeft = nullptr; }
            if (p2_lineRight) { delete p2_lineRight; p2_lineRight = nullptr; }

            selectionState = 1;
            p1_deltaX = 0.0;
            p2_deltaX = 0.0;

            markerItem = new QGraphicsEllipseItem();
            markerItem->setRect(-4, -4, 7, 7);
            markerItem->setBrush(Qt::black);
            markerItem->setPen(QPen(Qt::black));
            markerItem->setZValue(1000);
            markerItem->setVisible(false);
            chart->scene()->addItem(markerItem);

            markerItem2 = new QGraphicsRectItem();
            markerItem2->setRect(-4, -4, 7, 7);
            markerItem2->setBrush(Qt::black);
            markerItem2->setPen(QPen(Qt::black));
            markerItem2->setZValue(1000);
            markerItem2->setVisible(false);
            chart->scene()->addItem(markerItem2);

            QPen linePen(Qt::darkGray);
            linePen.setStyle(Qt::DashLine);
            linePen.setWidth(2);

            p1_lineLeft = new QGraphicsLineItem(); p1_lineLeft->setPen(linePen); p1_lineLeft->setZValue(999); p1_lineLeft->setVisible(false); chart->scene()->addItem(p1_lineLeft);
            p1_lineRight = new QGraphicsLineItem(); p1_lineRight->setPen(linePen); p1_lineRight->setZValue(999); p1_lineRight->setVisible(false); chart->scene()->addItem(p1_lineRight);
            
            p2_lineLeft = new QGraphicsLineItem(); p2_lineLeft->setPen(linePen); p2_lineLeft->setZValue(999); p2_lineLeft->setVisible(false); chart->scene()->addItem(p2_lineLeft);
            p2_lineRight = new QGraphicsLineItem(); p2_lineRight->setPen(linePen); p2_lineRight->setZValue(999); p2_lineRight->setVisible(false); chart->scene()->addItem(p2_lineRight);
            
            restoreMeasurement(currentLoadedFile);
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
    font.setBold(isbroken);
    item->setFont(font);
    item->setForeground(isbroken ? QColor("#b61818") : Qt::white);
}

PeaksPeaksPeaks::~PeaksPeaksPeaks()
{
    delete markerItem;
    delete markerItem2;
    delete ui;
    delete p1_lineLeft;
    delete p1_lineRight;
    delete p2_lineLeft;
    delete p2_lineRight;
}

void PeaksPeaksPeaks::showMarkerAtX(double x)
{
    if (!currentChart || !fitSeries || !markerItem || !markerItem2)
        return;

    const auto points = fitSeries->points();
    if (points.isEmpty())
        return;

    QPointF bestPoint = points.first();
    double bestDist = std::abs(points.first().x() - x);

    for (const QPointF& p : points)
    {
        double dist = std::abs(p.x() - x);
        if (dist < bestDist)
        {
            bestDist = dist;
            bestPoint = p;
        }
    }

    QPointF scenePos = currentChart->mapToPosition(bestPoint, fitSeries);
    
    if (selectionState == 1) {
        markerItem->setPos(scenePos);
        markerItem->setVisible(true);
        markerItem2->setVisible(false);
    } 
    else if (selectionState == 2) {
        markerItem2->setPos(scenePos);
        markerItem2->setVisible(true);
    }
    updateInfoPanel();
}

bool PeaksPeaksPeaks::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        if (obj == ui->listWidget && (key == Qt::Key_Up || key == Qt::Key_Down))
        {
            int currentRow = ui->listWidget->currentRow();
            int nextRow = (key == Qt::Key_Down) ? currentRow + 1 : currentRow - 1;
            
            if (nextRow >= 0 && nextRow < ui->listWidget->count()) {
                ui->listWidget->setCurrentRow(nextRow);
                loadSelectedItem(ui->listWidget->item(nextRow));
            }
            return true;
        }

        if (key == Qt::Key_Left || key == Qt::Key_Right) {
            double change = (key == Qt::Key_Right) ? arrowStep : -arrowStep;
            if (selectionState == 11) {
                p1_deltaX = std::max(0.0, p1_deltaX + change);
                updateWidthLines(1);
                return true;
            } else if (selectionState == 22) {
                p2_deltaX = std::max(0.0, p2_deltaX + change);
                updateWidthLines(2);
                return true;
            }
        }

        if (key == Qt::Key_Enter || key == Qt::Key_Return) {
            if (selectionState == 11) {
                selectionState = 2; 
                return true;
            } else if (selectionState == 22) {
                selectionState = 0; 
                return true;
            }
        }

        if (key == Qt::Key_Escape) {
            if (selectionState == 11) {
                selectionState = 1; 
                p1_lineLeft->setVisible(false); p1_lineRight->setVisible(false);
                return true;
            } else if (selectionState == 3) {
                selectionState = 1; 
                return true;
            } else if (selectionState == 22) {
                selectionState = 2; 
                p2_lineLeft->setVisible(false); p2_lineRight->setVisible(false);
                return true;
            } else if (selectionState == 0) {
                selectionState = 2;
                p2_lineLeft->setVisible(false); p2_lineRight->setVisible(false);
                return true;
            } else if (selectionState == 2) {
                selectionState = 3;
                if (markerItem2) markerItem2->setVisible(false);
                return true;
            }
        }

        if (key == Qt::Key_Escape) 
        {
            
            if (keyEvent->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) {
                if (!currentLoadedFile.empty()) {
                    measurements.erase(currentLoadedFile); 
                    saveCSV(); 
                }
                
                selectionState = 1;
                p1_deltaX = 0.0; p2_deltaX = 0.0;
                if (markerItem) markerItem->setVisible(false);
                if (markerItem2) markerItem2->setVisible(false);
                if (p1_lineLeft) p1_lineLeft->setVisible(false);
                if (p1_lineRight) p1_lineRight->setVisible(false);
                if (p2_lineLeft) p2_lineLeft->setVisible(false);
                if (p2_lineRight) p2_lineRight->setVisible(false);
                return true;
            }
        }
    }

    if (obj == ui->graphView->viewport())
    {
        if (event->type() == QEvent::MouseMove)
        {
            if (!currentChart || !fitSeries) return false;

            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF scenePos = ui->graphView->mapToScene(mouseEvent->pos());
            QPointF valuePos = currentChart->mapToValue(scenePos, fitSeries);

            if (selectionState == 1 || selectionState == 2) 
            {
                showMarkerAtX(valuePos.x());
            }
            else if (selectionState == 11 || selectionState == 22) 
            {
                QGraphicsItem* centerMarker = (selectionState == 11) ? (QGraphicsItem*)markerItem : (QGraphicsItem*)markerItem2;
                
                if (centerMarker) {
                    QPointF centerVal = currentChart->mapToValue(centerMarker->pos(), fitSeries);
                    double delta = std::abs(valuePos.x() - centerVal.x());

                    if (selectionState == 11) {
                        p1_deltaX = delta;
                        updateWidthLines(1);
                    } else {
                        p2_deltaX = delta;
                        updateWidthLines(2);
                    }
                }
            }
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress)
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                if (selectionState == 1) {
                    selectionState = 11; 
                    p1_deltaX = 0.0;
                    p1_lineLeft->setVisible(true); p1_lineRight->setVisible(true);
                    updateWidthLines(1);
                    updateInfoPanel();
                    return true;
                }
                else if (selectionState == 11) {
                    selectionState = 2; 
                    return true;
                }
                else if (selectionState == 2) {
                    selectionState = 22; 
                    p2_deltaX = 0.0;
                    p2_lineLeft->setVisible(true); p2_lineRight->setVisible(true);
                    updateWidthLines(2);
                    updateInfoPanel();
                    return true;
                }
                else if (selectionState == 22) {
                    selectionState = 0; 
                    return true;
                }
                else if (selectionState == 0 || selectionState == 3) {
                    selectionState = 1;
                    return true;
                }
            }
        }
    }

    updateInfoPanel();
    return QMainWindow::eventFilter(obj, event);
}

void PeaksPeaksPeaks::updateWidthLines(int peakNum)
{
    if (!currentChart || !fitSeries) return;

    QGraphicsItem* centerMarker = (peakNum == 1) ? (QGraphicsItem*)markerItem : (QGraphicsItem*)markerItem2;
    if (!centerMarker || !centerMarker->isVisible()) return;

    QPointF centerVal = currentChart->mapToValue(centerMarker->pos(), fitSeries);
    double delta = (peakNum == 1) ? p1_deltaX : p2_deltaX;

    QPointF leftVal(centerVal.x() - delta, 0);
    QPointF rightVal(centerVal.x() + delta, 0);

    double sceneLeftX = currentChart->mapToPosition(leftVal, fitSeries).x();
    double sceneRightX = currentChart->mapToPosition(rightVal, fitSeries).x();

    QRectF plotArea = currentChart->plotArea();

    if (peakNum == 1) {
        p1_lineLeft->setLine(sceneLeftX, plotArea.top(), sceneLeftX, plotArea.bottom());
        p1_lineRight->setLine(sceneRightX, plotArea.top(), sceneRightX, plotArea.bottom());
    } else {
        p2_lineLeft->setLine(sceneLeftX, plotArea.top(), sceneLeftX, plotArea.bottom());
        p2_lineRight->setLine(sceneRightX, plotArea.top(), sceneRightX, plotArea.bottom());
    }
    updateInfoPanel();

}

void PeaksPeaksPeaks::loadCSV()
{
    measurements.clear();
    std::ifstream file(csvFilePath.toStdString());
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string fname, p1x_str, p1w_str, p2x_str, p2w_str, state_str;
        
        if (std::getline(ss, fname, ',') &&
            std::getline(ss, p1x_str, ',') && std::getline(ss, p1w_str, ',') &&
            std::getline(ss, p2x_str, ',') && std::getline(ss, p2w_str, ',')) 
        {
            PeakMeasurement m;
            m.p1_x = std::stod(p1x_str);
            m.p1_width = std::stod(p1w_str);
            m.p2_x = std::stod(p2x_str);
            m.p2_width = std::stod(p2w_str);
            
            if (std::getline(ss, state_str, ',')) {
                m.final_state = std::stoi(state_str);
            } else {
                m.final_state = 0;
            }
            
            measurements[fname] = m;
        }
    }
}

void PeaksPeaksPeaks::saveCSV()
{
    std::ofstream file(csvFilePath.toStdString());
    if (!file.is_open()) return;

    for (const auto& pair : measurements) {
        file << pair.first << "," 
             << pair.second.p1_x << "," << pair.second.p1_width << ","
             << pair.second.p2_x << "," << pair.second.p2_width << ","
             << pair.second.final_state << "\n"; // Zapisujemy stan na końcu linii
    }
}

void PeaksPeaksPeaks::saveCurrentMeasurement()
{
    if ((selectionState == 0 || selectionState == 3) && !currentLoadedFile.empty() && currentChart && fitSeries && markerItem) 
    {
        PeakMeasurement m;
        m.final_state = selectionState;
        m.p1_x = currentChart->mapToValue(markerItem->pos(), fitSeries).x();
        m.p1_width = p1_deltaX;
        
        if (selectionState == 0 && markerItem2) {
            m.p2_x = currentChart->mapToValue(markerItem2->pos(), fitSeries).x();
            m.p2_width = p2_deltaX;
        } else {
            m.p2_x = 0.0;
            m.p2_width = 0.0;
        }
        
        measurements[currentLoadedFile] = m;
        saveCSV(); 
    }
}

void PeaksPeaksPeaks::setMarkerToX(QGraphicsItem* marker, double x)
{
    if (!currentChart || !fitSeries) return;
    const auto points = fitSeries->points();
    if (points.isEmpty()) return;

    QPointF bestPoint = points.first();
    double bestDist = std::abs(points.first().x() - x);

    for (const QPointF& p : points) {
        double dist = std::abs(p.x() - x);
        if (dist < bestDist) { bestDist = dist; bestPoint = p; }
    }
    marker->setPos(currentChart->mapToPosition(bestPoint, fitSeries));
    marker->setVisible(true);
}

void PeaksPeaksPeaks::restoreMeasurement(const std::string& fileName)
{
    if (measurements.count(fileName)) {
        PeakMeasurement m = measurements[fileName];
        selectionState = m.final_state; 

        p1_deltaX = m.p1_width;
        setMarkerToX(markerItem, m.p1_x);
        p1_lineLeft->setVisible(true); 
        p1_lineRight->setVisible(true);
        updateWidthLines(1);
        
        if (selectionState == 0) {
            p2_deltaX = m.p2_width;
            setMarkerToX(markerItem2, m.p2_x);
            p2_lineLeft->setVisible(true); 
            p2_lineRight->setVisible(true);
            updateWidthLines(2);
        } else {
            p2_deltaX = 0.0;
            if(markerItem2) markerItem2->setVisible(false);
            if(p2_lineLeft) p2_lineLeft->setVisible(false);
            if(p2_lineRight) p2_lineRight->setVisible(false);
        }
    } else {
        selectionState = 1; 
        p1_deltaX = 0.0;
        p2_deltaX = 0.0;
    }
    updateInfoPanel();
}

void PeaksPeaksPeaks::updateInfoPanel()
{
    double x1 = 0.0, w1 = 0.0;
    double x2 = 0.0, w2 = 0.0;

    if (selectionState != 1 && currentChart && fitSeries && markerItem) {
        x1 = currentChart->mapToValue(markerItem->pos(), fitSeries).x();
        w1 = p1_deltaX * 2.0;
    } else if (selectionState == 1 && markerItem && markerItem->isVisible()) {
        x1 = currentChart->mapToValue(markerItem->pos(), fitSeries).x();
        w1 = 0.0;
    }

    if ((selectionState == 0 || selectionState == 22) && currentChart && fitSeries && markerItem2) {
        x2 = currentChart->mapToValue(markerItem2->pos(), fitSeries).x();
        w2 = p2_deltaX * 2.0;
    } else if (selectionState == 2 && markerItem2 && markerItem2->isVisible()) {
        x2 = currentChart->mapToValue(markerItem2->pos(), fitSeries).x();
        w2 = 0.0;
    }

    ui->label_p1_info->setText(QString("● %1 nm").arg(x1, 0, 'f', 2));
    ui->label_p1_width->setText(QString("%1 nm").arg(w1, 0, 'f', 1));
    ui->label_p2_info->setText(QString("■ %1 nm").arg(x2, 0, 'f', 2));
    ui->label_p2_width->setText(QString("%1 nm").arg(w2, 0, 'f', 1));
}