#pragma once
#include <vector>
#include <utility>
#include <future>
#include <thread>

class MathEngine 
{
    public:
        MathEngine();
        std::vector<std::pair<double, double>> smooth(const std::vector<std::pair<double, double>>& input_data);
};