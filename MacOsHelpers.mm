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

bool open_minimeters() {
    @autoreleasepool {
        NSURL* minimeters_app_url = [[NSWorkspace sharedWorkspace]
            URLForApplicationWithBundleIdentifier:@"com.josephlyncheski.MiniMeters"];

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
