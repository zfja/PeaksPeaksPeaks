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

            if (markerItem)
            {
                delete markerItem;
                markerItem = nullptr;
            }

            markerItem = new QGraphicsEllipseItem();
            markerItem->setRect(-5, -5, 7, 7);
            markerItem->setBrush(Qt::black);
            markerItem->setPen(QPen(Qt::black));
            markerItem->setZValue(1000);
            markerItem->setVisible(false);

            chart->scene()->addItem(markerItem);
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
    delete ui;
}

void PeaksPeaksPeaks::showMarkerAtX(double x)
{
    if (!currentChart || !fitSeries || !markerItem)
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
    markerItem->setPos(scenePos);
    markerItem->setVisible(true);
}


bool PeaksPeaksPeaks::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->listWidget && event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        int currentRow = ui->listWidget->currentRow();

        if (keyEvent->key() == Qt::Key_Down)
        {
            int nextRow = currentRow + 1;
            if (nextRow < ui->listWidget->count())
            {
                ui->listWidget->setCurrentRow(nextRow);
                loadSelectedItem(ui->listWidget->item(nextRow));
            }
            return true;
        }

        if (keyEvent->key() == Qt::Key_Up)
        {
            int prevRow = currentRow - 1;
            if (prevRow >= 0)
            {
                ui->listWidget->setCurrentRow(prevRow);
                loadSelectedItem(ui->listWidget->item(prevRow));
            }
            return true;
        }
    }

    if (obj == ui->graphView->viewport() && event->type() == QEvent::MouseMove)
    {
        if (!currentChart || !fitSeries || !markerItem)
            return false;

        auto* mouseEvent = static_cast<QMouseEvent*>(event);

        QPointF scenePos = ui->graphView->mapToScene(mouseEvent->pos());
        QPointF valuePos = currentChart->mapToValue(scenePos, fitSeries);

        showMarkerAtX(valuePos.x());
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}
