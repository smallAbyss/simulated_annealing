#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include "Hill/HillProblem.hpp"

///! - stands for "Pay attention"
///!! - "BUG HERE"

using namespace std;

const int SAMPLES_NUM = 1000;
const double ALPHA_TEMP = 0; /// make high for big kMax, low for small kMax  |  (0.6 ; 0.9995)

const int TASK_NUM = 39;
const unsigned KMAX = 1000;
const size_t STARTS_NUM = 1;

double rnd() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

double GetNewNeighbour(const double cur_state, const double left_border, const double right_border) {
    return cur_state + (2 * rnd() - 1); ///!hardcode: no borders && no cuts
}

double GetNewNeighbourWithTemp(const double cur_state, const double left_border, const double right_border, const double temp) {
    return cur_state + (2 * rnd() - 1) * sqrt(temp); ///!hardcode: no borders && no cuts
}

double GenInitialState(const double left_border, const double right_border) { ///!hardcode: double -> vector
    return left_border + rnd() * (right_border - left_border);
}

double estimateInitialTemp(std::function<double(const double)> E, const double left_border, 
                           const double right_border, const int samples) {
    double x = GenInitialState(left_border, right_border);
    double sum = 0;
    int count = 0;
    double x_new = NULL;
    double e_new = NULL;
    double dE = NULL;
    double glob_ans = E(x); ///! unused var

    for (int i = 0; i < samples; ++i) {
        x_new = GetNewNeighbour(x, left_border, right_border); 
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

double SA(double a, double b, unsigned k_max, std::function<double(const double x)> CalcEnergy, const bool all_tasks_run) { /// a, b, temp, k_max
    unsigned k = 0;
    double x =  GenInitialState(a, b);
    double x_best = x;
    double temp = estimateInitialTemp(CalcEnergy, SAMPLES_NUM, a, b); //-- off bcs of a bug inside
    /* when return NaN its working like a local search and its working BETTER than my SA.. my~25% vs NaN~45% solved by k=50 & 0.01 
    khm, WHAT
    ///!!
    */

    std::ofstream point_coverage_file;
    point_coverage_file.open("./point_coverage.txt");

    std::ofstream sa_trace;
    sa_trace.open("./sa_trace.txt");
    double y = 0;
    if (!all_tasks_run) {
        //cout << x_new << endl;
        point_coverage_file << x << ' ' << y << endl;
        sa_trace << x << ' ' << CalcEnergy(x) << endl;
        y += 0.01;
    }

    while (k <= k_max) {
        temp *= ALPHA_TEMP;
        double x_old = x;
        double x_new = GetNewNeighbour(x, a, b);

        if (x_new < a)
            x_new = a + (a - x_new);
        if (x_new > b)
            x_new = b - (x_new - b);
        //x_new = max(a, min(b, x_new));

        double e_old = CalcEnergy(x_old);
        double e_new = CalcEnergy(x_new);

        /// stat stuff
        double dE = e_old - e_new;
        double dET = (e_old - e_new) / temp;
        double Pexp = (exp((e_old - e_new) / temp));
        double ran = rnd();
        bool tmp_ = Pexp > ran;

        if (e_old > e_new) {
            x_best = x_new;
        }

        if ((e_old > e_new) || exp((e_old - e_new) / temp) > rnd() ){
            x = x_new;
        }

        k += 1;

        if (!all_tasks_run) {
            //cout << x_new << endl;
            point_coverage_file << x << ' ' << y << endl;
            sa_trace << x_new << ' ' << CalcEnergy(x_new) << endl;
            y += 0.01;
        }
    }
    point_coverage_file.close();
    sa_trace.close();
    return (CalcEnergy(x) > CalcEnergy(x_best) ? x_best : x);
}
   

int task_run(int task_num, unsigned Kmax, const size_t N, const bool all_tasks_run, std::ofstream& file_ans) {
    double dif = 0.0;
    vector<double> low_bounds(0), upper_bounds(0);

    THillProblem task = THillProblem(task_num);    
    task.GetBounds(low_bounds, upper_bounds);
    
    std::function<double(double)> EnergyCalc = [&task](double x) {
        return task.ComputeFunction({ x });
    };

    double ans = SA(low_bounds[0], upper_bounds[0], Kmax, EnergyCalc, all_tasks_run);
    for (size_t i = 0; i < N-1; i++) {
        double tmp_ans = SA(low_bounds[0], upper_bounds[0], Kmax, EnergyCalc, all_tasks_run);
        if (EnergyCalc(ans) > EnergyCalc(tmp_ans))
            ans = tmp_ans;
    }
   
    dif = fabs(EnergyCalc(ans) - task.GetOptimumValue());

    std::cout << task_num << '\n';
    if (!all_tasks_run)
        std:cout << fabs(ans - task.GetOptimumPoint()[0]);
    file_ans << task_num << ';' << EnergyCalc(ans) << ';' << task.GetOptimumValue() << ';' << dif << ';' <<
        ans << ';' << task.GetOptimumPoint()[0]  << ';' << fabs(ans - task.GetOptimumPoint()[0]) << endl;

    return dif;
}

int main_plot(const int task_num) {
    const double step = 0.001;
    std::ofstream file;
    file.open("./gcg.txt");

    THillProblem a = THillProblem(task_num);
    vector<double> low_bounds(0), upper_bounds(0);
    a.GetBounds(low_bounds, upper_bounds);

    cout << "bounds:" << low_bounds[0] << " " << upper_bounds[0] << endl << endl;
    
    for (double x = low_bounds[0]; x <= upper_bounds[0]; x += step) {
        file << x << "   " << a.ComputeFunction({ x }) << std::endl;
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
    main_plot(task_num);
}

// task num просто чтобы меньше букв менять в мейне
void main_all(const int task_num, const unsigned kMax, const size_t N) {
    std::ofstream file_ans;
    file_ans.open("./ans.txt");
    for (size_t i = 0; i < 1000; i++)
        task_run(i, kMax, N, true, file_ans);
    file_ans.close();
}


// 268 - two min's
int main() {
    const int task_num = TASK_NUM;
    const unsigned kMax = KMAX;
    const size_t N = STARTS_NUM;

    //for (int i = 0; i < 10; i++)
    //    main_one(task_num, kMax, N);

    //main_all(task_num, kMax, N);
    main_one(task_num, kMax, N);
    return 0;
}