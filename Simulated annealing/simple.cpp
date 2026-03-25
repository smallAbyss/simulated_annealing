#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include <omp.h>

#include "Hill/HillProblem.hpp"


using namespace std;

double rnd() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

double estimateInitialTemp(std::function<double(const double)> E, int samples = 100) {
    double x = rnd(); ///!hardcode
    double sum = 0;
    int count = 0;
    double glob_ans = E(x);
    for (int i = 0; i < samples; ++i) {
        double x_new = x + rnd(); ///!hardcode
        double e_new = E(x_new);
        glob_ans = min(glob_ans, e_new);
        double dE = e_new - E(x);

        if (dE > 0) {
            sum += dE;
            count++;
        }
        x = x_new;
    }
    double avg_dE = sum / count;
    double P0 = 0.8;
    double ans = -avg_dE / log(P0);
    return ans;
}

double SA(double a, double b, unsigned k_max, std::function<double(const double x)> CalcEnergy) { /// a, b, temp, k_max
    unsigned k = 0;
    double x = a + rnd() * (b - a);
    double temp = estimateInitialTemp(CalcEnergy); 
    unsigned stuck_count = 0;
        
    std::ofstream file;
    file.open("./test.txt");

    std::ofstream file2;
    file2.open("./cov.txt");
    double y = 0.01;
    unsigned cuts = 0;

    while (k <= k_max) {
        if (stuck_count > 20) {
            //temp *= 1.5;
            stuck_count = 0;
        }
        temp *= 0.95;
        double x_old = x;
        double x_new = x + (2 * rnd() - 1);

        double x_cached = x_new;

        if (x_new < a)
            x_new = a + (a - x_new);
        if (x_new > b)
            x_new = b - (x_new - b);
        //x_new = max(a, min(b, x_new));
        double e_old = CalcEnergy(x_old);
        double e_new = CalcEnergy(x_new);
        
        if (x_cached != x_new)
            cuts++;

        if (e_old > e_new)  {
            x = x_new;
            stuck_count = 0;
        }

        if (k == 80 ) 
            double a;
        double dE = e_old - e_new;
        double dET = (e_old - e_new) / temp;
        double Pexp = (exp((e_old - e_new) / temp));
        double ran = rnd();
       if (e_old <= e_new) {
            if (exp((e_old - e_new) / temp) > ran) {
                x = x_new;
                stuck_count = 0;
            }
            else {
                x = x_old;
                stuck_count++;
            }
       }
        k += 1;
        file << x << " " << y << endl;
        file2 << x << " " << CalcEnergy(x) << endl;
        y += 0.01;
    }
    //cout << "CUTS: " << cuts << endl;
    file.close();
    file2.close();
    return x;
}
   

int task_run(int task_num, unsigned Kmax, std::ofstream& file_ans) {
    const size_t N = 1;
    double dif = 0.0;
    vector<double> low_bounds(0), upper_bounds(0);

    THillProblem task = THillProblem(task_num);    
    task.GetBounds(low_bounds, upper_bounds);
    
    std::function<double(double)> EnergyCalc = [&task](double x) {
        return task.ComputeFunction({ x });
    };

    double ans = 0.0;
    ans = SA(low_bounds[0], upper_bounds[0], Kmax, EnergyCalc);
   
    dif = fabs(EnergyCalc(ans) - task.GetOptimumValue());
    std::cout << task_num << "  " << EnergyCalc(ans) << "  " << task.GetOptimumValue() << "  " <<  dif << std::endl;
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

int main() {

    std::cout << "TASK_NUM" << "  " << "SA ENERGY" << "  " << "OPTIMUM VAL" << "  " << "DIFF" << "     SA X" << "  " << "ANS X\n";
//#pragma omp parallel for ordered
    std::ofstream file_ans;
    file_ans.open("./ans.txt");
    for (size_t i = 0; i < 1000; i++)
        task_run(i, 1000, file_ans);
    /*task_run(7, 100);
    main_plot();*/
    file_ans.close();
   /* vector<double> low_bounds(0), upper_bounds(0);
    for (size_t i = 0; i < 1000; i++) {
    THillProblem task = THillProblem(7);
    task.GetBounds(low_bounds, upper_bounds);

    std::function<double(double)> EnergyCalc = [&task](double x) {
        return task.ComputeFunction({ x });
        };

    std::cout << estimateInitialTemp(EnergyCalc) << endl;
    }
    */
    return 0;
}