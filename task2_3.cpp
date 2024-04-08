#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
int counter = 0;
bool turn = 0;

void synchronized_increment(int id) {
    for (int i = 0; i < 1000; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [id] { return turn == id; });
        ++counter;
        turn = (turn + 1) % 2;
        cv.notify_all();
    }
}

int main() {
    std::thread t1(synchronized_increment, 0);
    std::thread t2(synchronized_increment, 1);

    t1.join();
    t2.join();

    std::cout << "Final counter value: " << counter << std::endl;
    return 0;
}