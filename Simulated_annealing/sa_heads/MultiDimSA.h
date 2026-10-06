#include "SA.h"

class MultiDimSA : public SA {
protected:
    MathVec<double> leftBorder; 
    MathVec<double> rightBorder;

    MathVec<double> cur_state;
    MathVec<double> best_state;

    std::function<double(const MathVec<double> x)> CalcEnergy;
    size_t DIM = 0;

public:
    MultiDimSA (
        size_t iteration_max, 
        MathVec<double> leftBorder, 
        MathVec<double> rightBorder, 
        std::function<double(const MathVec<double> x)> CalcEnergy
    ) : SA(iteration_max), leftBorder(leftBorder), rightBorder(rightBorder), CalcEnergy(CalcEnergy) 
    {  
        if (leftBorder.size() != rightBorder.size())
            throw std::invalid_argument("borders have different sizes in SA:constructor");
        DIM = leftBorder.size();
        cur_state = GenInitialState();
    }

    virtual MathVec<double> run(unsigned k_max, const bool all_tasks_run) {
        unsigned k = 0;
        
        //double x =  GenInitialState(a, b);
        // MathVec<double> x(leftBorder.size());
        
        best_state = cur_state; // cur_state is zero vector here
        //double temp = estimateInitialTemp(CalcEnergy, a[0], b[0], SAMPLES_NUM); //-- off bcs of a bug inside
        temp = 10.0;

        while (k <= k_max) {
            temp *= ALPHA_TEMP;
            MathVec<double>new_state = GetNewNeighbour();

            for (size_t i = 0; i < new_state.size(); ++i) {
                // if (new_state[i] < leftBorder[i])
                //     new_state[i] = leftBorder[i] + (leftBorder[i] - new_state[i]);
                // if (new_state[i] > rightBorder[i])
                //     new_state[i] = rightBorder[i] - (new_state[i] - rightBorder[i]);
                new_state[i] = min(rightBorder[i], max(leftBorder[i], new_state[i]));
            }

            double e_cur = CalcEnergy(cur_state);
            double e_new = CalcEnergy(new_state);
            double e_best = CalcEnergy(best_state);

            // stat stuff
            double dE = e_cur - e_new;
            double dET = (e_cur - e_new) / temp;
            double Pexp = (exp((e_cur - e_new) / temp));
            double ran = rnd();
            bool tmp_ = Pexp > ran;

            if (CalcEnergy(best_state) > CalcEnergy(new_state)) {
                best_state = new_state;
            }
            
            if ((e_cur > e_new) || exp((e_cur - e_new) / temp) > rnd() ){
                cur_state = new_state;
            }

            k += 1;
        }
        return (CalcEnergy(cur_state) > CalcEnergy(best_state) ? best_state : cur_state);
    }
    
    virtual MathVec<double> GetNewNeighbour() { ///!hardcode: no borders && no cuts
        MathVec<double>new_st(cur_state.size());
        for (size_t i = 0; i < cur_state.size(); i++) // &i : cur_state ???
            new_st[i] = cur_state[i] + (2 * rnd() - 1) * temp;
        return new_st;
    
    }
    
    // just vec of 0, no border's
    virtual MathVec<double> GenInitialState() { //! no borders
        return MathVec<double>(DIM);
    }

private:

    bool isBeyondRightBorder(MathVec<double> point) {
        if (point.size() != rightBorder.size()) {
            throw std::invalid_argument(
                "different sizes of vectors while comparing to right border in SA" +
                std::to_string(point.size()) + " vs " +
                std::to_string(rightBorder.size()) + ")"
            );
        }

        for (size_t i = 0; i < point.size(); i++) 
            if (rightBorder[i] < point[i]) 
                return true;
        return false;
    }

    bool isBeyondLeftBorder(MathVec<double> point) {
        if (point.size() != leftBorder.size()) {
            throw std::invalid_argument(
                "different sizes of vectors while comparing to left border in SA" +
                std::to_string(point.size()) + " vs " +
                std::to_string(leftBorder.size()) + ")"
            );
        }

        for (size_t i = 0; i < point.size(); i++) 
            if (point[i] < leftBorder[i]) 
                return true;
        return false;
    }

    // можно притвориться умным и сохранять последние sample точек
    // только это никакого объяснения кроме костыля для не-потери вот эти точек не имеет
    virtual double estimateInitialTemp(const int samples) {
        MathVec<double> x = GenInitialState();
        double sum = 0;
        int count = 0;
        MathVec<double> new_state;
        double e_new;
        double dE;
        double glob_ans = CalcEnergy(x); ///! unused var

        for (int i = 0; i < samples; ++i) {
            new_state = GetNewNeighbour(); 
            if (isBeyondLeftBorder(new_state))
                new_state = leftBorder + (leftBorder - new_state);
            if (isBeyondRightBorder(new_state))
                new_state = rightBorder - (new_state - rightBorder);
                
            e_new = CalcEnergy(new_state); ///! no borders cut
            glob_ans = min(glob_ans, e_new);
            dE = e_new - CalcEnergy(x);

            if (dE > 0) {
                sum += dE;
                count++;
            }
            x = new_state;
        }
        double avg_dE = sum / count; 
        double P0 = 0.8;
        if (count > 0) ///!! either if count == 0 returns nan
            return -avg_dE / log(P0);
        return 0.0;
    }


};

