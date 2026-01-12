#ifndef TSPHEURISTIC_H
#define TSPHEURISTIC_H

#include "TSPInstance.h"
#include <vector>
#include <chrono>

class TSPHeuristic {
public:
    explicit TSPHeuristic(const TSPInstance& instance);

    void solve();

    double getObjValue() const;
    double getSolvingTime() const;
    std::vector<int> getTour() const;

private:
    const TSPInstance& inst;
    int n;

    std::vector<int> tour;
    double obj_value;
    double solving_time;

    double tourLength(const std::vector<int>& t) const;
    void greedyInitialization();
    bool twoOptLongEdgeFirst();
};

#endif
