#include "SpectrumLoader.h"

/**
 * @brief Loads and parses numerical spectral data from a text file.
 * * This method clears any existing data, opens the specified file, and reads it 
 * line by line. It automatically replaces commas with dots to ensure compatibility 
 * with different floating-point decimal notations before parsing the X and Y coordinates.
 * * @param file Constant reference to a string containing the path to the data file.
 * @throws std::runtime_error If the file cannot be opened successfully.
 */
void SpectrumLoader::load(const std::string &file)
{
    data.clear();

    std::ifstream ifile(file);
    if (!ifile.is_open()){throw std::runtime_error("Nie udalo sie otworzyc pliku: " + file);}

    std::string line;
    while (std::getline(ifile, line)) 
    {
        if (line.empty())
            continue;
        std::replace(line.begin(), line.end(), ',', '.');

        std::stringstream ss(line);
        double x, y;

        if (ss >> x >> y) 
            data.push_back(std::make_pair(x, y));
    }

    ifile.close();
}