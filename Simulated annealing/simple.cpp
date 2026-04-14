#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include "Hill/HillProblem.hpp"
#include "GKLS/GKLSConstrainedProblem.hpp"
#include "Shekel/ShekelProblem.hpp"
#include "Grishagin/GrishaginConstrainedProblem.hpp"
#include "Grishagin/grishagin_function.hpp"
#include "GKLS/GKLSProblem.hpp"

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    //os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i != vec.size() - 1) {
            os << "; ";
        }
    }
    //os << "]";
    return os;
}

vector<double> operator-(vector<double> lhs, vector<double> rhs) {
    vector<double> ans(lhs.size());
    for (size_t i = 0; i < lhs.size(); i++)
        ans[i] = lhs[i] - rhs[i];
    return ans;
}

vector<double> operator+(const vector<double> lhs, const vector<double> rhs) {
    vector<double> ans(lhs.size());
    for (size_t i = 0; i < lhs.size(); i++)
        ans[i] = lhs[i] + rhs[i];
    return ans;
}


///! - stands for "Pay attention"
///!! - "BUG HERE"

using namespace std;

const int SAMPLES_NUM = 100;
const double ALPHA_TEMP = 0.9999; /// make high for big kMax, low for small kMax  |  (0.6 ; 0.9995)

const int TASK_NUM = 20;
const bool RUN_ALL_TASK = true;
const unsigned KMAX = 10000;
const size_t STARTS_NUM = 1;

// todo ну
double rnd() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

vector<double> GetNewNeighbour(const vector<double> cur_state, const vector<double> left_border, const vector<double> right_border) {
    vector<double> new_st(cur_state.size());
    for (size_t i = 0; i < cur_state.size(); i++) // &i : cur_state ???
        new_st[i] = cur_state[i] + (2 * rnd() - 1);
    return new_st; ///!hardcode: no borders && no cuts
}

double GetNewNeighbourWithTemp(const double cur_state, const double left_border, const double right_border, const double temp) {
    return cur_state + (2 * rnd() - 1) * sqrt(temp); ///!hardcode: no borders && no cuts
}

double GenInitialState(const double left_border, const double right_border) { ///!hardcode: double -> vector
    return left_border + rnd() * (right_border - left_border);
}

