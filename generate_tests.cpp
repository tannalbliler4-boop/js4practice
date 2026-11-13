#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>

// Helper function to write a vector of numbers to a text file
void saveToFile(const std::string& filename, const std::vector<int>& data) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error: Could not open " << filename << " for writing.\n";
        return;
    }

    for (size_t i = 0; i < data.size(); ++i) {
        outFile << data[i] << "\n";
    }
    
    outFile.close();
    std::cout << "Created: " << filename << " (" << data.size() << " numbers)\n";
}

int main() {
    // Dataset sizes defined in the Testing Design Document
    std::vector<int> sizes = {10, 100, 1000, 10000};

    // Set up modern random number generation (fixed seed for reproducible results)
    std::mt19937 rng(42); 
    std::uniform_int_distribution<int> dist(1, 100000);
    std::uniform_int_distribution<int> dupDist(1, 10); // Small range forces lots of duplicates

    std::cout << "Generating test files...\n\n";

    for (int n : sizes) {
        std::string sizeStr = std::to_string(n);

        // 1. Random Numbers (Test 1 & Test 5)
        std::vector<int> randomData(n);
        for (int i = 0; i < n; ++i) {
            randomData[i] = dist(rng);
        }
        saveToFile("random_" + sizeStr + ".txt", randomData);

        // 2. Already Sorted Numbers (Test 2)
        std::vector<int> sortedData = randomData;
        std::sort(sortedData.begin(), sortedData.end());
        saveToFile("sorted_" + sizeStr + ".txt", sortedData);

        // 3. Reverse-Sorted Numbers (Test 3)
        std::vector<int> reverseData = sortedData;
        std::reverse(reverseData.begin(), reverseData.end());
        saveToFile("reverse_" + sizeStr + ".txt", reverseData);

        // 4. Numbers with Heavy Duplicates (Test 4)
        std::vector<int> duplicateData(n);
        for (int i = 0; i < n; ++i) {
            duplicateData[i] = dupDist(rng);
        }
        saveToFile("duplicates_" + sizeStr + ".txt", duplicateData);
        
        std::cout << "----------------------------------------\n";
    }

    std::cout << "\nAll 16 test files successfully generated!\n";
    return 0;
}
