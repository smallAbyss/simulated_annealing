#pragma once
#include "MultiDimSA.h"
#include <omp.h>
#include <limits>


class le_OMP_SA : public MultiDimSA {
    const int BATCH;   // number of independent chains (= threads)

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
        int batch = 10
    ) : MultiDimSA(iteration_max, leftBorder, rightBorder, CalcEnergy), BATCH(batch) {}


    virtual MathVec<double> run(unsigned k_max) override {
        const double T0 = 10.0;
        const double alpha = std::pow(T_end / T0, 1.0 / k_max);

        MathVec<double> global_best(DIM);
        double global_e = std::numeric_limits<double>::infinity();

        // master thread, before the team starts: logger opens one trace file per chain
        if (observer) observer->onChainsBegin(BATCH);

        #pragma omp parallel num_threads(BATCH)
        {
            const int chain = omp_get_thread_num();

            MathVec<double> cur = RandomPoint();
            double e_cur = CalcEnergy(cur);
            MathVec<double> best = cur;
            double e_best = e_cur;
            double temp = T0;                 // chain-local, shadows the member on purpose

            if (observer) observer->onChainStart(chain, cur, e_cur, k_max);

            for (unsigned k = 1; k <= k_max; ++k) {
                temp *= alpha;
                const double step_frac = 0.2 * std::sqrt(temp / T0);

                MathVec<double> cand = NeighbourOf(cur, step_frac);
                const double e_new = CalcEnergy(cand);

                const bool accepted = (e_new < e_cur) || (std::exp((e_cur - e_new) / temp) > rnd_local());
                const bool new_best = e_new < e_best;   // implies accepted (e_best <= e_cur)

                if (accepted) { cur = cand; e_cur = e_new; }
                if (new_best) { best = cand; e_best = e_new; }

                // each chain only touches its own trace stream -> no sync needed
                if (observer)
                    observer->onStep({k, temp, e_cur, e_new, e_best, accepted, new_best, cur, cand, chain});
            }

            #pragma omp critical
            if (e_best < global_e) { global_e = e_best; global_best = best; }
        }

        cur_state = global_best;
        return global_best;
    }
};