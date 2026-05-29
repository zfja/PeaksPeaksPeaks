#include "MathEngine.h"


MathEngine::MathEngine() {}

std::vector<std::pair<double, double>> MathEngine::smooth(const std::vector<std::pair<double, double>>& input_data) 
{
    int window_length = 51;
    int m = window_length / 2; 
    int n = input_data.size();
    std::vector<std::pair<double, double>> result = input_data;
    
    if (n < window_length) {return result;} //aby się nie wykrzaczyło

    double norm = (2.0 * m - 1.0) * (2.0 * m + 1.0) * (2.0 * m + 3.0);
    std::vector<double> weights(window_length);
    for (int j = -m; j <= m; ++j)
        weights[j + m] = 3.0 * (3.0 * m * m + 3.0 * m - 1.0 - 5.0 * j * j) / norm;

    int mid_idx = m + (n - 2 * m) / 2;

    auto thread1 = std::async(std::launch::async, [&]() 
    {
        for (int i = m; i < mid_idx; ++i) 
        {
            double sum = 0.0;
            for (int j = -m; j <= m; ++j)
                sum += input_data[i + j].second * weights[j + m];
            result[i].second = sum;
        }
    });

    auto thread2 = std::async(std::launch::async, [&]() {
        for (int i = mid_idx; i < n - m; ++i) 
        {
            double sum = 0.0;
            for (int j = -m; j <= m; ++j)
                sum += input_data[i + j].second * weights[j + m];
            result[i].second = sum;
        }
    });

    thread1.get();
    thread2.get();

    return result;
}