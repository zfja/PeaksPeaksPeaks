#include "peakspeakspeaks.h"
#include "./ui_peakspeakspeaks.h"
#include "FileManager.h"      
#include "SpectrumLoader.h"   
#include <iostream>        
#include <QListWidget>
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


PeaksPeaksPeaks::PeaksPeaksPeaks(QWidget *parent) : QMainWindow(parent), ui(new Ui::PeaksPeaksPeaks)
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


    connect(ui->settingsButton, &QPushButton::clicked, this, [=]() 
    {
        QDialog settingsDialog(this);
        settingsDialog.setWindowTitle("Chart Settings");
        settingsDialog.setMinimumWidth(250);

        QVBoxLayout* layout = new QVBoxLayout(&settingsDialog);

        // --- SEKCJA KOLORU ---
        QPushButton* chose_color = new QPushButton("Change marker 1 colour", &settingsDialog);
        layout->addWidget(chose_color);

        connect(chose_color, &QPushButton::clicked, this, [&]() 
        {
            QColor new_color = QColorDialog::getColor(chart_color1, &settingsDialog, "Choose colour");
            
            if (new_color.isValid()) {
                chart_color1 = new_color; 

                if (ui->graphView->chart() && !ui->graphView->chart()->series().isEmpty()) {
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
        
        layout->addSpacing(10);
        
        // --- SEKCJA OPISÓW OSI ---
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

        // --- PRZYCISK ZAPISZ ---
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
                
                if (!current_chart->axes(Qt::Horizontal).isEmpty()) {
                    current_chart->axes(Qt::Horizontal).first()->setTitleText(x_title);
                }
                
                if (!current_chart->axes(Qt::Vertical).isEmpty()) {
                    current_chart->axes(Qt::Vertical).first()->setTitleText(y_title);
                }
            }

            settingsDialog.accept();
        });


        // (W przyszłości tutaj będziesz dodawać kolejne przyciski/suwaki do layoutu)


        settingsDialog.exec();
    });


    FileManager manager;
    std::string path = manager.get_path();
    manager.load(path, -8, -5);

    ui->listWidget->clear();

    for (const auto& f : manager.files) 
    {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(f.display_name));
        item->setData(Qt::UserRole, QString::fromStdString(f.file_name));
        ui->listWidget->addItem(item);
    }

    ui->graphView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphView->setFrameShape(QFrame::NoFrame);

    connect(ui->listWidget, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) 
    {
        std::string file_name = item->data(Qt::UserRole).toString().toStdString();
        std::string full_path = path + "/" + file_name;

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

                for (const auto& point : loader.data)
                    series->append(point.first, point.second);

                QChart* chart = new QChart();
                chart->addSeries(series);

                chart->legend()->hide();

                chart->setBackgroundBrush(Qt::white);
                chart->setBackgroundRoundness(0);
                chart->setMargins(QMargins(0, 0, 0, 0));


                QValueAxis* axisX = new QValueAxis();
                axisX->setTitleText(x_title); 
                axisX->setTickCount(10);              
                axisX->setGridLineVisible(false);                  
                // axisX->setGridLineColor(QColor("#dcdde1"));       
                chart->addAxis(axisX, Qt::AlignBottom);
                series->attachAxis(axisX); 


                QValueAxis* axisY = new QValueAxis();
                axisY->setTitleText(y_title); 
                axisY->setTickCount(8);
                axisY->setGridLineVisible(false);
                // axisY->setGridLineColor(QColor("#dcdde1"));
                chart->addAxis(axisY, Qt::AlignLeft);
                series->attachAxis(axisY);

                ui->graphView->setChart(chart);
                ui->graphView->setRenderHint(QPainter::Antialiasing);
            } 
            else 
                ui->graphView->setChart(new QChart());
        }
        catch (const std::exception& e) {ui->graphView->setChart(new QChart());}

        QFont font = item->font();
        font.setBold(isbroken); 
        item->setFont(font);
        item->setForeground(isbroken ? QColor("#b61818") : Qt::white);
    });

    //DEBUG//
    // std::cout << "Program szuka plikow w folderze: " << path << std::endl;

    // std::cout << "--- ZNALEZIONE PLIKI ---" << std::endl;
    // for (const auto& f : manager.files) 
    // {
    //     std::cout << "Oryginal: " << f.file_name 
    //               << " | Pokaz: " << f.display_name 
    //               << " | Wartosc: " << f.val << std::endl;
    // }
    // std::cout << "------------------------" << std::endl;


    // SpectrumLoader loader;
    // loader.load(path + "/" + manager.files[1].file_name);

    // std::cout << "--- PUNKTY WIDMA ---" << std::endl;
    // for (const auto& d : loader.data)
    // {
    //     std::cout << "x: " << d.first << std::endl;
    // }
    // std::cout << "--------------------" << std::endl;
}

PeaksPeaksPeaks::~PeaksPeaksPeaks()
{
    delete ui;
}