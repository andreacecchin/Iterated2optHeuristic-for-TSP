#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <filesystem>

using namespace std;

// Parameters in mm
static const double WIDTH = 100.0;  // board width
static const double HEIGHT = 100.0; // board height
// here we "set" as 2mm the diameter of each screw
static const double MARGIN = 5.0;   // distance from the margin to place the 4 external corner holes
static const double MIN_DIST = 3.0; // min distance from a point to another (at least 1mm space between each screw)

// number of nodes in our instances
static const vector<int> N_VALUES = {10, 20, 30, 50, 70, 80, 100};
// how many instances per n value
static const int INSTANCES_PER_N = 5;

// Max attempts to place a node (debug)
static const int MAX_ATTEMPTS = 10000;

struct Point {
    double x;
    double y;
};

double dist(const Point& a, const Point& b) {
    return hypot(a.x - b.x, a.y - b.y);
}

bool is_valid(const Point& p, const vector<Point>& points) {
    for (const auto& q : points) {
        if (dist(p, q) < MIN_DIST)
            return false;
    }
    return true;
}

vector<Point> generate_instance(int n, mt19937& rng) {
    vector<Point> points;

    // Fixed corner points
    points.push_back({MARGIN, MARGIN});
    points.push_back({WIDTH - MARGIN, MARGIN});
    points.push_back({WIDTH - MARGIN, HEIGHT - MARGIN});
    points.push_back({MARGIN, HEIGHT - MARGIN});

    // we want the other points to be within the area of the external ones
    uniform_real_distribution<double> dx(MARGIN, WIDTH - MARGIN);
    uniform_real_distribution<double> dy(MARGIN, HEIGHT - MARGIN);

    while ((int)points.size() < n) {
        bool placed = false;

        for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
            Point p{dx(rng), dy(rng)};
            if (is_valid(p, points)) {
                points.push_back(p);
                placed = true;
                break;
            }
        }

        if (!placed) {
            // just in case we want to generate samples with much more than 100 holes, debug here
            throw runtime_error("Failed to place a point. Try reducing n or MIN_DIST.");
        }
    }

    return points;
}

void write_instance(const string& filename, const vector<Point>& points) {
    ofstream out(filename);
    if (!out)
        throw runtime_error("Cannot open file: " + filename);

    out << points.size() << "\n";
    for (const auto& p : points) {
        out << p.x << " " << p.y << "\n";
    }
}

int main() {
    try {
        filesystem::create_directories("../");  // ensure /data exists

        random_device rd;
        mt19937 rng(rd());

        for (int n : N_VALUES) {
            for (int k = 1; k <= INSTANCES_PER_N; ++k) {
                string filename = "../instance_" + to_string(n) + "_" + to_string(k) + ".dat";
                auto points = generate_instance(n, rng);
                write_instance(filename, points);
                cout << "Generated " << filename << endl;
            }
        }

        cout << "All instances generated successfully." << endl;
    }
    catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }

    return 0;
}