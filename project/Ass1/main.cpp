#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

#include "TSPInstance.h"
#include "TSPModel.h"
#include "cpxmacro.h" 

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string instance_filter = "all";
    if (argc >= 2){
        instance_filter = argv[1];   // e.g. "10", "20", "all"
    }

    // check for CPLEX
    try {
        DECL_ENV(env);
        CPXcloseCPLEX(&env);
    } catch (const std::exception& e) {
        std::cerr << "Cannot open CPLEX environment: " << e.what() << std::endl;
        return 1;
    }

    // the program will try to run the optimization with a certain time limit (first step, 1s)
    // if no optimal solution is found up to this limit, then the time will be increased for the next iteration
    // at the end, if no optimal solution is found in 5 minutes, the program will continue with another data/sample
    std::vector<double> time_limits = {
        1, 10, 20, 30, 60, 90, 120, 180, 240, 300
    };

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
    csv << "instance,n,time_limit,status,obj_value,solving_time\n";

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

        bool solved_optimal = false;

        for (double tl : time_limits) {

            std::cout << "  Time limit: " << tl << "s" << std::endl;

            TSPModel model(instance);
            model.setTimeLimit(tl);

            try {
                model.solve();
            } catch (const std::exception& e) {
                std::cerr << "Error solving model: " << e.what() << std::endl;
                break;
            }

            int status = model.getStatus();
            std::string status_str;

            double objValue = model.getObjValue();
            double solvingTime = model.getSolvingTime();
            auto tour = model.getTour();

            if (status == CPXMIP_OPTIMAL || status == CPXMIP_OPTIMAL_TOL) {
                // for each instance, we want to know the final objective value, the time required and the final solution
                status_str = "OPTIMAL";
                solved_optimal = true;
                std::cout << "  OPTIMAL solution found with objValue ";
                std::cout << objValue;
                std::cout << " with solving time (sec) ";
                std::cout << solvingTime;
                std::cout << "\n  Solution (Tour): ";
                for (int v : tour) std::cout << v << " ";
                std::cout << "\n";
            } else if (status == CPXMIP_TIME_LIM_FEAS) {
                status_str = "TIME_LIMIT";
                std::cout << "  Time limit reached (feasible solution with objValue ";
                std::cout << objValue;
                std::cout << " )\n";
            } else {
                status_str = "NO_SOLUTION";
                std::cout << "  No feasible solution\n";
            }

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
                << tl << ","
                << status_str << ","
                << model.getObjValue() << ","
                << model.getSolvingTime() << ","
                << tour_str << "\n";
            csv.flush();

            if (solved_optimal) break;
        }

        if (!solved_optimal) {
            std::cout << "  No optimal solution found within limits\n";
        }
    }

    csv.close();
    return 0;
}