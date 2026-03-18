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

double SA(double a, double b, unsigned k_max, std::function<double(const double x)> CalcEnergy) { /// a, b, temp, k_max
    //double a = -10, b = 10; // отрезок на котором ищем максимум
    //k_max = 1000; // число итераций

    unsigned k = 0;
    double x = (b - a) / 2; // ответ
    double temp = 400.0; //  температура
    unsigned stuck_count = 0;
        
    while (k <= k_max) {
        if (stuck_count > 10) {
            temp *= 100;
            stuck_count /= 2;
        }
        temp *= 0.9995;
        double x_old = x;
        double x_new = x + (rnd() > 0.5 ? 1.0 : -1.0) * temp * 0.001;

        //x_new = max(a, min(b, x_new));
        if (x_new < a)
            x_new = a + (a - x_new);
        if (x_new > b)
            x_new = b - (x_new - b);
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
    //cout << x << endl;
    return x;
}
   
int main_MY() {
    size_t N = 100;

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


int main() {
    const size_t N = 10;
    double dif = 0.0;
    // 15
    std::srand(std::time({0}));
    THillProblem a = THillProblem(15);
    //std::cout << a.ComputeFunction({ 1.8901 }); //-0.292235
    vector<double> low_bounds(0), upper_bounds(0);
    a.GetBounds(low_bounds, upper_bounds);
    cout << "bounds:" << low_bounds[0] << " " << upper_bounds[0] << endl;
    
    std::function<double(double)> EnergyCalc = [&a](double x) {
        return a.ComputeFunction({ x });
    };

    for (size_t ITER = 0; ITER < 20; ITER++) {
        double x = 0.0, ans = 0.0;
        for (size_t i = 0; i < N; i++) {
            x = SA(low_bounds[0], upper_bounds[0], 10000, EnergyCalc);
            if (EnergyCalc(x) < EnergyCalc(ans))
                ans = x;
        }
   
        dif += fabs(EnergyCalc(ans) - a.GetOptimumValue());
        cout << "SA point: " << ans << " " << EnergyCalc( ans ) << endl;
        cout << "Exact point: " << a.GetOptimumPoint()[0] << " " << a.GetOptimumValue() << endl;
        if (a.GetMaxPoint().size() > 1) cout << "something wrong..";

        cout << "\n\n\n";
    }
    cout << dif / double(N) << endl;

    return 0;
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