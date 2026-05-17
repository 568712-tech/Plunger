#include "core/Engine.h"

#include <iostream>

int main()
{
    try {
        plunger::Engine engine;
        engine.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 3;
    } catch (...) {
        std::cerr << "Unknown startup error\n";
        return 3;
    }
}
