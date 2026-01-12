#include "TSPHeuristic.h"
#include <algorithm>
#include <numeric>
#include <cmath>

TSPHeuristic::TSPHeuristic(const TSPInstance& instance):inst(instance),n(instance.n),obj_value(0.0),solving_time(0.0) {}

double TSPHeuristic::tourLength(const std::vector<int>& t) const {
    double sum = 0.0;
    for (size_t i = 0; i + 1 < t.size(); ++i) {
        sum += inst.cost[t[i]][t[i + 1]];
    }
    return sum;
}

// initialization of starting graph with Kruskal-like heuristic
// add the shortest edges while avoiding early cycles enforcing degree <= 2 at each node, finally close the tour
void TSPHeuristic::greedyInitialization() {

    // store edge with its weight
    struct Edge {
        int u, v;
        double w;
    };

    // generate all possible edges of the graph
    std::vector<Edge> edges;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            edges.push_back({i, j, inst.cost[i][j]});

    // sort edges by increasing length
    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) { return a.w < b.w; });
    // degree of nodes in the partial solution
    std::vector<int> degree(n, 0);
    
    // Union-Find structure to prevent cycles
    std::vector<int> parent(n);
    std::iota(parent.begin(), parent.end(), 0);
    // Find operation
    auto find = [&](int x) {
        while (parent[x] != x)
            x = parent[x] = parent[parent[x]];
        return x;
    };
    // Union operation
    auto unite = [&](int a, int b) {
        a = find(a);
        b = find(b);
        if (a != b) parent[b] = a;
    };

    std::vector<std::pair<int,int>> selected;

    for (const auto& e : edges) {
        // check not exceed degree 2
        if (degree[e.u] == 2 || degree[e.v] == 2) continue;
        // avoid creating a cycle before having n-1 edges
        if (find(e.u) == find(e.v) && selected.size() < n - 1) continue;

        selected.emplace_back(e.u, e.v);
        degree[e.u]++;
        degree[e.v]++;
        unite(e.u, e.v);

        // stop when we have a Hamiltonian path (n-1 edges)
        if (selected.size() == n - 1) break;
    }

    // search where to locate the last n-th edge to close the graph
    std::vector<int> endpoints;
    for (int i = 0; i < n; ++i)
        if (degree[i] == 1)  endpoints.push_back(i);
    // add the final edge
    selected.emplace_back(endpoints[0], endpoints[1]);

    // preparing adjacent list to reconstruct tour order
    std::vector<std::vector<int>> adj(n);
    for (auto& e : selected) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    // reconstruct tour
    tour.clear();
    tour.push_back(0);
    int prev = -1, curr = 0;

    while (true) {
        int next;
        // each node has two neighbors: choose the one not visited previously
        if (adj[curr][0] != prev) {
            next = adj[curr][0];
        } else {
            next = adj[curr][1];
        }
        // if we return to the start node 0, the tour is complete
        if (next == tour[0]) break;
        tour.push_back(next);
        prev = curr;
        curr = next;
    }
    tour.push_back(tour[0]);
}


// iterated 2-opt with arcs sorted by their lenght
// tt each iteration, the longest edges of the current tour are considered first
// the first improving 2-opt move is immediately accepted and another 2-opt iteration is started
bool TSPHeuristic::twoOptLongEdgeFirst() {
    bool improved = false;
    int m = n;

    // i corresponds to removing edge (tour[i-1], tour[i])
    struct Cut {
        int i;          // index of the cut
        double length;  // length of the edge to be removed
    };

    // list of all edges of the current tour
    std::vector<Cut> cuts;
    for (int i = 1; i < m - 1; ++i) {
        cuts.push_back({i, inst.cost[tour[i-1]][tour[i]]});
    }
    // sort edges in descending order of length
    // for our heuristic long edges are tested first, as they are more likely to yield better improvements
    std::sort(cuts.begin(), cuts.end(), [](const Cut& a, const Cut& b) { return a.length > b.length; });
    // cost of the current tour
    double best = tourLength(tour);
    // try all 2-opt moves starting from the longest edges
    for (const auto& c : cuts) {
        // first cut position
        int i = c.i;
        // second cut position
        for (int j = i + 1; j < m; ++j) {
            // create a candidate tour by reversing the segment [i, j]
            std::vector<int> candidate = tour;
            std::reverse(candidate.begin() + i, candidate.begin() + j + 1);
            // cost of th new tour
            double cand_cost = tourLength(candidate);
            if (cand_cost < best) {
                tour = candidate;
                improved = true;
                return true;   // first improvement is accepted as new versione for the graph, another 2-opt iteration will be started
            }
        }
    }
    // if a not improving 2-opt move is found, we return the previous iteration result
    return improved;
}

void TSPHeuristic::solve() {
    auto start = std::chrono::high_resolution_clock::now();
    greedyInitialization();

    while (twoOptLongEdgeFirst()) {
        // twoOptLongEdgeFirst() is repeatedly called untill an improvement using a 2-opt is no more possible 
    }

    obj_value = tourLength(tour);
    auto end = std::chrono::high_resolution_clock::now();
    solving_time = std::chrono::duration<double>(end - start).count();
}


double TSPHeuristic::getObjValue() const 
{
    return obj_value;
}

double TSPHeuristic::getSolvingTime() const 
{
    return solving_time;
}

std::vector<int> TSPHeuristic::getTour() const 
{
    return tour;
}