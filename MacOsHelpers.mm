#include "MacOsHelpers.h"
#include <AppKit/NSWorkspace.h>
#include <Foundation/NSBundle.h>

namespace MacOsHelpers {
static NSURL* get_bundle_id_if_it_exists(NSString* bundle_id) {
    NSURL* minimeters_app_url = [[NSWorkspace sharedWorkspace]
        URLForApplicationWithBundleIdentifier:bundle_id];
    // Check if the bundle exists.
    NSError* err;
    bool is_available = [minimeters_app_url checkResourceIsReachableAndReturnError:&err];

    if (is_available)
        return minimeters_app_url;

    return nullptr;
}

bool is_minimeters_present_or_running() {
    @autoreleasepool {
        NSURL* minimeters_app_url = get_bundle_id_if_it_exists(@"com.josephlyncheski.minimeters");
        NSURL* minimeters_demo_app_url = get_bundle_id_if_it_exists(@"com.josephlyncheski.minimetersdemo");

        NSString* minimeters_bundle_id = @"com.josephlyncheski.minimeters";

        if (minimeters_demo_app_url && !minimeters_app_url) {
            // User is running the demo swap the id we are looking for.
            minimeters_bundle_id = @"com.josephlyncheski.minimetersdemo";
        } else if (!minimeters_app_url && !minimeters_demo_app_url) {
            // It looks as if neither are installed here.
            return true; // Return that we are running as to not show the button.
        }

        NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
        NSArray* apps = [workspace runningApplications];

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

bool open_minimeters() {
    @autoreleasepool {
        NSURL* minimeters_app_url = get_bundle_id_if_it_exists(@"com.josephlyncheski.minimeters");
        NSURL* minimeters_demo_app_url = get_bundle_id_if_it_exists(@"com.josephlyncheski.minimetersdemo");

        // Check if MiniMeters.app exists somewhere on the computer.
        // If not then try the demo. If nether just return without doing anything.
        if (!minimeters_app_url) {
            if (minimeters_demo_app_url) {
                minimeters_app_url = minimeters_demo_app_url;
            } else {
                return true; // Just say we opened it even if we cannot.
            }
        }

        // The following is stolen from JUCE's code.
        NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
        if (@available(macOS 10.15, *)) {
            auto config = [NSWorkspaceOpenConfiguration configuration];
            [config setCreatesNewApplicationInstance:YES];
            config.arguments = @[ @"-d", @"MiniMeters Plugin" ];

            [workspace openApplicationAtURL:minimeters_app_url
                              configuration:config
                          completionHandler:nil];

            return true;
        } else {
            NSMutableDictionary* dict = [[NSMutableDictionary new] autorelease];

            [dict setObject:@[ @"-d", @"MiniMeters Plugin" ]
                     forKey:@"NSWorkspaceLaunchConfigurationArguments"];
            return [workspace launchApplicationAtURL:minimeters_app_url
                                             options:NSWorkspaceLaunchDefault | NSWorkspaceLaunchNewInstance
                                       configuration:dict
                                               error:nil];
            return true;
        }
    }
}

}
