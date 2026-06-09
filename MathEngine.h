#pragma once
#include <vector>
#include <utility>
#include <future>
#include <thread>

/**
 * @brief Savitzky–Golay smoothing of spectral (x, y) data.
 */
class MathEngine 
{
    public:
        MathEngine();

        /**
         * @brief Applies Savitzky–Golay smoothing to the y values.
         *
         * X coordinates are copied unchanged. If there are fewer than 51 points,
         * the input is returned as-is.
         *
         * @param input_data Raw spectrum as (wavelength, intensity) pairs.
         * @return Smoothed spectrum with the same x values.
         */
        std::vector<std::pair<double, double>> smooth(const std::vector<std::pair<double, double>>& input_data);
};
