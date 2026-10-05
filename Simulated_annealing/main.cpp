#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <fstream>
#include <random>

#include "sa_heads/MultiDimSA.h"

#include "Hill/HillProblem.hpp"
#include "GKLS/GKLSConstrainedProblem.hpp"
#include "Shekel/ShekelProblem.hpp"
#include "Grishagin/GrishaginConstrainedProblem.hpp"
#include "Grishagin/grishagin_function.hpp"
#include "GKLS/GKLSProblem.hpp"

using namespace std;

const int TASK_NUM = 10;
const int DIM = 2;
const unsigned KMAX = 10000; // pow(10, 6);

// const size_t STARTS_NUM = 1;
// const bool RUN_ALL_TASK = false; 
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
    
    MultiDimSA sa(KMAX, lowBound, upperBound, EnergyCalc);
    MathVec<double> ans = sa.run(KMAX, false);

    // vector<double> ans = SA(low_bounds, upper_bounds, Kmax, EnergyCalc, all_tasks_run);
    // for (size_t i = 0; i < N-1; i++) {
    //     vector<double> tmp_ans = SA(low_bounds, upper_bounds, Kmax, EnergyCalc, all_tasks_run);
    //     if (EnergyCalc(ans) > EnergyCalc(tmp_ans))
    //     ans = tmp_ans;
    // }
    
    dif = fabs(EnergyCalc(ans) - task.GetOptimumValue());
    
    // std::cout << task_num << '\n';
    // if (!all_tasks_run) {
    //     cout << task_num << endl;
    //     cout << EnergyCalc(ans) << "   " << task.GetOptimumValue() << "   " << dif << endl;
    //     cout << ans << endl;
    //     cout << task.GetOptimumPoint() << endl;
    //     cout << (ans - task.GetOptimumPoint()) << endl;
    // }
    file_ans << task_num << ';' << EnergyCalc(ans) << ';' << task.GetOptimumValue() << ';' << dif << ';';
    cout << endl << task_num << ';' << EnergyCalc(ans) << ';' << task.GetOptimumValue() << ';' << dif << ';' << endl;
    
    return dif;
}

// 268 - two min's
int main() {
    cout << KMAX << endl;
    const int task_num = TASK_NUM;
    const unsigned kMax = KMAX;


    std::ofstream file_ans;
    file_ans.open("./statsSA/ans.txt");
    task_run(task_num, kMax, false, file_ans);
    file_ans.close();

    return 0;
}

