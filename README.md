# SeedworkLog

Unreal Engine runtime plugin that adds debug displays for registered log categories:

- **Warnings:** magenta on-screen messages lasting 10 seconds.
- **Errors:** entries in **Message Log > Play In Editor** in editor builds.

Enable the plugin and add `"SeedworkLog"` to your module's `.Build.cs` dependencies.
No categories are registered by default. Register your categories during startup
and unregister them during shutdown:

```cpp
#include "SeedLog.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMySystem, Log, All);

// StartupModule():
auto& logModule = FModuleManager::LoadModuleChecked<FSeedworkLogModule>("SeedworkLog");
logModule.RegisterCategory(LogMySystem);

// Log normally:
UE_LOG(LogMySystem, Warning, TEXT("This also appears on screen."));

// ShutdownModule():
if (auto* logModule = FModuleManager::GetModulePtr<FSeedworkLogModule>("SeedworkLog"))
{
    logModule->UnregisterCategory(LogMySystem);
}
```

Use `RegisterCategoryName(FName)` and `UnregisterCategoryName(FName)` to register
by name. Normal Unreal logging is preserved. Forwarding is disabled when
`NO_LOGGING` is set; on-screen warnings follow Unreal's debug-message settings.
