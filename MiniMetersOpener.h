#pragma once
#include "MacOsHelpers.h"
#include <JuceHeader.h>
#include <cstdlib>

class MiniMetersOpener {

public:
    enum Error {
        Success = 0,
        Failure
    };

    static bool is_minimeters_running() {
#if JUCE_MAC
        // Naive way of checking on macOS.
        // bool is_running = system("ps -Ac | grep 'MiniMeters' > /dev/null") == 0;
        return MacOsHelpers::is_minimeters_running();
#elif JUCE_WINDOWS
        // TODO:
#endif
        // Assume it is running otherwise.
        return true;
    }

    static bool launch_minimeters() {
#if JUCE_MAC
        auto minimeters_url = URL("file:///Applications/MiniMeters.app");
        minimeters_url.launchInDefaultBrowser();
#elif JUCE_WINDOWS
        // TODO
#endif
        return false;
    }
};