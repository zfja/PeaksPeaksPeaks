// PeaksPeaksPeaks — Copyright (c) 2026 Zofia Tryznowska
#pragma once
#include <future>
#include <thread>
#include <utility>
#include <vector>

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
         * Fits a local polynomial of degree @p poly_order over a window of
         * @p window_length points (forced odd) and replaces each y with the
         * fitted center value. Near the edges the window shrinks symmetrically
         * so boundary points are smoothed too, avoiding the raw, noisy tails a
         * fixed window would leave. X coordinates are copied unchanged. With
         * fewer than 3 points the input is returned as-is.
         *
         * @param input_data    Raw spectrum as (wavelength, intensity) pairs.
         * @param window_length Target window size in points; even values are
         *                      bumped up to the next odd number.
         * @param poly_order    Degree of the local polynomial (e.g. 2 = quadratic).
         * @return Smoothed spectrum with the same x values.
         */
        std::vector<std::pair<double, double>> smooth(const std::vector<std::pair<double, double>>& input_data,
                                                      int window_length = 51,
                                                      int poly_order = 2);
};
