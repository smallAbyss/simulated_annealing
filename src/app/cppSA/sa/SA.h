#pragma once
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>
#include "MathVec.h"
#include "Logger.hpp"

using namespace std;

const int SAMPLES_NUM = 100;
const double ALPHA_TEMP = 0.9995; /// make high for big kMax, low for small kMax  |  (0.6 ; 0.9995)
                        //0.999991
const unsigned KMAX = pow(10, 5);


double rnd() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}



class SA {
protected:
    size_t iteration_max;
    double temp;
    SAObserver<MathVec<double>>* observer = nullptr;   // non-owning
public:
    SA(size_t iteration_max) : iteration_max(iteration_max), temp(0) {}
    virtual ~SA() = default;
    void setObserver(SAObserver<MathVec<double>>* o) { observer = o; }
};