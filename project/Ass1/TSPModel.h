#ifndef TSP_MODEL_H
#define TSP_MODEL_H

#include "cpxmacro.h"
#include "TSPInstance.h"
#include <vector>
#include <ilcplex/cplex.h>

//typedef CPXENVptr CEnv;
//typedef CPXLPptr  Prob;

class TSPModel {
public:
    explicit TSPModel(const TSPInstance& instance);

    void setTimeLimit(double seconds);
    bool solve();

    int getStatus() const;
    double getObjValue() const;
    double getSolvingTime() const;
    std::vector<int> getTour() const;

private:
    const TSPInstance& inst;

    std::vector<std::vector<int>> map_y;
    std::vector<std::vector<int>> map_x;

    int lp_status;

    double obj_value;
    double solving_time;
    std::vector<int> tour;

    double time_limit;

    void setupLP(CEnv env, Prob lp);
    void extractTour(CEnv env, Prob lp);
};

#endif
