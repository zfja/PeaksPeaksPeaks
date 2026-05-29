#pragma once
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>


class SpectrumLoader
{
    public:
        std::vector<std::pair<double, double>> data;
        void load(const std::string &file);
};