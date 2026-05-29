#include "FileManager.h"


void FileManager::load(const std::string& directory, int start_index, int end_index) 
{
    files.clear();

    for (const auto& entry : std::filesystem::directory_iterator(directory)) 
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") 
        {  
            std::string file_name = entry.path().filename().string();
            std::string display_name = file_name;
            double val = 0.0;

            int start = file_name.length() + start_index; 
            int length = end_index - start_index + 1;

            if (start >= 0 && start + length <= file_name.length()) 
            {
                display_name = file_name.substr(start, length);
                
                try {val = std::stod(display_name);}
                catch (...) {val = 0.0;}
            }

            files.push_back({file_name, display_name, val});
        }
    }
    this->sort();
}

std::string FileManager::get_path()
{
    QString app_directory = QApplication::applicationDirPath();
    
    if (app_directory.endsWith("MacOS")) 
    {
        QDir dir(app_directory);
        dir.cdUp();
        dir.cdUp(); 
        dir.cdUp(); 
        dir.cdUp(); 
        app_directory = dir.absolutePath(); 
    }

    return app_directory.toStdString();
}