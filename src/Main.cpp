#include "Mashiro.h"

int main(int argc, char *argv[]) {
    try {
        auto mashiro = std::make_unique<Mashiro>();
        mashiro->Run();
    } catch (...) {
        return -1;
    }
    return 0;
}