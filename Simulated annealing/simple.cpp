#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "Hill/HillProblem.hpp"

using namespace std;


double CalcEnergy(const double x) {
    return sin(x);
}

double rnd() {
    return static_cast<double>(std::rand()) / RAND_MAX;
}

double SA(double a, double b, unsigned k_max) { /// a, b, temp, k_max
    //double a = -10, b = 10; // отрезок на котором ищем максимум
    //k_max = 1000; // число итераций

    unsigned k = 0;
    double x = 0; // ответ
    double temp = 15.0; //  температура
        
    while (k <= k_max) {
        temp *= 0.99;
        double x_old = x;
        double x_new = x + (rnd() > 0.5 ? 1.0 : -1.0) * temp;

        x_new = max(a, min(b, x_new));
        if (x_new < a)
        if (x_new > b)
            x_new = b - (x_new - b);
        double e_old = CalcEnergy(x_old);
        double e_new = CalcEnergy(x_new);

        if ((e_old < e_new) || (exp((e_new - e_old) / temp) > rnd()))
            x = x_new;
        else
            x = x_old;
        k += 1;
    }
    //cout << x << endl;
    return x;
}
   
int main_() {
    size_t N = 10;

    std::srand(std::time({}));
    double tmp = 0.0, ans = -10.0;
    for (size_t i = 0; i < N; i++) {
        tmp = SA(-10, 10, 1000);
        if (CalcEnergy(tmp) > CalcEnergy(ans))
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
    THillProblem a = THillProblem(0);
    //std::cout << a.ComputeFunction({ 1.8901 }); //-0.292235
    vector<double> low_bounds(0), upper_bounds(0);
    a.GetBounds(low_bounds, upper_bounds);
    cout << "bounds:" << low_bounds[0] << " " << upper_bounds[0];



    return 0;
}