#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "Hill/HillProblem.hpp"
#include <functional>
#include <fstream>
#include <random>

using namespace std;


double CalcEnergy(const double x) {
    return sin(x);
}

//double rnd() {
//    return static_cast<double>(std::rand()) / RAND_MAX;
//}


double rnd() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

double estimateInitialTemp(
    std::function<double(const double)> E,
    int samples = 1000)
{
    double x = 0.5;
    double sum = 0;
    int count = 0;

    for (int i = 0; i < samples; ++i)
    {
        double x_new = x + rnd();
        double dE = E(x_new) - E(x);

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
    double x = a + rnd() * (b - a); // ответ
    double temp = 10.0; //  температура
    unsigned stuck_count = 0;
        
    while (k <= k_max) {
        if (stuck_count > 20) {
            temp *= 100;
            stuck_count /= 2;
        }
        temp *= 0.9995;
        double x_old = x;
        double x_new = x + (rnd() > 0.5 ? 1.0 : -1.0) * 0.1 * sqrt(sqrt(temp));


        if (x_new < a)
            x_new = a + (a - x_new);
        if (x_new > b)
            x_new = b - (x_new - b);
        x_new = max(a, min(b, x_new));
        double e_old = CalcEnergy(x_old);
        double e_new = CalcEnergy(x_new);

        if ((e_old > e_new) || (exp((e_new - e_old) / temp) > rnd())) {
            x = x_new;
            stuck_count = 0;
        }   
        else {
            x = x_old;
            stuck_count++;
        }
        k += 1;
    }
    return x;
}
   
int main_MY() {
    size_t N = 1000;

    std::srand(std::time({0}));
    double tmp = 0.0, ans = -10.0;
    for (size_t i = 0; i < N; i++) {
        tmp = SA(-10, 10, 10000, CalcEnergy);
        if (CalcEnergy(tmp) < CalcEnergy(ans))
            ans = tmp;
    }
    cout << "\n\n";
    cout << "x: " << ans << "  Max energy: " << CalcEnergy(ans);
    cout << "\n\n\n";
    cout << 3.14 / 2.0 << endl;
    cout << 3.14 / 2.0 * 5.0 << endl;
    cout << -3.14 / 2.0 * 3.0 << endl;
    return 0;
}


int task_run(int task_num) {
    const size_t N = 50;
    double dif = 0.0;
    vector<double> low_bounds(0), upper_bounds(0);

    THillProblem task = THillProblem(task_num);    
    task.GetBounds(low_bounds, upper_bounds);
    
    std::function<double(double)> EnergyCalc = [&task](double x) {
        return task.ComputeFunction({ x });
    };

    double tmp_ans = 0.0, ans = 0.0;
    for (size_t i = 0; i < N; i++) {
        tmp_ans = SA(low_bounds[0], upper_bounds[0], 5000, EnergyCalc);
        if (EnergyCalc(tmp_ans) < EnergyCalc(ans))
            ans = tmp_ans;
    }
   
    dif = fabs(EnergyCalc(ans) - task.GetOptimumValue());
    
    //std::cout << task_num << "  " << EnergyCalc(ans) << "  " << task.GetOptimumValue() << "  " << dif << std::endl;
    std::cout << task_num << ';' << EnergyCalc(ans) << ';' << task.GetOptimumValue() << ';' << dif << std::endl;

    return dif;
}

int main_plot() {
    std::ofstream file;
    file.open("./gcg.txt");

    THillProblem a = THillProblem(15);
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

    for (size_t i = 0; i < 1000; i++)
        task_run(i);


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