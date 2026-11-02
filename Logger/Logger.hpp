#pragma once

#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


template <class Vec>
struct SAStep {
    size_t k;          // 1-based iteration number
    double temp;
    double e_cur;      // energy of the current state AFTER accept/reject
    double e_cand;     // energy of the proposed state
    double e_best;
    bool   accepted;
    bool   new_best;
    const Vec& cur;    // current state after accept/reject
    const Vec& cand;   // proposed state
    int    chain = 0;  // independent chain / thread id (0 for single-chain SA)
};


template <class Vec>
class SAObserver {
public:
    virtual ~SAObserver() = default;

    // single-chain SA
    virtual void onStart(const Vec& /*init*/, double /*e_init*/, size_t /*k_max*/) {}

    virtual void onChainsBegin(int /*n_chains*/) {}
    virtual void onChainStart(int /*chain*/, const Vec& /*init*/,
                              double /*e_init*/, size_t /*k_max*/) {}

    virtual void onStep(const SAStep<Vec>& /*step*/) {}
};


// Logger
struct LoggerConfig {
    std::string dir          = "./stats";
    bool        trace_steps  = false;  // per-iteration trace (single-task mode)
    size_t      trace_stride = 1;      // write every N-th iteration
    bool        echo_console = false;  // print the task summary to stdout
};


template <class Vec>
class Logger : public SAObserver<Vec> {
    using clock = std::chrono::steady_clock;

public:
    explicit Logger(LoggerConfig cfg = {}) : cfg_(std::move(cfg)) {
        if (cfg_.trace_stride == 0) cfg_.trace_stride = 1;
        
        std::filesystem::create_directories(cfg_.dir);

        summary_.open(cfg_.dir + "/ans.csv");
        if (!summary_)
            throw std::runtime_error("Logger: cannot open " + cfg_.dir + "/ans.csv");
            
        summary_ << std::setprecision(12);
        summary_ << "#;SA_E;OPT_E;dif;time_s;SA_Point;OPT_Point\n";
    }

    // per-task API 
    void beginTask(int task_num) {
        task_ = task_num;
        t0_   = clock::now();

        closeTraces();
        if (cfg_.trace_steps) openTraces(1);
    }

    // Returns |SA value - optimum value|.
    double endTask(const Vec& sa_pt, double sa_val, const Vec& opt_pt, double opt_val) {
        const double dif  = std::fabs(sa_val - opt_val);
        const double secs = std::chrono::duration<double>(clock::now() - t0_).count();

        summary_ << task_ << ';' << sa_val << ';' << opt_val << ';' << dif << ';'
                 << secs << ';' << sa_pt.toRawString() << ';' << opt_pt.toRawString() << '\n';

        if (cfg_.echo_console) {
            std::cout << "#" << task_ << '\n'
                      << "Optimum is " << opt_val << ", we got " << sa_val
                      << ", diff: " << dif << '\n'
                      << "Got: " << sa_pt << '\n'
                      << "Opt: " << opt_pt << '\n';
        }

        closeTraces();
        return dif;
    }

    // ---- SAObserver 
    void onStart(const Vec& init, double e_init, size_t /*k_max*/) override {
        writeStart(0, init, e_init);
    }

    void onChainsBegin(int n_chains) override {
        if (!cfg_.trace_steps || n_chains < 1) return;
        closeTraces();
        openTraces(n_chains);          // one file per chain, opened before threads start
    }

    void onChainStart(int chain, const Vec& init, double e_init, size_t /*k_max*/) override {
        writeStart(chain, init, e_init);
    }

    void onStep(const SAStep<Vec>& s) override {
        if (s.chain < 0 || static_cast<size_t>(s.chain) >= traces_.size()) return;
        if (s.k != 1 && s.k % cfg_.trace_stride != 0) return;

        // each chain writes only to its own stream -> no locking needed
        traces_[s.chain] << s.k << ';' << s.temp << ';' << s.e_cur << ';' << s.e_cand << ';'
                         << s.e_best << ';' << s.accepted << ';' << s.new_best << ';'
                         << s.cur.toRawString() << '\n';
    }

private:
    void openTraces(int n) {
        traces_.resize(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) {
            std::string name = cfg_.dir + "/steps_task_" + std::to_string(task_);
            if (n > 1) name += "_chain_" + std::to_string(i);
            name += ".csv";

            traces_[i].open(name);
            traces_[i] << std::setprecision(12);
            traces_[i] << "k;T;E_cur;E_cand;E_best;accepted;new_best;cur_point\n";
        }
    }

    void closeTraces() {
        for (auto& f : traces_)
            if (f.is_open()) f.close();
        traces_.clear();
    }

    void writeStart(int chain, const Vec& init, double e_init) {
        if (chain < 0 || static_cast<size_t>(chain) >= traces_.size()) return;
        // row k = 0: the starting point
        traces_[chain] << 0 << ';' << 0 << ';' << e_init << ';' << e_init << ';' << e_init
                       << ";1;1;" << init.toRawString() << '\n';
    }// /! innit temp skipped & inits with 0

    LoggerConfig  cfg_;
    std::ofstream summary_;
    std::vector<std::ofstream> traces_;   // [0] for single-chain, one per chain otherwise
    int           task_ = 0;
    clock::time_point t0_{};
};