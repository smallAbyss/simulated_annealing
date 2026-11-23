#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include "cxxopts.hpp"

#include "SA.h"
#include "OMP_SA.h"
#include "le_OMP_SA.h"

#include "Hill/HillProblem.hpp"
#include "GKLS/GKLSConstrainedProblem.hpp"
#include "Shekel/ShekelProblem.hpp"
#include "Grishagin/GrishaginConstrainedProblem.hpp"
#include "Grishagin/grishagin_function.hpp"
#include "GKLS/GKLSProblem.hpp"

using namespace std;

// # define SA_CLASS le_OMP_SA
# define SA_CLASS MultiDimSA
int TASK_NUM = 52;

const int DIM = 2;
bool RUN_ALL_TASK = false; 


    

const size_t STARTS_NUM = 10;
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

int main(int argc, char** argv) {

    // KMAX = 0; // значение по умолчанию
    // RUN_ALL_TASK = false;
    // ALPHA_TEMP = 1.0;
    
    try {
        cxxopts::Options options("SA", "Парсер аргументов");

        options.add_options()
            ("k,kmax", "Максимальное значение KMAX", cxxopts::value<unsigned>(KMAX))
            ("t,task", "Task number", cxxopts::value<int>(TASK_NUM))
            ("run-all", "Флаг RUN_ALL_TASK (включён, если указан)")
            ("a,alpha", "Значение ALPHA_TEMP", cxxopts::value<double>(ALPHA_TEMP))
            ("h,help", "Показать помощь");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        // RUN_ALL_TASK - флаг: если опция присутствовала, ставим true
        if (result.count("run-all")) {
            RUN_ALL_TASK = true;
        }

        std::cout << "KMAX = " << KMAX << "\n"
                  << "RUN_ALL_TASK = " << std::boolalpha << RUN_ALL_TASK << "\n"
                  << "ALPHA_TEMP = " << ALPHA_TEMP << "\n\n";

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Ошибка парсинга: " << e.what() << std::endl;
        return 1;
    }

    LoggerConfig cfg;
    cfg.dir          = "./data/tmp_run";
    cfg.trace_steps  = true; //!RUN_ALL_TASK;   // step-by-step only for a single task
    cfg.trace_stride = 10000;             // 1 = every iteration
    cfg.echo_console = !RUN_ALL_TASK;
    Log log(cfg);

    if (RUN_ALL_TASK)
        for (int i = 1; i < 101; ++i) {
            double dif = 12;    
            for (int j = 0; j < STARTS_NUM; ++j) {
                double dif_tmp = task_run(i, KMAX, log);
                dif = dif < dif_tmp ? dif : dif_tmp;
            }
            // cout << "\nBest dif: " << dif << endl;
        }
    else {
        for (int i = 0; i < STARTS_NUM; i++)
            task_run(TASK_NUM, KMAX, log);
    }
}