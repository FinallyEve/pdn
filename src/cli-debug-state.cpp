#ifdef NATIVE_BUILD

#include <chrono>

namespace cli {
    // Debug cycling state variables
    bool g_panelCycling = false;
    int g_panelCycleInterval = 3000;
    std::chrono::steady_clock::time_point g_panelCycleLastSwitch;

    bool g_stateCycling = false;
    int g_stateCycleDevice = -1;
    int g_stateCycleInterval = 2000;
    int g_stateCycleStep = 0;
    std::chrono::steady_clock::time_point g_stateCycleLastSwitch;
}

#endif // NATIVE_BUILD
