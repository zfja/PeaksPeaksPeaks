#include "peakspeakspeaks.h"
#include "./ui_peakspeakspeaks.h"
#include "FileManager.h"      
#include "SpectrumLoader.h"   
#include <iostream>        
#include <QListWidget>


PeaksPeaksPeaks::PeaksPeaksPeaks(QWidget *parent) : QMainWindow(parent), ui(new Ui::PeaksPeaksPeaks)
{
    ui->setupUi(this);
    this->setStyleSheet(R"(
        QMainWindow {
            background-color: #2b2b2b; /* Ciemne tło całego okna */
        }
        
        QListWidget {
            background-color: #3c3f41; /* Tło samej listy */
            color: #e0e0e0;            /* Jasnoszary tekst */
            border: 1px solid #555555; /* Delikatna ramka */
            padding: 5px;
            font-size: 14px;
        }

        QListWidget::item {
            padding: 2px; /* Odstępy między plikami na liście */
        }

        QListWidget::item:selected {
            background-color: #2980b9; /* Niebieskie tło po kliknięciu */
            color: #ffffff;            /* Biały tekst po kliknięciu */
            border-radius: 3px;
        }
        
        QListWidget::item:hover {
            background-color: #4f5356; /* Podświetlenie, gdy najedziesz myszką! */
        }
    )");

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

    connect(ui->listWidget, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) 
    {
        std::string file_name = item->data(Qt::UserRole).toString().toStdString();
        std::string full_path = path + "/" + file_name;

        //DEBUG//
        std::cout << "\n[UI] Zaznaczono na liscie: " << item->text().toStdString() << std::endl;
        
        SpectrumLoader loader;
        bool isbroken = true;
        try 
        {
            loader.load(full_path);

            if (!loader.data.empty()) 
            {
                isbroken = false;
                //DEBUG//
                std::cout << "[UI] Sukces! Wczytano punktow: " << loader.data.size() << std::endl; 
            } 
            //DEBUG//
            else 
            {
                std::cout << "[UI] Plik pusty lub uszkodzony." << std::endl;
            }            
        }
        catch (const std::exception& e) 
        {
            //DEBUG//
            std::cout << "[UI] Blad wczytywania: " << e.what() << std::endl;
        }

        QFont font = item->font();
        font.setBold(isbroken); 
        item->setFont(font);
        item->setForeground(isbroken ? QColor("#b61818") : Qt::white);
    });





    //DEBUG//
    std::cout << "Program szuka plikow w folderze: " << path << std::endl;

    std::cout << "--- ZNALEZIONE PLIKI ---" << std::endl;
    for (const auto& f : manager.files) 
    {
        std::cout << "Oryginal: " << f.file_name 
                  << " | Pokaz: " << f.display_name 
                  << " | Wartosc: " << f.val << std::endl;
    }
    std::cout << "------------------------" << std::endl;


    SpectrumLoader loader;
    loader.load(path + "/" + manager.files[1].file_name);

    std::cout << "--- PUNKTY WIDMA ---" << std::endl;
    for (const auto& d : loader.data)
    {
        std::cout << "x: " << d.first << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

PeaksPeaksPeaks::~PeaksPeaksPeaks()
{
    delete ui;
}