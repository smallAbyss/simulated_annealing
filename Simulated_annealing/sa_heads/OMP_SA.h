#include "MultiDimSA.h"
#include <omp.h>

class OMP_SA: public MultiDimSA {
    const int BATCH = 5;
public:
    OMP_SA (
        size_t iteration_max, 
        MathVec<double> leftBorder, 
        MathVec<double> rightBorder, 
        std::function<double(const MathVec<double> cur_x)> CalcEnergy
    ) : MultiDimSA(iteration_max, leftBorder, rightBorder, CalcEnergy)
    {  }
    
    virtual MathVec<double> run(unsigned k_max, const bool all_tasks_run) override {
        unsigned k = 0;
        MathVec<double> best_state = cur_state;
        //double temp = estimateInitialTemp(CalcEnergy, a[0], b[0], SAMPLES_NUM); //-- off bcs of a bug inside
        temp = 10.0;

        while (k <= k_max) {
            temp *= ALPHA_TEMP;
            
            std::vector<MathVec<double>> candidates(BATCH);
            std::vector<double> energies(BATCH);

            #pragma omp parallel for
            for (size_t j = 0; j < BATCH; j++) {

                candidates[j] = GetNewNeighbour();
                
                for (size_t i = 0; i < candidates[j].size(); ++i) {
                    // if (candidates[j][i] < leftBorder[i])
                    // candidates[j][i] = leftBorder[i] + (leftBorder[i] - candidates[j][i]);
                    // if (candidates[j][i] > rightBorder[i])
                    // candidates[j][i] = rightBorder[i] - (candidates[j][i] - rightBorder[i]);
                    candidates[j][i] = min(rightBorder[i], max(leftBorder[i], candidates[j][i]));
                }
                energies[j] = CalcEnergy(candidates[j]);
            }

            int best_idx = 0;

            for (size_t j = 1; j < BATCH; j++) {
                if (energies[j] < energies[best_idx])
                    best_idx = j;
            }

            MathVec<double> x_new = candidates[best_idx];
            double e_new = energies[best_idx];
            double e_cur = CalcEnergy(cur_state);
            
            if (CalcEnergy(best_state) > CalcEnergy(x_new)) {
                best_state = x_new;
            }

            if (e_new < e_cur || std::exp((e_cur - e_new) / temp) > rnd()) {
                cur_state = candidates[best_idx];
            }

            k += 1;
        }
        return (CalcEnergy(cur_state) > CalcEnergy(best_state) ? best_state : cur_state);
    }
};