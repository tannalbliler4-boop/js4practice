#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <map>

// Secure Disjoint Set implementation with Path Compression
class DisjointSet {
private:
    std::map<int, int> parent;
public:
    void add(int i) {
        if (parent.find(i) == parent.end()) {
            parent[i] = i;
        }
    }

    int find(int i) {
        if (parent.find(i) == parent.end()) {
            return 0; 
        }
        // Path compression via iterative gathering
        int root = i;
        while (root != parent[root]) {
            root = parent[root];
        }
        int curr = i;
        while (curr != root) {
            int nxt = parent[curr];
            parent[curr] = root;
            curr = nxt;
        }
        return root;
    }

    void merge(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j && root_i != 0 && root_j != 0) {
            // Tie-break with smaller label value
            if (root_i < root_j) {
                parent[root_j] = root_i;
            } else {
                parent[root_i] = root_j;
            }
        }
    }
};

// Main processing engine using standard vectors
std::vector<std::vector<int>> labelComponents(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty() || matrix[0].empty()) return {};
    
    size_t rows = matrix.size();
    size_t cols = matrix[0].size();
    std::vector<std::vector<int>> output(rows, std::vector<int>(cols, 0));
    DisjointSet ds;
    int next_label = 1;

    // --- PASS 1 ---
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            if (matrix[r][c] == 1) {
                std::vector<int> neighbors;

                // Secure bounds verification for upper neighbor
                if (r > 0 && output[r - 1][c] > 0) {
                    neighbors.push_back(output[r - 1][c]);
                }
                // Secure bounds verification for left neighbor
                if (c > 0 && output[r][c - 1] > 0) {
                    neighbors.push_back(output[r][c - 1]);
                }

                if (neighbors.empty()) {
                    output[r][c] = next_label;
                    ds.add(next_label);
                    next_label++;
                } else {
                    int min_label = *std::min_element(neighbors.begin(), neighbors.end());
                    output[r][c] = min_label;
                    for (int lbl : neighbors) {
                        ds.merge(min_label, lbl);
                    }
                }
            }
        }
    }

    // --- PASS 2 ---
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            if (output[r][c] > 0) {
                output[r][c] = ds.find(output[r][c]);
            }
        }
    }
    return output;
}

// System formatting helper
void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            if (val == 0) {
                std::cout << " . "; // Clear visual representation for 0-spaces
            } else {
                std::cout << std::setw(2) << val << " ";
            }
        }
        std::cout << "\n";
    }
}

int main() {
    // Generate programmatic mock targets mirroring physical dimensions
    const size_t N = 25; 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);

    // Image Set A generation
    std::vector<std::vector<int>> imgA(N, std::vector<int>(N));
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            imgA[i][j] = distrib(gen);

    std::cout << "=== RUNNING SIMULATION ON RANDOM IMAGE A ===\n";
    auto labeledA = labelComponents(imgA);
    printMatrix(labeledA);

    // Real-world simulated Photo Input with distinct geometric clusters
    std::vector<std::vector<int>> photoImg = {
        {0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0},
        {0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0},
        {0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
        {1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1}
    };

    std::cout << "\n=== RUNNING SIMULATION ON PHOTO-CONVERTED GEOMETRIC IMAGE ===\n";
    auto labeledPhoto = labelComponents(photoImg);
    printMatrix(labeledPhoto);

    return 0;
}
