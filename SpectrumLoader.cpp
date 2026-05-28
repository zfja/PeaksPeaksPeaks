#include "SpectrumLoader.h"

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