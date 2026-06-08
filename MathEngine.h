#pragma once
#include <vector>
#include <utility>
#include <future>
#include <thread>

/**
 * @brief A class that provides mathematical algorithms for data processing.
 * * The MathEngine class encapsulates mathematical operations used in the application,
 * such as applying smoothing algorithms to spectral data points.
 */
class MathEngine 
{
    public:
        /**
         * @brief Default constructor for MathEngine.
         */
        MathEngine();

        /**
         * @brief Smooths the provided set of data points.
         * * This function takes raw 2D data points and applies a smoothing algorithm
         * to reduce noise and provide a cleaner signal/curve.
         * * @param input_data A vector of pairs representing the raw (X, Y) coordinates.
         * @return std::vector<std::pair<double, double>> A vector of pairs containing the smoothed (X, Y) coordinates.
         */
        std::vector<std::pair<double, double>> smooth(const std::vector<std::pair<double, double>>& input_data);
};