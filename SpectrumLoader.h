#pragma once
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>

/**
 * @brief Class responsible for loading and parsing spectral data from files.
 * * This class reads coordinate data from a specified file and stores it 
 * in a container for further processing and visualization.
 */
class SpectrumLoader
{
    public:
        std::vector<std::pair<double, double>> data; ///< Vector storing the loaded data points as (X, Y) pairs.

        /**
         * @brief Loads spectral data from a specified file path.
         * * Opens the given file, parses its content, and populates the data vector 
         * with the extracted coordinate pairs.
         * * @param file The path to the file to be loaded.
         */
        void load(const std::string &file);
};