#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>
#include "MathVec.h"

using namespace std;

const int SAMPLES_NUM = 100;
const double ALPHA_TEMP = 0.99995; /// make high for big kMax, low for small kMax  |  (0.6 ; 0.9995)


double rnd() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}


// template <typename V> // V - vector aka point, R - result aka FunctionResult aka double
class SA {
protected:
    size_t iteration_max;
    double temp;
public:    
    SA(size_t iteration_max) : temp(0), iteration_max(iteration_max) {}

    // virtual MathVec<double> run();
    // virtual MathVec<double> GetNewNeighbour();
    // virtual MathVec<double> GenInitialState();

protected:
    // virtual double estimateInitialTemp();
};