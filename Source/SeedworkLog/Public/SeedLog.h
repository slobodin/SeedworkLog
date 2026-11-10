// Copyright (c) Mistfly Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

SEEDWORKGAME_API DECLARE_LOG_CATEGORY_EXTERN(TheGame, Log, All);

namespace SeedLog
{
    // Registered categories show warnings on screen and errors in the PIE Message Log.
    // TheGame is registered by default. Repeated registration has no effect.
    SEEDWORKGAME_API void RegisterCategory(FName category);
    SEEDWORKGAME_API void UnregisterCategory(FName category);
}

#if NO_LOGGING

#define GAME_LOG(Cat, Format, ...) ((void) 0)
#define GAME_WARNING(Format, ...) ((void) 0)
#define GAME_ERROR(Format, ...) ((void) 0)

#else

void SEEDWORKGAME_API DoTraceGameLog(const FLogCategoryBase* category, const FString& text);
void SEEDWORKGAME_API DoTraceWarning(const FString& text);
void SEEDWORKGAME_API DoTraceError(const FString& text);

#define GAME_LOG(Cat, Format, ...) DoTraceGameLog(&Cat, FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define GAME_WARNING(Format, ...) DoTraceWarning(FString::Printf(TEXT(Format), ##__VA_ARGS__))
#define GAME_ERROR(Format, ...) DoTraceError(FString::Printf(TEXT(Format), ##__VA_ARGS__))

#endif
// Copyright (c) Mistfly Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSeedworkGameModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
