#include <QtGlobal>
#import <Cocoa/Cocoa.h>

#include "macos.h"

// see https://github.com/haiwen/seafile-client/blob/master/src/utils/utils-mac.mm
// 

unsigned osver_major = 0;
unsigned osver_minor = 0;
unsigned osver_patch = 0;
inline bool isInitializedSystemVersion() { return osver_major != 0; }
inline void initializeSystemVersion() {
    if (isInitializedSystemVersion())
        return;
#if (__MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10)
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    osver_major = version.majorVersion;
    osver_minor = version.minorVersion;
    osver_patch = version.patchVersion;
#else
    NSString *versionString = [[NSProcessInfo processInfo] operatingSystemVersionString];
    NSArray *array = [versionString componentsSeparatedByString:@" "];
    if (array.count < 2) {
        osver_major = 10;
        osver_minor = 7;
        osver_patch = 0;
        return;
    }
    NSArray *versionArray = [[array objectAtIndex:1] componentsSeparatedByString:@"."];
    if (versionArray.count < 3) {
        osver_major = 10;
        osver_minor = 7;
        osver_patch = 0;
        return;
    }
    osver_major = [[versionArray objectAtIndex:0] intValue];
    osver_minor = [[versionArray objectAtIndex:1] intValue];
    osver_patch = [[versionArray objectAtIndex:2] intValue];
#endif
}

void getSystemVersion(unsigned *major, unsigned *minor, unsigned *patch) {
    initializeSystemVersion();
    *major = osver_major;
    *minor = osver_minor;
    *patch = osver_patch;
}

void setDockIconStyle(bool hidden) {
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    OSStatus err;
    if (hidden) {
        // kProcessTransformToBackgroundApplication is not support on OSX 10.7 and before
        // kProcessTransformToUIElementApplication is used for better fit when possible
        unsigned major;
        unsigned minor;
        unsigned patch;
        getSystemVersion(&major, &minor, &patch);
        if (major == 10 && minor == 7) {
            err = TransformProcessType(&psn, kProcessTransformToBackgroundApplication);
        } else {
            err = TransformProcessType(&psn, kProcessTransformToUIElementApplication);
        }
    } else {
        // kProcessTransformToForegroundApplication is supported on OSX 10.6 or later
        err = TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    }
    if (err != noErr) {
        qWarning("setDockIconStyle %s failure, status code: %d\n", (hidden ? "hidden" : "show"), err);
    }
}
