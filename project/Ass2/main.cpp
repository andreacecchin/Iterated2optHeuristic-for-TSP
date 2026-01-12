#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

#include "TSPInstance.h"
#include "TSPHeuristic.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string instance_filter = "all";
    if (argc >= 2){
        instance_filter = argv[1];   // i.e. "10", "20", "30", "50", "70", "80", "100", "all"
    }

    // the folder where all test samples are located
    std::string data_folder = "./data";
    // the folder where all solution to tests will be located
    fs::create_directories("./data/solution");

    // setup for the solution/report
    std::string csv_name = "./data/solution/results_" + instance_filter + ".csv";
    std::ofstream csv(csv_name);
    if (!csv.is_open()) {
        std::cerr << "Cannot open CSV file: " << csv_name << std::endl;
        return 1;
    }
    csv << "instance,n,obj_value,solving_time\n";

    // when this program is run, all data of our instance_filter are tested one by one
    for (const auto& entry : fs::directory_iterator(data_folder)) {

        // we avoid to select possible files different from the one we want with .dat extension, and also the /generator folder
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".dat") continue;

        std::string filename = entry.path().string();
        std::string fname = entry.path().filename().string();
        if (instance_filter != "all") {
            std::string key = "instance_" + instance_filter + "_";
            if (fname.find(key) == std::string::npos) continue;
        }
        std::cout << "Processing instance: " << fname << std::endl;

        TSPInstance instance;
        try {
            instance = TSPInstance::readFromFile(filename);
        } catch (const std::exception& e) {
            std::cerr << "Error reading instance: " << e.what() << std::endl;
            continue;
        }

        TSPHeuristic model(instance);

        try {
                model.solve();
            } catch (const std::exception& e) {
                std::cerr << "Error solving model: " << e.what() << std::endl;
                break;
            }

        double objValue = model.getObjValue();
        double solvingTime = model.getSolvingTime();
        auto tour = model.getTour();

        std::cout << "  Feasible solution found with objValue ";
        std::cout << objValue;
        std::cout << " with solving time (sec) ";
        std::cout << solvingTime;
        std::cout << "\n  Solution (Tour): ";
        for (int v : tour) std::cout << v << " ";
            std::cout << "\n";

        std::ostringstream tour_ss;
        std::string tour_str;
        // Convert tour to string for our csv
        for (size_t i = 0; i < tour.size(); ++i) {
            tour_ss << tour[i];
            if (i != tour.size() - 1) tour_ss << "-";
        }
        tour_str = tour_ss.str();

        csv << fname << ","
            << instance.n << ","
            << model.getObjValue() << ","
            << model.getSolvingTime() << ","
            << tour_str << "\n";
        csv.flush();

    }

    csv.close();
    return 0;
}