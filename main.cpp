#include "peakspeakspeaks.h"
#include "FileManager.h"
#include "SpectrumLoader.h"   
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
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

    SpectrumLoader loader;
    loader.load(path + "/" + manager.files[1].file_name);

    std::cout << "--- PUNKTY WIDMA ---" << std::endl;
    for (const auto& d :loader.data)
    {
        std::cout << "x: " << d.first << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    PeaksPeaksPeaks w;
    w.show();
    return app.exec();
}