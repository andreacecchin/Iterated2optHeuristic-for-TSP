#include "cpxmacro.h"
#include "TSPModel.h"
#include <cstdio>
#include <iostream>

using namespace std;

static const int NAME_SIZE = 512;
static char name[NAME_SIZE];
char errmsg[BUF_SIZE];
int status;

TSPModel::TSPModel(const TSPInstance& instance): inst(instance),lp_status(-1),obj_value(0.0),solving_time(0.0),time_limit(0.0) {
    map_y.assign(inst.n, vector<int>(inst.n, -1));
    map_x.assign(inst.n, vector<int>(inst.n, -1));
}

void TSPModel::setupLP(CEnv env, Prob lp) {
    int position = 0;
    int n = inst.n;
    const vector<vector<double>>& c = inst.cost;

    // x_ij continuous, only if i != j AND j != 0
    for (int i = 0; i < n; i++) {
        for (int j = 1; j < n; j++) {
            if (i != j) {
                char type = 'C'; 
                double lb = 0.0;
                double ub = CPX_INFBOUND;
                double obj = 0.0;
                snprintf(name, NAME_SIZE, "x_%d_%d", i, j);
                char* cname = &name[0];

                CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &type, &cname);

                map_x[i][j] = position;
                position++;
            }
        }
    }

    // y_ij binary, only if i != j
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j){
                char type = 'B'; 
                double lb = 0.0;
                double ub = 1.0;
                double obj = c[i][j];
                snprintf(name, NAME_SIZE, "y_%d_%d", i, j);
                char* cname = &name[0];

                CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &type, &cname);

                map_y[i][j] = position;
                position++;
            }
        }
    }

    // sum_j y_ij = 1   ∀ i ∈ N
    for (int i = 0; i < n; i++) {
        vector<int> idx;
        vector<double> coef;
        for (int j = 0; j < n; j++) {
            if (i != j && map_y[i][j] >= 0) {
                idx.push_back(map_y[i][j]);
                coef.push_back(1.0);
            }
        }
        char sense = 'E'; // =
        double rhs = 1.0;
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }

    // sum_i y_ij = 1   ∀ j ∈ N
    for (int j = 0; j < n; j++) {
        vector<int> idx;
        vector<double> coef;
        for (int i = 0; i < n; i++) {
            if (i != j && map_y[i][j] >= 0) {
                idx.push_back(map_y[i][j]);
                coef.push_back(1.0);
            }
        }
        char sense = 'E'; // =
        double rhs = 1.0;
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }

    // sum_i x_ik − sum_j x_kj = 1   ∀ k ∈ N \ {0}
    for (int k = 1; k < n; k++) {
        vector<int> idx;
        vector<double> coef;

        // incoming flow
        for (int i = 0; i < n; i++) {
            if (map_x[i][k] >= 0) {
                idx.push_back(map_x[i][k]);
                coef.push_back(1.0);
            }
        }

        // outgoing flow j != 0
        for (int j = 1; j < n; j++) {
            if (map_x[k][j] >= 0) {
                idx.push_back(map_x[k][j]);
                coef.push_back(-1.0);
            }
        }
        char sense = 'E'; // =
        double rhs = 1.0;
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }

    // x_ij ≤ (n − 1) y_ij   ∀ i ≠ j, j ≠ 0
    // it is equal to x_ij - (n − 1) y_ij ≤ 0   ∀ i ≠ j, j ≠ 0
    for (int i = 0; i < n; i++) {
        for (int j = 1; j < n; j++) {
            if (i != j && map_x[i][j] >= 0 && map_y[i][j] >= 0) {
                vector<int> idx(2);
                vector<double> coef(2);

                idx[0] = map_x[i][j];   // x_ij
                coef[0] = 1.0;
                idx[1] = map_y[i][j];   // y_ij
                coef[1] = -(n - 1);

                char sense = 'L'; // ≤
                double rhs = 0.0;
                int matbeg = 0;
                CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, 2, &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
            }
        }
    }
    CHECKED_CPX_CALL(CPXwriteprob, env, lp, "tsp_CPX_CALL.lp", NULL);
}

bool TSPModel::solve() {
    DECL_ENV(env);
    DECL_PROB(env, lp);

    if (time_limit > 0.0){
        CHECKED_CPX_CALL(CPXsetdblparam, env, CPX_PARAM_TILIM, time_limit);
    }

    setupLP(env, lp);
    double t1, t2;
    CPXgettime(env, &t1);
    CHECKED_CPX_CALL(CPXmipopt, env, lp);
    CPXgettime(env, &t2);

    solving_time = t2 - t1;
    lp_status = CPXgetstat(env, lp);

    if (lp_status == CPXMIP_OPTIMAL || lp_status == CPXMIP_OPTIMAL_TOL || lp_status == CPXMIP_TIME_LIM_FEAS){
        CHECKED_CPX_CALL(CPXgetobjval, env, lp, &obj_value);
        extractTour(env, lp);
    }

    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

    return true;
}

void TSPModel::extractTour(CEnv env, Prob lp) {
    int ncols = CPXgetnumcols(env, lp);
    vector<double> vals(ncols);

    CHECKED_CPX_CALL(
        CPXgetx, env, lp,
        vals.data(), 0, ncols - 1
    );

    tour.clear();
    tour.reserve(inst.n + 1);

    int current = 0;
    tour.push_back(0);

    vector<bool> visited(inst.n, false);
    visited[current] = true;

    while ((int)tour.size() < inst.n) {
        bool found_next = false;
        for (int j = 0; j < inst.n; ++j) {
            int idx = map_y[current][j];
            if (idx >= 0 && vals[idx] > 0.5 && !visited[j]) {
                tour.push_back(j);
                visited[j] = true;
                current = j;
                found_next = true;
                break;
            }
        }
        if (!found_next) {
            std::cerr << "Error: unable to extract a valid tour from solution!" << std::endl;
            tour.clear();
            return;
        }
    }
    tour.push_back(0);
}

int TSPModel::getStatus() const
{
    return lp_status;
}

double TSPModel::getObjValue() const
{
    return obj_value;
}

double TSPModel::getSolvingTime() const
{
    return solving_time;
}

std::vector<int> TSPModel::getTour() const
{
    return tour;
}

void TSPModel::setTimeLimit(double seconds)
{
    time_limit = seconds;
}