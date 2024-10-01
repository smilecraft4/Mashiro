#include "Mashiro.h"

#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {
    try {
        auto mashiro = std::make_unique<Mashiro>();
        mashiro->Run();
        
    } catch (std::runtime_error& err) {
        spdlog::critical("{}", err.what());
        return -1;
    } catch (...) {
        return -1;
    }
    return 0;
}