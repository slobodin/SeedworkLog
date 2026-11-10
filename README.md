# SeedworkLog

`SeedLog.h` declares the `TheGame` log category and hooks Unreal's log output.
`TheGame` is registered by default. Register other categories in your module's
`StartupModule()` and unregister them in `ShutdownModule()`:

```cpp
#include "SeedLog.h"

// StartupModule():
SeedLog::RegisterCategory(TEXT("LogMySystem"));

UE_LOGF(LogMySystem, Warning, "This appears on screen for 10 seconds.");

// ShutdownModule():
SeedLog::UnregisterCategory(TEXT("LogMySystem"));
```

For registered categories, warnings appear on screen in magenta, and errors are
added as errors to **Message Log > Play In Editor** in editor builds. Other
categories and verbosity levels receive normal Unreal logging only. Screen
messages follow Unreal's debug-message settings and are unavailable in Shipping
and Test builds.
