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
    std::filesystem::path start = QApplication::applicationDirPath().toStdString();
    std::filesystem::path dir = start;

    for (int level = 0; level <= 6; ++level)
    {
        std::error_code ec;
        bool is_build_dir = std::filesystem::exists(dir / "CMakeCache.txt", ec);
        bool has_spectrum = false;

        if (!is_build_dir)
        {
            for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".txt")
                {
                    has_spectrum = true;
                    break;
                }
            }
        }

        if (has_spectrum)
            return dir.string();

        std::filesystem::path parent = dir.parent_path();
        if (parent.empty() || parent == dir)
            break;
        dir = parent;
    }

    return start.string();
}
