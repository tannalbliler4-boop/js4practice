#include <iostream>
#include <random>
#include <iomanip>
#include <stdexcept> // Added for handling standard exceptions

// Global tracking size parameters for standard visible terminal printing layouts
const int ROWS = 25;
const int COLS = 40;

// --- DYNAMIC MEMORY MANAGEMENT HELPERS WITH SECURE EXCEPTION HANDLING ---
int** allocateMatrix(int r, int c) {
    // Secure strategy: Validate array dimensions before hitting memory allocation loops
    if (r <= 0 || c <= 0) {
        throw std::invalid_argument("Matrix dimensions must be strictly positive.");
    }

    int** mat = new int*[r];
    for (int i = 0; i < r; ++i) {
        mat[i] = nullptr; // Track safely to avoid dangling pointers if a row fails down the line
    }

    try {
        for (int i = 0; i < r; ++i) {
            mat[i] = new int[c](); // Initialized to 0
        }
    } catch (const std::bad_alloc& e) {
        // Secure strategy: Rollback cleanup already allocated pointers to prevent memory leaks
        for (int i = 0; i < r; ++i) {
            if (mat[i] != nullptr) delete[] mat[i];
        }
        delete[] mat;
        throw; // Escalate failure up to main level safely
    }
    return mat;
}

void freeMatrix(int** mat, int r) {
    if (mat != nullptr) {
        for (int i = 0; i < r; ++i) delete[] mat[i];
        delete[] mat;
    }
}

// --- SIMPLIFIED UNION-FIND OPERATIONS ---
int findRoot(int* parent, int i) {
    int root = i;
    while (root != parent[root]) {
        root = parent[root];
    }
    // Path compression for execution efficiency
    int curr = i;
    while (curr != root) {
        int next = parent[curr];
        parent[curr] = root;
        curr = next;
    }
    return root;
}

// Merges two groups keeping the smaller label value as the root
void unionLabels(int* parent, int i, int j) {
    int root_i = findRoot(parent, i);
    int root_j = findRoot(parent, j);
    if (root_i != root_j && root_i > 0 && root_j > 0) {
        if (root_i < root_j) parent[root_j] = root_i;
        else parent[root_i] = root_j;
    }
}

// --- TWO-PASS LABELING ENGINE ---
int** labelComponents(int** input, int r, int c) {
    int** output = allocateMatrix(r, c);
    
    // Max possible labels cannot exceed total array size area
    int max_labels = r * c;
    int* parent = nullptr;
    
    try {
        parent = new int[max_labels + 1](); 
    } catch (const std::bad_alloc&) {
        freeMatrix(output, r); // Clean up main matrix to prevent memory leaks if helper array fails
        throw;
    }
    
    int next_label = 1;

    // PASS 1: Assign preliminary labels and record equivalence links
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (input[i][j] == 1) {
                int up = (i > 0) ? output[i - 1][j] : 0;
                int left = (j > 0) ? output[i][j - 1] : 0;

                if (up == 0 && left == 0) {
                    output[i][j] = next_label;
                    parent[next_label] = next_label;
                    next_label++;
                } else if (up > 0 && left == 0) {
                    output[i][j] = up;
                } else if (left > 0 && up == 0) {
                    output[i][j] = left;
                } else { // Both neighbors have valid positive labels
                    output[i][j] = (up < left) ? up : left;
                    unionLabels(parent, up, left);
                }
            }
        }
    }

    // PASS 2: Resolve equivalence trees to final component root labels
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (output[i][j] > 0) {
                output[i][j] = findRoot(parent, output[i][j]);
            }
        }
    }

    delete[] parent;
    return output;
}

// --- VISUALIZATION OUTPUT INTERFACE ---
void printMatrix(int** mat, int r, int c) {
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (mat[i][j] == 0) std::cout << " . ";
            else std::cout << std::setw(2) << mat[i][j] << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    try {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 1);

        // 1. GENERATE & RUN RANDOM IMAGE A
        int** imgA = allocateMatrix(ROWS, COLS);
        for (int i = 0; i < ROWS; ++i)
            for (int j = 0; j < COLS; ++j)
                imgA[i][j] = (dist(gen) && dist(gen)) ? 1 : 0; // Dense balancing layout

        std::cout << "=== EXECUTING MATRIX RUN ON RANDOM IMAGE A ===\n";
        int** resA = labelComponents(imgA, ROWS, COLS);
        printMatrix(resA, ROWS, COLS);
        freeMatrix(imgA, ROWS); freeMatrix(resA, ROWS);

        // 2. GENERATE & RUN RANDOM IMAGE B
        int** imgB = allocateMatrix(ROWS, COLS);
        for (int i = 0; i < ROWS; ++i)
            for (int j = 0; j < COLS; ++j)
                imgB[i][j] = (dist(gen) && dist(gen)) ? 1 : 0;

        std::cout << "\n=== EXECUTING MATRIX RUN ON RANDOM IMAGE B ===\n";
        int** resB = labelComponents(imgB, ROWS, COLS);
        printMatrix(resB, ROWS, COLS);
        freeMatrix(imgB, ROWS); freeMatrix(resB, ROWS);

        // 3. EXECUTE REAL-WORLD SIMULATED PHOTO INPUT (MAPPED GEOMETRIC MOCK)
        int pRows = 8, pCols = 15;
        int** photoImg = allocateMatrix(pRows, pCols);
        
        // Fixed dimensions syntax
        int staticLayout[8][15] = {
            {0,0,1,1,1,0,0,0,0,1,1,1,1,0,0},
            {0,0,1,1,1,0,0,0,0,1,1,1,1,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {1,1,1,0,0,0,1,1,0,0,0,1,1,1,1},
            {1,1,1,0,0,0,1,1,0,0,0,1,1,1,1},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,1,1,1,1,1,1,1,1,1,1,1,0,0},
            {0,0,1,1,1,1,1,1,1,1,1,1,1,0,0}
        };
        for (int i = 0; i < pRows; ++i)
            for (int j = 0; j < pCols; ++j)
                photoImg[i][j] = staticLayout[i][j];

        std::cout << "\n=== EXECUTING MATRIX RUN ON REAL PHOTOGRAPH SIMULATION ===\n";
        int** resPhoto = labelComponents(photoImg, pRows, pCols);
        printMatrix(resPhoto, pRows, pCols);
        freeMatrix(photoImg, pRows); freeMatrix(resPhoto, pRows);

    } catch (const std::exception& e) {
        std::cerr << "Secure Intercept Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
