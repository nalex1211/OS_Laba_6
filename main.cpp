#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <cstdlib>

std::mutex mtx;

std::vector<std::vector<int>> generate_matrix(int rows, int cols) {
    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 9);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = distrib(gen);
        }
    }

    return matrix;
}

void multiply_row_by_column(const std::vector<std::vector<int>>& A,
                            const std::vector<std::vector<int>>& B,
                            std::vector<std::vector<int>>& C,
                            int row, int col) {
    int result = 0;
    for (int i = 0; i < B.size(); ++i) {
        result += A[row][i] * B[i][col];
    }

    mtx.lock();
    C[row][col] = result;
    std::cout << "[" << row << "," << col << "]=" << result << std::endl;
    mtx.unlock();
}

void print_matrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto &row : matrix) {
        for (int elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <n> <m> <k>" << std::endl;
        return 1;
    }

    const int n = std::atoi(argv[1]);
    const int m = std::atoi(argv[2]);
    const int k = std::atoi(argv[3]);

    auto A = generate_matrix(n, m);
    auto B = generate_matrix(m, k);
    std::vector<std::vector<int>> C(n, std::vector<int>(k, 0));

    std::cout << "Matrix A:" << std::endl;
    print_matrix(A);
    std::cout << "Matrix B:" << std::endl;
    print_matrix(B);

    std::vector<std::thread> threads;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            threads.emplace_back(std::thread(multiply_row_by_column, std::ref(A), std::ref(B), std::ref(C), i, j));
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Matrix C (Result):" << std::endl;
    print_matrix(C);

    return 0;
}