#include "MacOsHelpers.h"
#include <AppKit/NSWorkspace.h>
#include <Foundation/NSBundle.h>

namespace MacOsHelpers {
bool is_minimeters_running() {
    @autoreleasepool {
        NSArray* apps = [[NSWorkspace sharedWorkspace] runningApplications];
        NSString* minimeters_bundle_id = @"com.josephlyncheski.minimeters";

        for (NSRunningApplication* app in apps) {
            NSBundle* bundle = [NSBundle bundleWithURL:[app bundleURL]];
            NSDictionary* info = [bundle infoDictionary];
            NSString* identifier = info[@"CFBundleIdentifier"];
            // Using isEqual so if for some reason nil is passed we don't hit an exception.
            if ([identifier isEqual:minimeters_bundle_id]) {
                return true;
            }
        }
    };
    return false;
}

}
