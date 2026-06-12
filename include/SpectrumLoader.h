// PeaksPeaksPeaks — Copyright (c) 2026 Zofia Tryznowska
#pragma once
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

/**
 * @brief Reads (x, y) spectrum points from a text file.
 *
 * Expected format: one pair per line, whitespace-separated; commas are accepted as decimal separators.
 */
class SpectrumLoader
{
    public:
        std::vector<std::pair<double, double>> data; ///< Points loaded by the last @ref load call.

        /**
         * @brief Parses spectrum data from @p file into @c data.
         * @param file Path to the spectrum file.
         * @throws std::runtime_error if the file cannot be opened.
         */
        void load(const std::string &file);
};
