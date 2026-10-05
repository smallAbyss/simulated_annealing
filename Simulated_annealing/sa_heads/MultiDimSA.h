#include "SA.h"

class MultiDimSA : public SA {
    MathVec<double> leftBorder; 
    MathVec<double> rightBorder;
    MathVec<double> cur_state;
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
        MathVec<double> x(leftBorder.size());
        
        MathVec<double> x_best = x;
        //double temp = estimateInitialTemp(CalcEnergy, a[0], b[0], SAMPLES_NUM); //-- off bcs of a bug inside
        double temp = 10.0;
        /* when return NaN its working like a local search and its working BETTER than my SA.. my~25% vs NaN~45% solved by k=50 & 0.01 
        khm, WHAT
        ///!!
        */

        std::ofstream point_coverage_file;
        point_coverage_file.open("./point_coverage.txt");
        std::ofstream sa_trace;
        sa_trace.open("./sa_trace.txt");
        std::ofstream sa_trace_lucky;
        sa_trace_lucky.open("./sa_trace_lucky.txt");
        
        double y = 0;
        if (!all_tasks_run) {
            //cout << x_new << endl;
            point_coverage_file << x << endl;
            sa_trace << x << ' ' << CalcEnergy(x) << endl;
            sa_trace_lucky << x << ' ' << CalcEnergy(x) << endl;

            y += 0.01;
        }
        while (k <= k_max) {
            temp *= ALPHA_TEMP;
            //cout << "TEMP: " << temp << endl;
            MathVec<double>x_old = x;
            MathVec<double>x_new = GetNewNeighbour();


            for (size_t i = 0; i < x_new.size(); ++i) {
                x_new[i] = max(rightBorder[i], min(leftBorder[i], x_new[i]));
            }

            double e_old = CalcEnergy(x_old);
            double e_new = CalcEnergy(x_new);

            /// stat stuff
            double dE = e_old - e_new;
            double dET = (e_old - e_new) / temp;
            double Pexp = (exp((e_old - e_new) / temp));
            double ran = rnd();
            bool tmp_ = Pexp > ran;

            if (CalcEnergy(x_best) > CalcEnergy(x_new)) {
                x_best = x_new;
            }

            if ((e_old > e_new) || exp((e_old - e_new) / temp) > rnd() ){
                x = x_new;
            }

            k += 1;

            if (!all_tasks_run) {
                //cout << x_new << endl;

                point_coverage_file << x << endl;
                sa_trace << x_new << ' ' << CalcEnergy(x_new) << endl;
                sa_trace_lucky << x << ' ' << CalcEnergy(x) << endl;
                y += 0.01;
            }
        }
        point_coverage_file.close();
        sa_trace.close();
        sa_trace_lucky.close();
        return (CalcEnergy(x) > CalcEnergy(x_best) ? x_best : x);
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
        MathVec<double> x_new;
        double e_new;
        double dE;
        double glob_ans = CalcEnergy(x); ///! unused var

        for (int i = 0; i < samples; ++i) {
            x_new = GetNewNeighbour(); 
            if (isBeyondLeftBorder(x_new))
                x_new = leftBorder + (leftBorder - x_new);
            if (isBeyondRightBorder(x_new))
                x_new = rightBorder - (x_new - rightBorder);
                
            e_new = CalcEnergy(x_new); ///! no borders cut
            glob_ans = min(glob_ans, e_new);
            dE = e_new - CalcEnergy(x);

            if (dE > 0) {
                sum += dE;
                count++;
            }
            x = x_new;
        }
        double avg_dE = sum / count; 
        double P0 = 0.8;
        if (count > 0) ///!! either if count == 0 returns nan
            return -avg_dE / log(P0);
        return 0.0;
    }


};

