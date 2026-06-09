#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <QApplication>
#include <QDir>

/**
 * @brief One spectrum file discovered in the data directory.
 */
struct FileEntry 
{
    std::string file_name;    ///< Filename on disk (e.g. @c spectrum_12345.txt).
    std::string display_name; ///< Substring shown in the file list.
    double val;               ///< Numeric value parsed from @c display_name; used for sorting.
};

/**
 * @brief Scans a directory for @c .txt spectra and sorts them by parsed numeric label.
 */
class FileManager 
{
    std::string path;
    public:
        std::vector<FileEntry> files; ///< Entries from the last @ref load call.

        /**
         * @brief Loads all @c .txt files from @p directory and sorts by @c val descending.
         *
         * For each filename, a substring is taken using indices counted from the end
         * of the name: start at @c length + start_index, length @c end_index - start_index + 1.
         * That substring becomes @c display_name; if it parses as a number, it becomes @c val.
         *
         * @param directory     Path to scan.
         * @param start_index   Start offset from the end of the filename (typically negative).
         * @param end_index     End offset from the end of the filename (typically negative).
         */
        void load(const std::string& directory, int start_index, int end_index);

        /** @brief Sorts @c files by @c val in descending order. */
        void sort() {std::sort(files.begin(), files.end(), [](const FileEntry& a, const FileEntry& b) {return a.val > b.val;});};

        /**
         * @brief Returns the directory that holds the spectra and @c data.csv.
         *
         * Normally this is the executable's directory. On macOS the binary sits inside
         * the app bundle (@c …/Contents/MacOS), so the path is moved up four levels to
         * reach the directory that contains the @c .app.
         */
        std::string get_path();
};
