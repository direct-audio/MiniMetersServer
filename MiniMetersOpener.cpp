#include "MiniMetersOpener.h"

#if defined(__APPLE__)
#include "MacOsHelpers.h"
#elif defined(_WIN32)
#include <windows.h> // Must be first it the list.

#include <ShlObj_core.h>
#include <processthreadsapi.h>
#include <tchar.h>
#include <tlhelp32.h>

#endif
#include <fstream>
#include <string>

bool MiniMetersOpener::is_minimeters_running() {
#if defined(__APPLE__)
    // Naive way of checking on macOS.
    // bool is_running = system("ps -Ac | grep 'MiniMeters' > /dev/null") == 0;
    return MacOsHelpers::is_minimeters_present_or_running();
#elif defined(_WIN32)
    // Pulled from here: https://stackoverflow.com/questions/3477097/get-full-running-process-list-visual-c
    std::string compare;
    bool minimeters_running = false;

    HANDLE process_snapshot_handle;
    PROCESSENTRY32 pe32;
    process_snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (process_snapshot_handle == INVALID_HANDLE_VALUE) {
        minimeters_running = false;
    } else {
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(process_snapshot_handle, &pe32)) { // Gets first running process
            if (std::string(pe32.szExeFile) == std::string("MiniMeters.exe")) {
                minimeters_running = true;
            } else {
                // loop through all running processes looking for process
                while (Process32Next(process_snapshot_handle, &pe32)) {
                    if (std::string(pe32.szExeFile) == "MiniMeters.exe") {
                        // if found process is running, set to true and break from loop
                        minimeters_running = true;
                        break;
                    }
                }
            }
            // clean the snapshot object
            CloseHandle(process_snapshot_handle);
        }
    }

    return minimeters_running;
#endif
    // Assume it is running otherwise.
    return true;
}

static bool does_file_exist(const std::string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

bool MiniMetersOpener::launch_minimeters() {
#if defined(__APPLE__)
    //        juce::Process::openDocument("com.josephlyncheski.minimeters", "-d \"MiniMeters Plugin\"");
    MacOsHelpers::open_minimeters();
#elif defined(_WIN32)

    // Pulled from here: https://stackoverflow.com/questions/15435994/how-do-i-open-an-exe-from-another-c-exe

    // Get the Program Files folder.
    // FIXME: This isn't fool proof. Someone could install MiniMeters anywhere. Maybe we should save a RegKey
    //        on install with the location. Might be overkill for a simple QOL thing though.

    char program_files_folder[MAX_PATH];
    bool success = SHGetSpecialFolderPath(
        0,
        program_files_folder,
        CSIDL_PROGRAM_FILES,
        FALSE);

    if (!success) {
        // FIXME: We should probably notify the user that we could not get the program files folder.
        return false;
    }

    std::string minimeters_location = std::string(program_files_folder) + "\\MiniMeters\\MiniMeters.exe";
    std::string minimeters_demo_location = std::string(program_files_folder) + "\\MiniMeters\\MiniMeters-demo.exe";

    std::ifstream mm_exists(minimeters_location);
    std::ifstream mm_demo_exists(minimeters_demo_location);

    if (!mm_exists.good()) {
        if (mm_demo_exists.good()) {
            minimeters_location = minimeters_demo_location;
        } else {
            return true; // Just say we opened it even if we cannot.
        }
    }

    STARTUPINFOA
    startup_info;
    PROCESS_INFORMATION process_info;

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    ZeroMemory(&process_info, sizeof(process_info));

    char args[] = "-d \"MiniMeters Plugin\"";

    CreateProcessA(
        minimeters_location.c_str(),
        args,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &startup_info,
        &process_info);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
#endif
    return is_minimeters_running();
}
