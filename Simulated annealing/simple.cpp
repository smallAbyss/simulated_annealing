#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include "Hill/HillProblem.hpp"


using namespace std;

const int SAMPLES_NUM = 10;

double rnd() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

double GetNewNeighbour(const double cur_state, const double left_border, const double right_border) {
    return cur_state + (2 * rnd() - 1); ///!hardcode: no borders
}

double GenInitialState(const double left_border, const double right_border) { ///!hardcode: double -> vector
    return right_border + rnd() * (right_border - left_border); 
}

double estimateInitialTemp(std::function<double(const double)> E, const double left_border, 
                           const double right_border, const int samples) {
    double x = GenInitialState(left_border, right_border);
    double sum = 0;
    int count = 0;
    double x_new = NULL;
    double e_new = NULL;
    double dE = NULL;
    double glob_ans = E(x);

    for (int i = 0; i < samples; ++i) {
        x_new = GetNewNeighbour(x, left_border, right_border);
        e_new = E(x_new);
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
    return -avg_dE / log(P0);
}

double SA(double a, double b, unsigned k_max, std::function<double(const double x)> CalcEnergy) { /// a, b, temp, k_max
    unsigned k = 0;
    double x =  GenInitialState(a, b);
    double temp = estimateInitialTemp(CalcEnergy, SAMPLES_NUM, a, b);

    std::ofstream point_coverage_file;
    point_coverage_file.open("./point_coverage.txt");

    std::ofstream cov;
    cov.open("./cov.txt");
    double y = 0;


    while (k <= k_max) {
        temp *= 0.95;
        double x_old = x;
        double x_new = GetNewNeighbour(x, a, b);

        if (x_new < a)
            x_new = a + (a - x_new);
        if (x_new > b)
            x_new = b - (x_new - b);
        //x_new = max(a, min(b, x_new));

        double e_old = CalcEnergy(x_old);
        double e_new = CalcEnergy(x_new);

        //double dE = e_old - e_new;
        //double dET = (e_old - e_new) / temp;
        //double Pexp = (exp((e_old - e_new) / temp));
        //double ran = rnd();
        
        if ((e_old > e_new) || exp((e_old - e_new) / temp) > rnd() ){
            x = x_new;
        }

        k += 1;

        point_coverage_file << x << ' ' << y << endl;
        cout<< x << ' ' << CalcEnergy(x) << endl;
        y += 0.01;

    }
    point_coverage_file.close();
    cov.close();
    return x;
}
   

int task_run(int task_num, unsigned Kmax, const size_t N, std::ofstream& file_ans) {
    double dif = 0.0;
    vector<double> low_bounds(0), upper_bounds(0);

    THillProblem task = THillProblem(task_num);    
    task.GetBounds(low_bounds, upper_bounds);
    
    std::function<double(double)> EnergyCalc = [&task](double x) {
        return task.ComputeFunction({ x });
    };

    double ans = SA(low_bounds[0], upper_bounds[0], Kmax, EnergyCalc);
    for (size_t i = 0; i < N-1; i++) {
        double tmp_ans = SA(low_bounds[0], upper_bounds[0], Kmax, EnergyCalc);
        if (EnergyCalc(ans) > EnergyCalc(tmp_ans))
            ans = tmp_ans;
    }
   
    dif = fabs(EnergyCalc(ans) - task.GetOptimumValue());

    std::cout << task_num << '\n';
    file_ans << task_num << ';' << EnergyCalc(ans) << ';' << task.GetOptimumValue() << ';' << dif << ';' <<
        ans << ';' << task.GetOptimumPoint()[0]  << ';' << fabs(ans - task.GetOptimumPoint()[0]) << endl;

    return dif;
}

int main_plot() {
    std::ofstream file;
    file.open("./gcg.txt");

    THillProblem a = THillProblem(7);
    vector<double> low_bounds(0), upper_bounds(0);
    a.GetBounds(low_bounds, upper_bounds);

    cout << "bounds:" << low_bounds[0] << " " << upper_bounds[0] << endl << endl;
    
    for (double x = low_bounds[0]; x <= upper_bounds[0]; x += 0.01) {
        file << x << "   " << a.ComputeFunction({ x }) << std::endl;
    }
    file.close();
    return 0;
}

void main_one() {
    const int task_num = 15;
    const unsigned Kmax = 1000;
    const size_t N = 1;
    std::ofstream file_ans;
    file_ans.open("./ans.txt");
    
    task_run(task_num, Kmax, N, file_ans);

    file_ans.close();
}

void main_all() {
    const unsigned kMax = 1000;
    const size_t N = 1;

    std::ofstream file_ans;
    file_ans.open("./ans.txt");
    for (size_t i = 0; i < 1000; i++)
        task_run(i, kMax, N, file_ans);
    file_ans.close();
}


int main() {
    main_one();
    return 0;
}