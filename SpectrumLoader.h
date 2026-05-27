#pragma once
#include <vector>
#include <fstream>
#include <sstream>

class SpectrumLoader
{
    public:
        std::vector<double, double> data;
    private:
        void load(const std::string &directory);
};