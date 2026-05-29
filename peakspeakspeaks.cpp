#include "peakspeakspeaks.h"
#include "./ui_peakspeakspeaks.h"
#include "FileManager.h"      
#include "SpectrumLoader.h"   
#include <iostream>        


PeaksPeaksPeaks::PeaksPeaksPeaks(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PeaksPeaksPeaks)
{
    // Ta linijka buduje Twój wyklikany interfejs
    ui->setupUi(this);

    // --- TUTAJ WKLEJAMY KOD Z MAINA ---
    FileManager manager;
    
    std::string path = manager.get_path();
    std::cout << "Program szuka plikow w folderze: " << path << std::endl;

    manager.load(path, -8, -5);
    
    std::cout << "--- ZNALEZIONE PLIKI ---" << std::endl;
    for (const auto& f : manager.files) 
    {
        std::cout << "Oryginal: " << f.file_name 
                  << " | Pokaz: " << f.display_name 
                  << " | Wartosc: " << f.val << std::endl;
    }
    std::cout << "------------------------" << std::endl;

    // Zabezpieczenie przed błędem, gdyby plików było mniej niż 2
    if (manager.files.size() > 1) 
    {
        SpectrumLoader loader;
        loader.load(path + "/" + manager.files[1].file_name);

        std::cout << "--- PUNKTY WIDMA ---" << std::endl;
        for (const auto& d : loader.data)
        {
            std::cout << "x: " << d.first << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
}

PeaksPeaksPeaks::~PeaksPeaksPeaks()
{
    delete ui;
}