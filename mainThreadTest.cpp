#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

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
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <n> <m> <k> <num_threads>" << std::endl;
        return 1;
    }

    int n = std::atoi(argv[1]);
    int m = std::atoi(argv[2]);
    int k = std::atoi(argv[3]);
    int num_threads = std::atoi(argv[4]);

    auto A = generate_matrix(n, m);
    auto B = generate_matrix(m, k);
    std::vector<std::vector<int>> C(n, std::vector<int>(k, 0));

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&A, &B, &C, i, n, k, num_threads]() {
            for (int row = i; row < n; row += num_threads) {
                for (int col = 0; col < k; ++col) {
                    multiply_row_by_column(A, B, C, row, col);
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << "Time taken for multiplication with " << num_threads << " threads: "
              << duration.count() << " milliseconds." << std::endl;

    return 0;
}