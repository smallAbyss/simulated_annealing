#include "MultiDimSA.h"
#include <omp.h>
#include <limits>


class le_OMP_SA : public MultiDimSA {
    const int BATCH;

    // avoiding data race 
    static double rnd_local() {
        thread_local std::mt19937 gen(std::random_device{}());
        thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(gen);
    }

    MathVec<double> RandomPoint() {
        MathVec<double> x(DIM);
        for (size_t i = 0; i < DIM; ++i)
            x[i] = leftBorder[i] + rnd_local() * (rightBorder[i] - leftBorder[i]);
        return x;
    }

    // neighbour of an arbitrary point (the base GetNewNeighbour() only perturbs cur_state)
    MathVec<double> NeighbourOf(const MathVec<double>& x, double step_frac) {
        MathVec<double> y(DIM);
        for (size_t i = 0; i < DIM; ++i) {
            double w = rightBorder[i] - leftBorder[i];
            y[i] = fold(x[i] + (2 * rnd_local() - 1) * step_frac * w,
                        leftBorder[i], rightBorder[i]);
        }
        return y;
    }

public:
    le_OMP_SA(
        size_t iteration_max,
        MathVec<double> leftBorder,
        MathVec<double> rightBorder,
        std::function<double(const MathVec<double> cur_x)> CalcEnergy,
        int batch = 4
    ) : MultiDimSA(iteration_max, leftBorder, rightBorder, CalcEnergy), BATCH(batch) {}


virtual MathVec<double> run(unsigned k_max, const bool all_tasks_run) override {
    const double T0 = 10.0;
    const double alpha = std::pow(T_end / T0, 1.0 / k_max);

    MathVec<double> global_best(DIM);
    double global_e = std::numeric_limits<double>::infinity();

    #pragma omp parallel num_threads(BATCH)
    {
        MathVec<double> cur = RandomPoint();
        double e_cur = CalcEnergy(cur);
        MathVec<double> best = cur;
        double e_best = e_cur;
        double temp = T0;

        for (unsigned k = 0; k < k_max; ++k) {
            temp *= alpha;
            const double step_frac = 0.2 * std::sqrt(temp / T0);

            MathVec<double> cand = NeighbourOf(cur, step_frac);
            double e_new = CalcEnergy(cand);

            if (e_new < e_cur || std::exp((e_cur - e_new) / temp) > rnd_local()) {
                cur = cand;
                e_cur = e_new;
                if (e_cur < e_best) { best = cur; e_best = e_cur; }
            }
        }

        #pragma omp critical
        if (e_best < global_e) { global_e = e_best; global_best = best; }
    }

    cur_state = global_best;
    return global_best;
}
};