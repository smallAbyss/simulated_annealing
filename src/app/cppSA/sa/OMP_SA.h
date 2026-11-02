#pragma once
#include "MultiDimSA.h"
#include <omp.h>
#include <algorithm>

class OMP_SA : public MultiDimSA {
    const int BATCH = 10;
public:
    OMP_SA (
        size_t iteration_max,
        MathVec<double> leftBorder,
        MathVec<double> rightBorder,
        std::function<double(const MathVec<double> cur_x)> CalcEnergy
    ) 
    : MultiDimSA(iteration_max, leftBorder, rightBorder, CalcEnergy)
    {  }

    virtual MathVec<double> run(unsigned k_max) override {
        constexpr double T_START = 10.0;
        
        T_0  = T_START;
        temp = T_0;

        best_state = cur_state;
        double e_cur  = CalcEnergy(cur_state);
        double e_best = e_cur;
        if (observer) observer->onStart(cur_state, e_cur, k_max);

        std::vector<MathVec<double>> candidates(BATCH);
        std::vector<double> energies(BATCH);

        const int n_threads = std::max(1, std::min(BATCH, omp_get_max_threads()));

        // The team is created ONCE. Every thread runs the k-loop; the work is shared
        // by `omp for` (candidates) and `omp single` (selection + accept/reject).
        // NOTE: GetNewNeighbour() calls rnd() -> rnd() must be thread_local.
        #pragma omp parallel num_threads(n_threads)
        {
            for (unsigned k = 1; k <= k_max; ++k) {

                #pragma omp for schedule(static)
                for (int j = 0; j < BATCH; ++j) {
                    candidates[j] = GetNewNeighbour();   // fold() keeps it inside the box
                    energies[j]   = CalcEnergy(candidates[j]);
                }   // implicit barrier: all candidates are ready

                #pragma omp single
                {
                    int best_idx = 0;
                    for (int j = 1; j < BATCH; ++j)
                        if (energies[j] < energies[best_idx])
                            best_idx = j;

                    const MathVec<double>& x_new = candidates[best_idx];
                    const double e_new  = energies[best_idx];
                    const double t_used = temp;           // temperature of this iteration

                    const bool new_best = e_new < e_best;
                    if (new_best) { best_state = x_new; e_best = e_new; }

                    const bool accepted = (e_new < e_cur) || (std::exp((e_cur - e_new) / t_used) > rnd());
                    if (accepted) { cur_state = x_new; e_cur = e_new; }

                    // one thread at a time, between two barriers -> safe for a single trace file
                    if (observer)
                        observer->onStep({k, t_used, e_cur, e_new, e_best, accepted, new_best, cur_state, x_new});

                    temp *= ALPHA_TEMP;                   // temperature of iteration k+1
                }   // implicit barrier: cur_state / temp are visible to all threads
            }
        }
        return best_state;
    }
};