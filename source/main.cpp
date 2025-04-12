#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <mutex>
#include <chrono>
#include <cstdlib>  // for std::atoi
#include <climits>  // for INT_MAX

// ---------------------------
// Data Structure Definitions
// ---------------------------

// Each room has a capacity.
struct Room {
    int capacity;  
};

// Represents a pair of students that are incompatible
// (i.e. should ideally not be assigned to the same room).
struct Incompatibility {
    int student1;
    int student2;
};

// -----------------------------
// Global Variables (Shared Data)
// -----------------------------

// A mutex to protect updating the global best solution.
std::mutex bestSolutionMutex;

// Global best fitness found so far (lower is better).
int globalBestFitness = INT_MAX;

// Global assignment vector for the best solution found.
// Each index represents a student and the value is the assigned room.
std::vector<int> globalBestAssignment;

// ---------------------------
// Evaluation Function
// ---------------------------
// Given an assignment of students to rooms, this function computes a fitness
// score. Violations of constraints (room capacity, incompatibilities) increase the penalty.
int evaluateSolution(const std::vector<int>& assignment,
                     const std::vector<Room>& rooms,
                     const std::vector<Incompatibility>& incompatibilities)
{
    int penalty = 0;
    std::vector<int> roomUsage(rooms.size(), 0);

    // Calculate the number of students assigned to each room.
    for (int room : assignment) {
        if (room >= 0 && room < static_cast<int>(roomUsage.size())) {
            roomUsage[room]++;
        }
    }
    
    // Penalty for exceeding room capacity.
    for (size_t i = 0; i < rooms.size(); i++) {
        if (roomUsage[i] > rooms[i].capacity) {
            // Each extra student in a room adds a penalty multiplier.
            penalty += (roomUsage[i] - rooms[i].capacity) * 10;
        }
    }
    
    // Penalty for placing incompatible students in the same room.
    for (const auto& inc : incompatibilities) {
        if (assignment[inc.student1] == assignment[inc.student2]) {
            penalty += 5;
        }
    }
    
    return penalty;
}

// ---------------------------
// Monte Carlo Worker Function
// ---------------------------
// Each thread repeatedly generates random assignments (solutions) and evaluates
// them. If a new solution’s fitness is better than the current global best,
// it updates the global best solution (with thread-safety).
void monteCarloThread(int iterations,
                      const std::vector<Room>& rooms,
                      const std::vector<Incompatibility>& incompatibilities,
                      int numStudents,
                      int threadId)
{
    // Thread-local random number generator seeded uniquely.
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> roomDist(0, rooms.size() - 1);

    std::vector<int> currentAssignment(numStudents, 0);

    for (int i = 0; i < iterations; i++) {
        // Generate a random assignment: each student is assigned a room at random.
        for (int j = 0; j < numStudents; j++) {
            currentAssignment[j] = roomDist(rng);
        }
        
        // Calculate the fitness (penalty) for the generated solution.
        int fitness = evaluateSolution(currentAssignment, rooms, incompatibilities);
        
        // Use a mutex to safely update the global best solution.
        {
            std::lock_guard<std::mutex> lock(bestSolutionMutex);
            if (fitness < globalBestFitness) {
                globalBestFitness = fitness;
                globalBestAssignment = currentAssignment;
                
                // Optionally print an improvement message.
                // std::cout << "Thread " << threadId << " found new best fitness: " << fitness << "\n";
            }
        }
    }
}

// ---------------------------
// Main Function & Benchmarking
// ---------------------------
int main(int argc, char* argv[])
{
    // ---------------------------
    // Dynamic Thread Count Setup
    // ---------------------------
    // Allow the user to specify the number of threads via command-line.
    // Default is 4 threads if no argument is given.
    int threadCount = 4;
    if (argc > 1) {
        threadCount = std::atoi(argv[1]);
    }
    
    // ---------------------------------
    // Problem Configuration Parameters
    // ---------------------------------
    int numRooms = 10;       // total rooms available
    int numStudents = 100;   // total students to assign
    
    // Create a vector of rooms, each with a given capacity.
    std::vector<Room> rooms(numRooms, Room{10});  // Each room has a capacity of 10
    // (If desired, you can vary capacity, e.g., alternating capacities.)
    
    // Define incompatibility constraints.
    std::vector<Incompatibility> incompatibilities;
    // For demonstration: every third pair of adjacent students are incompatible.
    for (int i = 0; i < numStudents - 1; i += 3) {
        incompatibilities.push_back({i, i+1});
    }
    
    // Number of iterations per thread (controls the number of random solutions generated).
    int iterationsPerThread = 100000;

    // ---------------------------
    // Benchmarking: Start Timer
    // ---------------------------
    auto startTime = std::chrono::steady_clock::now();
    
    // ---------------------------
    // Create and Start Threads
    // ---------------------------
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++) {
        threads.emplace_back(monteCarloThread,
                             iterationsPerThread,
                             std::cref(rooms),
                             std::cref(incompatibilities),
                             numStudents,
                             t);
    }
    
    // Wait for all threads to complete their work.
    for (auto& t : threads) {
        t.join();
    }
    
    // ---------------------------
    // Benchmarking: End Timer
    // ---------------------------
    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // ---------------------------
    // Reporting the Results
    // ---------------------------
    std::cout << "\nBest Room Assignment per Student:\n";
    for (int i = 0; i < numStudents; i++) {
        std::cout << "Student " << i << ": Room " << globalBestAssignment[i] << "\n";
    }

    std::cout << "Best fitness found: " << globalBestFitness << "\n";
    std::cout << "Total Time taken (ms): " << elapsed << "\n";
    std::cout << "Thread Count used: " << threadCount << "\n";

    return 0;
}