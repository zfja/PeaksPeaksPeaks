#include "peakspeakspeaks.h"
#include "FileManager.h"   
#include <QApplication>
#include <iostream>        

int main(int argc, char *argv[])
{
    FileManager manager;
    
    std::string testPath = "/Users/zofia/Desktop/PeaksPeaksPeaks/"; 
    
    manager.load(testPath, -8, -5);
    
    std::cout << "--- ZNALEZIONE PLIKI ---" << std::endl;
    for (const auto& f : manager.files) {
        std::cout << "Oryginal: " << f.file_name << " | Pokaz: " << f.display_name << " | Wartosc: " << f.val << std::endl;
    }
    std::cout << "------------------------" << std::endl;

    // QApplication a(argc, argv);
    // PeaksPeaksPeaks w;
    // w.show();
    // return QApplication::exec();
}