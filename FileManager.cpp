#include "FileManager.h"

/**
 * @brief Loads text files from the specified directory and processes their names.
 * * Iterates through the given directory, filtering for regular files with a `.txt` extension.
 * It extracts a substring from the filename using offsets relative to the filename length,
 * attempts to convert this substring to a double value, and stores the entry. 
 * Finally, it triggers the sorting of the loaded files.
 * * @param directory The path to the directory to scan.
 * @param start_index The starting index offset (typically negative, relative to the filename length).
 * @param end_index The ending index offset (typically negative, relative to the filename length).
 */
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

/**
 * @brief Retrieves the application's base execution directory path.
 * * Obtains the binary's directory path using Qt. If the application is running 
 * on macOS within an app bundle structure (path ends with "MacOS"), it navigates 
 * up the directory tree to reach the main deployment root directory.
 * * @return std::string The resolved base directory path as a standard string.
 */
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