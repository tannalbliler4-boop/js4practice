#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>

// Performs matrix x vector multiplication: y = A * x
void matrix_vector_multiply(const std::vector<double>& A, const std::vector<double>& x, std::vector<double>& y, size_t rows, size_t cols) {
    // A is stored in row-major order: index = r * cols + c
    for (size_t r = 0; r < rows; ++r) {
        double sum = 0.0;
        size_t row_offset = r * cols;
        for (size_t c = 0; c < cols; ++c) {
            sum += A[row_offset + c] * x[c];
        }
        y[r] = sum;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <rows> <cols>\n";
        return 1;
    }

    size_t rows = std::stoull(argv[1]);
    size_t cols = std::stoull(argv[2]);

    std::cout << "Initializing " << rows << " x " << cols << " matrix multiplication...\n";

    // Allocate memory
    std::vector<double> A(rows * cols, 1.0); // Contiguous matrix
    std::vector<double> x(cols, 2.0);        // Input vector
    std::vector<double> y(rows, 0.0);        // Output vector

    // Warm up and multiply
    auto start = std::chrono::high_resolution_clock::now();
    matrix_vector_multiply(A, x, y, rows, cols);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Execution time: " << elapsed.count() << " seconds\n";

    // Dummy print to prevent aggressive compiler optimization removing the loop
    std::cout << "Result sample (y[0]): " << y[0] << "\n";

    return 0;
}
