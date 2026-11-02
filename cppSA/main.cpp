#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include "OMP_SA.h"
#include "le_OMP_SA.h"

#include "Hill/HillProblem.hpp"
#include "GKLS/GKLSConstrainedProblem.hpp"
#include "Shekel/ShekelProblem.hpp"
#include "Grishagin/GrishaginConstrainedProblem.hpp"
#include "Grishagin/grishagin_function.hpp"
#include "GKLS/GKLSProblem.hpp"

using namespace std;

# define SA_CLASS le_OMP_SA
// # define SA_CLASS MultiDimSA
const int TASK_NUM = 52;

const int DIM = 2;
const bool RUN_ALL_TASK = false; 

// const size_t STARTS_NUM = 1;
// const int MAX_TASK_NUM = 100;

using Log = Logger<MathVec<double>>;

double task_run(int task_num, unsigned Kmax, Log& log) {
    cout << task_num << "\n";

    // setUp task
    vector<double> low_bounds, upper_bounds;
    TGKLSProblem task(task_num, DIM);
    task.GetBounds(low_bounds, upper_bounds);
    MathVec<double> lowBound(low_bounds), upperBound(upper_bounds);

    std::function<double(const MathVec<double>&)> EnergyCalc =
        [&task](const MathVec<double>& x) { return task.ComputeFunction({ x.GetRawVector() }); };

    SA_CLASS sa(Kmax, lowBound, upperBound, EnergyCalc);
    sa.setObserver(&log);
    
    // log
    log.beginTask(task_num); // /! move inside onStart
    
    // start
    MathVec<double> sa_ans = sa.run(Kmax);

    return log.endTask(sa_ans, EnergyCalc(sa_ans),
                       task.GetOptimumPoint(), task.GetOptimumValue());
}

int main() {
    LoggerConfig cfg;
    cfg.dir          = "./stats";
    cfg.trace_steps  = !RUN_ALL_TASK;   // step-by-step only for a single task
    cfg.trace_stride = 100;             // 1 = every iteration
    cfg.echo_console = !RUN_ALL_TASK;
    Log log(cfg);

    if (RUN_ALL_TASK)
        for (int i = 1; i < 100; ++i) task_run(i, KMAX, log);
    else
        task_run(TASK_NUM, KMAX, log);
}