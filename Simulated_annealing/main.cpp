#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include "sa_heads/OMP_SA.h"

#include "Hill/HillProblem.hpp"
#include "GKLS/GKLSConstrainedProblem.hpp"
#include "Shekel/ShekelProblem.hpp"
#include "Grishagin/GrishaginConstrainedProblem.hpp"
#include "Grishagin/grishagin_function.hpp"
#include "GKLS/GKLSProblem.hpp"

using namespace std;

const int TASK_NUM = 10;
const int DIM = 2;
const bool RUN_ALL_TASK = false; 

// const size_t STARTS_NUM = 1;
// const int MAX_TASK_NUM = 100;

int task_run(int task_num, unsigned Kmax, const bool all_tasks_run, std::ofstream& file_ans) {
    double dif = 0.0;
    
    // converting bounds to MathVec
    vector<double> low_bounds(0), upper_bounds(0);
    TGKLSProblem task = TGKLSProblem(task_num, DIM); // also has TGrishaginProblem, THillProblem(task_num) (both are linear)    
    task.GetBounds(low_bounds, upper_bounds);
    MathVec<double> lowBound(low_bounds), upperBound(upper_bounds); 
    
    // converting function to func(MathVec)
    std::function<double(const MathVec<double>&)> EnergyCalc = [&task](const MathVec<double>& x) {
        return task.ComputeFunction({ x.GetRawVector() });
    };
    
    // run SA
    MultiDimSA sa(KMAX, lowBound, upperBound, EnergyCalc);
    MathVec<double> sa_ans = sa.run(KMAX, false);

    // collect stats
    MathVec<double> true_ans = task.GetOptimumPoint();
    dif = fabs(EnergyCalc(sa_ans) - task.GetOptimumValue());
    
    if (!all_tasks_run) {
        // cout << endl << task_num << ';' << EnergyCalc(sa_ans) << ';' << task.GetOptimumValue() << ';' << dif << ';' << endl;
        cout << "#" << task_num << endl;
        cout << "Optimum is " << EnergyCalc(sa_ans) << ", we got " << task.GetOptimumValue() << ", the diff: " << dif << endl;
        cout << "Got: " << sa_ans << endl;
        cout << "Opt: " << true_ans << endl;
        cout << "dif: " << (sa_ans - task.GetOptimumPoint()) << endl;
    }
    file_ans << task_num << ';' << EnergyCalc(sa_ans) << ';' << task.GetOptimumValue() << ';' << dif << ';'
             << sa_ans.toRawString() << ';' << true_ans.toRawString() << endl;


    return dif;
}

// 268 - two min's
int main() {
    cout << KMAX << endl;

    std::ofstream file_ans;
    file_ans.open("./statsSA/ans.txt");
    file_ans << "  #  ;  SA_E  ;  OPT_E  ;  dif  ; SA_Point  ;  OPT_Point";
    task_run(TASK_NUM, KMAX, RUN_ALL_TASK, file_ans);
    file_ans.close();

    return 0;
}