double estimateInitialTemp(std::function<double(const vector<double>)> E, const vector<double> left_border,
                           const vector<double> right_border, const int samples) {
    //vector<double> x = GenInitialState(left_border, right_border);
    vector<double> x = { rnd(), rnd() };
    double sum = 0;
    int count = 0;
    vector<double> x_new = { NULL, NULL};
    double e_new = NULL;
    double dE = NULL;
    double glob_ans = E(x); ///! unused var

    for (int i = 0; i < samples; ++i) {
        x_new = GetNewNeighbour(x, left_border, right_border); 
        //if (x_new < left_border)
        //    x_new = left_border + (left_border - x_new);
        //if (x_new > right_border)
        //    x_new = right_border - (x_new - right_border);
        
        for (size_t i = 0; i < x_new.size(); ++i) {
            x_new[i] = max(left_border[i], min(right_border[i], x_new[i]));
        }
        
        e_new = E(x_new); ///! no borders cut
        glob_ans = min(glob_ans, e_new);
        dE = e_new - E(x);

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

vector<double> SA(vector<double> a, vector<double> b, unsigned k_max, std::function<double(const vector<double>)> CalcEnergy, const bool all_tasks_run) { /// a, b, temp, k_max
    unsigned k = 0;
    //double x =  GenInitialState(a, b);
    vector<double> x = { rnd(), rnd() };
    vector<double>  x_best = x;
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
        vector<double> x_old = x;
        vector<double> x_new = GetNewNeighbour(x, a, b);

        //if (x_new < a)
        //    x_new = a + (a - x_new);
        //if (x_new > b)
        //    x_new = b - (x_new - b);

        for (size_t i = 0; i < x_new.size(); ++i) {
            x_new[i] = max(a[i], min(b[i], x_new[i]));
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
   

int task_run(int task_num, unsigned Kmax, const size_t N, const bool all_tasks_run, std::ofstream& file_ans) {
    double dif = 0.0;
    vector<double> low_bounds(0), upper_bounds(0);

    //THillProblem task = THillProblem(task_num);
    TGKLSConstrainedProblem task2 = TGKLSConstrainedProblem(cptInFeasibleDomain, 0.5, 0, 2);
    TGKLSProblem tt = TGKLSProblem()
        // 0 1 2
    TGrishaginProblem task = TGrishaginProblem(task_num);
    task.GetBounds(low_bounds, upper_bounds);
     

    std::function<double(vector<double>)> EnergyCalc = [&task](vector<double> x) {
        return task.ComputeFunction({ x });
    };

    vector<double> ans = SA(low_bounds, upper_bounds, Kmax, EnergyCalc, all_tasks_run);
    for (size_t i = 0; i < N-1; i++) {
        vector<double> tmp_ans = SA(low_bounds, upper_bounds, Kmax, EnergyCalc, all_tasks_run);
        if (EnergyCalc(ans) > EnergyCalc(tmp_ans))
            ans = tmp_ans;
    }
   
    dif = fabs(EnergyCalc(ans) - task.GetOptimumValue());

    std::cout << task_num << '\n';
    if (!all_tasks_run)
        std:cout << (ans - task.GetOptimumPoint()); // rem fabs
    file_ans << task_num << ';' << EnergyCalc(ans) << ';' << task.GetOptimumValue() << ';' << dif << ';' <<
        ans << ';' << task.GetOptimumPoint()  << ';' << (ans - task.GetOptimumPoint()) << endl; /// fabs

    return dif;
}

int main_plot(const int task_num) {
    const double step = 0.01;
    std::ofstream file;
    file.open("./gcg.txt");

    vector<double> low_bounds(0), upper_bounds(0);

    GrishaginConstrainedProblem task = GrishaginConstrainedProblem();
    task.GetBounds(low_bounds, upper_bounds);

    task.GetBounds(low_bounds, upper_bounds);

    cout << "bounds:" << low_bounds[0] << " " << upper_bounds[0] << endl << endl;
    

    for (double x0 = low_bounds[0]; x0 <= upper_bounds[0]; x0 += step) {
        for (double x1 = low_bounds[1]; x1 <= upper_bounds[1]; x1 += step) {
            file << x0 << " " << x1 << " " << task.ComputeFunction(vector<double>{ x0, x1 }) << std::endl;
        }
    }
    file.close();
    return 0;
}

void main_one(const int task_num, const unsigned kMax, const size_t N) {
    // taskRun
    std::ofstream file_ans;
    file_ans.open("./ans.txt");
    task_run(task_num, kMax, N, false, file_ans);
    file_ans.close();

    // graphic
    //main_plot(task_num);
}

// task num просто чтобы меньше букв менять в мейне
void main_all(const int task_num, const unsigned kMax, const size_t N) {
    std::ofstream file_ans;
    file_ans.open("./ans.txt");
    for (size_t i = 1; i < 101; i++)
        task_run(i, kMax, N, true, file_ans);
    file_ans.close();
}


// 268 - two min's
int main() {
    const int task_num = TASK_NUM;
    const unsigned kMax = KMAX;
    const size_t N = STARTS_NUM;

    //TGKLSConstrainedProblem  t = TGKLSConstrainedProblem();
    //std::vector<double> v1,  v2;
    //t.GetBounds(v1,v2);
    //
    //v2 = t.GetOptimumPoint();




    
    if (RUN_ALL_TASK) 
        main_all(task_num, kMax, N);
    else
        main_one(task_num, kMax, N);
    return 0;
}



