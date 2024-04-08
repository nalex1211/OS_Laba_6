#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
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

void safe_increment(int& counter) {
    for (int i = 0; i < 1000000000; ++i) {
        std::lock_guard<std::mutex> guard(mtx);
        ++counter;
    }
}

void unsafe_increment(int& counter) {
    for (int i = 0; i < 1000000000; ++i) {
        ++counter;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <n> <m> <k> <num_threads_for_increment>" << std::endl;
        return 1;
    }

    const int n = std::atoi(argv[1]);
    const int m = std::atoi(argv[2]);
    const int k = std::atoi(argv[3]);
    int num_threads_for_increment = std::atoi(argv[4]);

    auto A = generate_matrix(n, m);
    auto B = generate_matrix(m, k);
    std::vector<std::vector<int>> C(n, std::vector<int>(k, 0));

    int safe_counter = 0, unsafe_counter = 0;
    std::vector<std::thread> increment_threads;

    auto start_safe = std::chrono::steady_clock::now();
    for (int i = 0; i < num_threads_for_increment; ++i) {
        increment_threads.emplace_back(safe_increment, std::ref(safe_counter));
    }
    for (auto& t : increment_threads) {
        t.join();
    }
    auto end_safe = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_safe = end_safe - start_safe;
    std::cout << "Safe increment result: " << safe_counter << ", Time: " << elapsed_safe.count() << " s" << std::endl;

    increment_threads.clear();
    auto start_unsafe = std::chrono::steady_clock::now();
    for (int i = 0; i < num_threads_for_increment; ++i) {
        increment_threads.emplace_back(unsafe_increment, std::ref(unsafe_counter));
    }
    for (auto& t : increment_threads) {
        t.join();
    }
    auto end_unsafe = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_unsafe = end_unsafe - start_unsafe;
    std::cout << "Unsafe increment result: " << unsafe_counter << ", Time: " << elapsed_unsafe.count() << " s" << std::endl;

    return 0;
}