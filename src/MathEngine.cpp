#include "MathEngine.h"
#include <algorithm>
#include <cmath>

MathEngine::MathEngine() {}

namespace 
{

/// Solves the small system A x = b via Gaussian elimination with partial
/// pivoting. Returns false if A is (numerically) singular.
bool solve_linear(std::vector<std::vector<double>> A, std::vector<double> b, std::vector<double>& x)
{
    int dim = static_cast<int>(b.size());
    x.assign(dim, 0.0);

    for (int col = 0; col < dim; ++col)
    {
        int pivot = col;
        double best = std::abs(A[col][col]);
        for (int r = col + 1; r < dim; ++r)
        {
            if (std::abs(A[r][col]) > best) { best = std::abs(A[r][col]); pivot = r; }
        }
        if (best < 1e-12)
            return false;

        std::swap(A[col], A[pivot]);
        std::swap(b[col], b[pivot]);

        for (int r = 0; r < dim; ++r)
        {
            if (r == col)
                continue;
            double factor = A[r][col] / A[col][col];
            for (int c = col; c < dim; ++c)
                A[r][c] -= factor * A[col][c];
            b[r] -= factor * b[col];
        }
    }

    for (int i = 0; i < dim; ++i)
        x[i] = b[i] / A[i][i];
    return true;
}

/// Savitzky–Golay smoothing weights for a symmetric window of half-width @p k
/// and polynomial degree @p p (uniform spacing assumed). The smoothed center
/// value is the constant term of the least-squares fit, i.e. row 0 of
/// (A^T A)^{-1} A^T, obtained by solving (A^T A) z = e0 and evaluating
/// w_j = sum_d z_d * j^d.
std::vector<double> sg_weights(int k, int p)
{
    int p_eff = std::min(p, 2 * k);
    int dim = p_eff + 1;

    std::vector<double> power_sum(2 * p_eff + 1, 0.0);
    for (int j = -k; j <= k; ++j)
    {
        double jp = 1.0;
        for (int t = 0; t <= 2 * p_eff; ++t)
        {
            power_sum[t] += jp;
            jp *= j;
        }
    }

    std::vector<std::vector<double>> ata(dim, std::vector<double>(dim, 0.0));
    for (int a = 0; a < dim; ++a)
        for (int b = 0; b < dim; ++b)
            ata[a][b] = power_sum[a + b];

    std::vector<double> e0(dim, 0.0);
    e0[0] = 1.0;

    std::vector<double> w(2 * k + 1, 0.0);
    std::vector<double> z;
    if (!solve_linear(ata, e0, z))
    {
        w[k] = 1.0;
        return w;
    }

    for (int j = -k; j <= k; ++j)
    {
        double val = 0.0, jp = 1.0;
        for (int d = 0; d < dim; ++d) { val += z[d] * jp; jp *= j; }
        w[j + k] = val;
    }
    return w;
}

} // namespace

/**
 * General Savitzky–Golay smoothing with a configurable window and polynomial
 * degree. Near both ends the half-window shrinks symmetrically
 * (k = min(m, i, n-1-i)) so boundary points are still smoothed with their own
 * valid coefficients instead of being left as raw noise. The work is split
 * across two std::async tasks over the two halves of the data.
 */
std::vector<std::pair<double, double>> MathEngine::smooth(const std::vector<std::pair<double, double>>& input_data,
                                                         int window_length,
                                                         int poly_order)
{
    int n = static_cast<int>(input_data.size());
    std::vector<std::pair<double, double>> result = input_data;

    if (n < 3)
        return result;

    if (window_length < 3)
        window_length = 3;
    if (window_length % 2 == 0)
        window_length += 1;
    if (poly_order < 0)
        poly_order = 0;

    int m = window_length / 2;
    if (m > (n - 1) / 2)
        m = (n - 1) / 2;
    if (m < 1)
        return result;

    std::vector<std::vector<double>> weights_by_k(m + 1);
    for (int k = 1; k <= m; ++k)
        weights_by_k[k] = sg_weights(k, poly_order);

    auto smooth_at = [&](int i)
    {
        int k = std::min(m, std::min(i, n - 1 - i));
        if (k <= 0)
            return;
        const std::vector<double>& w = weights_by_k[k];
        double sum = 0.0;
        for (int j = -k; j <= k; ++j)
            sum += input_data[i + j].second * w[j + k];
        result[i].second = sum;
    };

    int mid_idx = n / 2;

    auto thread1 = std::async(std::launch::async, [&]() 
    {
        for (int i = 0; i < mid_idx; ++i) 
            smooth_at(i);
    });

    auto thread2 = std::async(std::launch::async, [&]() 
    {
        for (int i = mid_idx; i < n; ++i) 
            smooth_at(i);
    });

    thread1.get();
    thread2.get();

    return result;
}
