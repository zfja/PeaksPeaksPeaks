#pragma once
#include <string>
#include <vector>

struct FileEntry 
{
    std::string file_name;
    std::string display_name;
    double val;
};

class FileManager 
{
public:
    std::vector<FileEntry> files;
    void load(const std::string& directory, int start_index, int end_index);
    void sort() {std::sort(files.begin(), files.end(), [](const FileEntry& a, const FileEntry& b) {return a.val < b.val;});};
};