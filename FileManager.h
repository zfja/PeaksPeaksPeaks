#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <QApplication>
#include <QDir>

/**
 * @brief Structure representing a single file entry.
 * * Stores the actual filename, a formatted name for UI display, 
 * and a parsed numeric value which can be used for sorting.
 */
struct FileEntry 
{
    std::string file_name;    ///< The actual name of the file on disk.
    std::string display_name; ///< The formatted name intended for user interface display.
    double val;               ///< A numeric value associated with the file, used for sorting.
};

/**
 * @brief Class responsible for managing file loading, sorting, and path resolution.
 * * The FileManager handles scanning a specific directory, extracting information 
 * from file names to populate FileEntry structures, and providing sorting capabilities.
 */
class FileManager 
{
    std::string path; ///< Internal storage for the base directory path.
    public:
        std::vector<FileEntry> files; ///< A collection of loaded file entries.

        /**
         * @brief Loads files from a specified directory and populates the files vector.
         * * Parses filenames to extract specific substrings (based on start_index and end_index)
         * to evaluate the display_name and val members of each FileEntry.
         * * @param directory The path to the directory containing the files to load.
         * @param start_index The starting index for substring extraction from the filename.
         * @param end_index The ending index (or offset) for substring extraction.
         */
        void load(const std::string& directory, int start_index, int end_index);

        /**
         * @brief Sorts the loaded files in descending order based on their 'val' member.
         */
        void sort() {std::sort(files.begin(), files.end(), [](const FileEntry& a, const FileEntry& b) {return a.val > b.val;});};

        /**
         * @brief Retrieves the target directory path for the data files.
         * * @return std::string The resolved directory path.
         */
        std::string get_path();
};