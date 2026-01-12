#include "TSPInstance.h"
#include <fstream>
#include <cmath>
#include <stdexcept>

struct Node {
    double x;
    double y;
};

TSPInstance TSPInstance::readFromFile(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file " + filename);
    }

    TSPInstance inst;
    fin >> inst.n;

    if (inst.n <= 1){
        throw std::runtime_error("Invalid number of nodes");
    }
    
    std::vector<Node> nodes(inst.n);

    for (int i = 0; i < inst.n; ++i) {
        fin >> nodes[i].x >> nodes[i].y;
        if (!fin) {
            throw std::runtime_error("Error reading coordinates in " + filename);
        }
    }

    inst.cost.assign(inst.n, std::vector<double>(inst.n, 0.0));

    for (int i = 0; i < inst.n; i++) {
        for (int j = 0; j < inst.n; j++) {
            if (i == j) continue;
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            inst.cost[i][j] = std::sqrt(dx * dx + dy * dy);
        }
    }

    return inst;
}