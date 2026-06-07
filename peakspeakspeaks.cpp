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
#include <map>
#include <string>
#include <fstream>
#include <sstream>



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

    std::string file_name = item->data(Qt::UserRole).toString().toStdString();
    std::string full_path = basePath.toStdString() + "/" + file_name;

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
            linePen.setStyle(Qt::DashLine); // Przerywana linia
            linePen.setWidth(2);

            p1_lineLeft = new QGraphicsLineItem(); p1_lineLeft->setPen(linePen); p1_lineLeft->setZValue(999); p1_lineLeft->setVisible(false); chart->scene()->addItem(p1_lineLeft);
            p1_lineRight = new QGraphicsLineItem(); p1_lineRight->setPen(linePen); p1_lineRight->setZValue(999); p1_lineRight->setVisible(false); chart->scene()->addItem(p1_lineRight);
            
            p2_lineLeft = new QGraphicsLineItem(); p2_lineLeft->setPen(linePen); p2_lineLeft->setZValue(999); p2_lineLeft->setVisible(false); chart->scene()->addItem(p2_lineLeft);
            p2_lineRight = new QGraphicsLineItem(); p2_lineRight->setPen(linePen); p2_lineRight->setZValue(999); p2_lineRight->setVisible(false); chart->scene()->addItem(p2_lineRight);
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
        // Kółko jeździ
        markerItem->setPos(scenePos);
        markerItem->setVisible(true);
        markerItem2->setVisible(false);
    } 
    else if (selectionState == 2) {
        // Kółko zamrożone (nie rusza się), Kwadrat jeździ
        markerItem2->setPos(scenePos);
        markerItem2->setVisible(true);
    }
}


bool PeaksPeaksPeaks::eventFilter(QObject *obj, QEvent *event)
{
    // --- OBSŁUGA KLAWIATURY (GLOBALNA) ---
    if (event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        // 1. Nawigacja po liście
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

        // 2. Strzałki (Lewo / Prawo) - regulacja szerokości z klawiatury
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

        // 3. ENTER (Zatwierdzanie szerokości)
        if (key == Qt::Key_Enter || key == Qt::Key_Return) {
            if (selectionState == 11) {
                selectionState = 2; // Zakończ P1 -> Aktywuj Kwadrat
                return true;
            } else if (selectionState == 22) {
                selectionState = 0; // Zakończ P2 -> Koniec wyboru
                return true;
            }
        }

        // 4. ESC (Logika cofania)
        if (key == Qt::Key_Escape) {
            if (selectionState == 11) {
                selectionState = 1; // Znikają linie P1 -> Kółko ruchome
                p1_lineLeft->setVisible(false); p1_lineRight->setVisible(false);
                return true;
            } else if (selectionState == 3) {
                selectionState = 1; // Z Zamrożonego Kółka -> Kółko ruchome
                return true;
            } else if (selectionState == 22) {
                selectionState = 2; // Znikają linie P2 -> Kwadrat ruchomy
                p2_lineLeft->setVisible(false); p2_lineRight->setVisible(false);
                return true;
            } else if (selectionState == 0) {
                selectionState = 2; // Z Gotowego (0) -> Kwadrat ruchomy (2)
                p2_lineLeft->setVisible(false); p2_lineRight->setVisible(false);
                return true;
            } else if (selectionState == 2) {
                selectionState = 3; // Z Aktywnego Kwadratu -> Anulowanie kwadratu
                if (markerItem2) markerItem2->setVisible(false);
                return true;
            }
        }
    }

    // --- OBSŁUGA MYSZY NA WYKRESIE ---
    if (obj == ui->graphView->viewport())
    {
        if (event->type() == QEvent::MouseMove)
        {
            if (!currentChart || !fitSeries) return false;

            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF scenePos = ui->graphView->mapToScene(mouseEvent->pos());
            QPointF valuePos = currentChart->mapToValue(scenePos, fitSeries);

            // Śledzenie Kółka / Kwadratu
            if (selectionState == 1 || selectionState == 2) 
            {
                showMarkerAtX(valuePos.x());
            }
            // Zmiana SZEROKOŚCI ruchem myszki
            else if (selectionState == 11 || selectionState == 22) 
            {
                QGraphicsItem* centerMarker = (selectionState == 11) ? (QGraphicsItem*)markerItem : (QGraphicsItem*)markerItem2;
                
                if (centerMarker) {
                    QPointF centerVal = currentChart->mapToValue(centerMarker->pos(), fitSeries);
                    // Obliczamy odległość myszki (w osi X) od środkowego znacznika
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
                    selectionState = 11; // Zamroź środek P1, aktywuj szerokość
                    p1_deltaX = 0.0;
                    p1_lineLeft->setVisible(true); p1_lineRight->setVisible(true);
                    updateWidthLines(1);
                    return true;
                }
                else if (selectionState == 11) {
                    selectionState = 2;  // Zamroź szerokość P1, przejdź do środka P2 (Zamiast Enter)
                    return true;
                }
                else if (selectionState == 2) {
                    selectionState = 22; // Zamroź środek P2, aktywuj szerokość
                    p2_deltaX = 0.0;
                    p2_lineLeft->setVisible(true); p2_lineRight->setVisible(true);
                    updateWidthLines(2);
                    return true;
                }
                else if (selectionState == 22) {
                    selectionState = 0;  // Zamroź szerokość P2, zakończ (Zamiast Enter)
                    return true;
                }
                else if (selectionState == 0 || selectionState == 3) {
                    // Start od nowa, jeśli klikniesz po wszystkim
                    selectionState = 1;
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void PeaksPeaksPeaks::updateWidthLines(int peakNum)
{
    if (!currentChart || !fitSeries) return;

    QGraphicsItem* centerMarker = (peakNum == 1) ? (QGraphicsItem*)markerItem : (QGraphicsItem*)markerItem2;
    if (!centerMarker || !centerMarker->isVisible()) return;

    // Pobieramy wartość X środka piku
    QPointF centerVal = currentChart->mapToValue(centerMarker->pos(), fitSeries);
    double delta = (peakNum == 1) ? p1_deltaX : p2_deltaX;

    // Wyznaczamy pozycje X dla lewej i prawej linii w jednostkach wykresu
    QPointF leftVal(centerVal.x() - delta, 0);
    QPointF rightVal(centerVal.x() + delta, 0);

    // Mapujemy z powrotem na pozycję pikselową sceny
    double sceneLeftX = currentChart->mapToPosition(leftVal, fitSeries).x();
    double sceneRightX = currentChart->mapToPosition(rightVal, fitSeries).x();

    // Pobieramy granice rysowania (żeby linie były na całą wysokość osi Y)
    QRectF plotArea = currentChart->plotArea();

    if (peakNum == 1) {
        p1_lineLeft->setLine(sceneLeftX, plotArea.top(), sceneLeftX, plotArea.bottom());
        p1_lineRight->setLine(sceneRightX, plotArea.top(), sceneRightX, plotArea.bottom());
    } else {
        p2_lineLeft->setLine(sceneLeftX, plotArea.top(), sceneLeftX, plotArea.bottom());
        p2_lineRight->setLine(sceneRightX, plotArea.top(), sceneRightX, plotArea.bottom());
    }
}