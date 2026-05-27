#include "FileManager.h"
#include <filesystem>
#include <algorithm>

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
            int length = end_index - start_index + 1; // Z -8 do -5 mamy 4 znaki

            if (start >= 0 && start + length <= file_name.length()) 
            {
                display_name = file_name.substr(start, length);
                
                try 
                {
                    val = std::stod(display_name);
                }
                catch (...) 
                {
                    val = 0.0;
                }
            }

            files.push_back({file_name, display_name, val});
        }
    }
}