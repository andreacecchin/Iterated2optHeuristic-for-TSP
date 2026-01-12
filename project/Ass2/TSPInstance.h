#ifndef TSPINSTANCE_H
#define TSPINSTANCE_H

#include <vector>
#include <string>

class TSPInstance {
public:
    int n;
    std::vector<std::vector<double>> cost;

    static TSPInstance readFromFile(const std::string& filename);
};

#endif
