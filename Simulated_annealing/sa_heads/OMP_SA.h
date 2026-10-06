#include "MultiDimSA.h"
#include <omp.h>

class OMP_SA: public MultiDimSA {
public:

    OMP_SA (
        size_t iteration_max, 
        MathVec<double> leftBorder, 
        MathVec<double> rightBorder, 
        std::function<double(const MathVec<double> x)> CalcEnergy
    ) : MultiDimSA(iteration_max, leftBorder, rightBorder, CalcEnergy)
    {  }


    MathVec<double> GetNewNeighbour(const MathVec<double>& state) {
        MathVec<double>new_st(cur_state.size());
        for (size_t i = 0; i < cur_state.size(); i++)
            new_st[i] = cur_state[i] + (2 * rnd() - 1) * temp;
        return new_st;
    }
    
    virtual MathVec<double> run(unsigned k_max, const bool all_tasks_run) override {
        unsigned k = 0;

        MathVec<double> x(leftBorder.size());
        MathVec<double> x_best = x;
        //double temp = estimateInitialTemp(CalcEnergy, a[0], b[0], SAMPLES_NUM); //-- off bcs of a bug inside
        temp = 10.0;

        while (k <= k_max) {
            MathVec<double> x_old = x;
            temp *= ALPHA_TEMP;
            //cout << "TEMP: " << temp << endl;

            const int BATCH = 32;
            std::vector<MathVec<double>> candidates(BATCH);
            std::vector<double> energies(BATCH);

            #pragma omp parallel for
            for (size_t j = 0; j < BATCH; j++) {

                candidates[j] = GetNewNeighbour(x);
                
                for (size_t i = 0; i < candidates[j].size(); ++i) {
                    if (candidates[j][i] < leftBorder[i])
                    candidates[j][i] = leftBorder[i] + (leftBorder[i] - candidates[j][i]);
                    if (candidates[j][i] > rightBorder[i])
                    candidates[j][i] = rightBorder[i] - (candidates[j][i] - rightBorder[i]);
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
            double e_old = CalcEnergy(x_old);
            
            if (CalcEnergy(x_best) > CalcEnergy(x_new)) {
                x_best = x_new;
            }

            if (e_new < e_old || std::exp((e_old - e_new) / temp) > rnd()) {
                x = candidates[best_idx];
            }

            k += 1;
        }
        return (CalcEnergy(x) > CalcEnergy(x_best) ? x_best : x);
    }


};