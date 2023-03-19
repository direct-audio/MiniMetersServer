#pragma once
#include <JuceHeader.h>
#include <cstdlib>

class MiniMetersOpener {

public:
    enum Error {
        Success = 0,
        Failure
    };

    static bool is_minimeters_running();
    static bool launch_minimeters();
};