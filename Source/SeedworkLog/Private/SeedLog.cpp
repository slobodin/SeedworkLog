// Copyright (c) Mistfly Games. All Rights Reserved.

#include "SeedLog.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Logging/LogCategory.h"
#include "Misc/BufferedOutputDevice.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/ScopeLock.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#endif

#if !NO_LOGGING

class FSeedLogOutputDevice final : public FBufferedOutputDevice
{
public:
    void RegisterCategory(FName category)
    {
        FScopeLock lock(&SynchronizationObject);
        Categories.Add(category);
    }

    void UnregisterCategory(FName category)
    {
        FScopeLock lock(&SynchronizationObject);
        Categories.Remove(category);
    }

    virtual bool CanBeUsedOnMultipleThreads() const override
    {
        return true;
    }

    virtual void Serialize(const TCHAR* text, ELogVerbosity::Type verbosity, const FName& category) override
    {
        const auto level = static_cast<ELogVerbosity::Type>(verbosity & ELogVerbosity::VerbosityMask);
        if (level != ELogVerbosity::Warning && level != ELogVerbosity::Error)
        {
            return;
        }

        FScopeLock lock(&SynchronizationObject);
        if (Categories.Contains(category))
        {
            BufferedLines.Emplace(text, category, level);
        }
    }

    bool Tick(float deltaSeconds)
    {
        check(IsInGameThread());

        TArray<FBufferedLine> lines;
        GetContents(lines);
        for (const auto& line : lines)
        {
            const auto text = FString::Printf(TEXT("[%s] %s"), *FName(line.Category).ToString(), line.Data.Get());
            if (line.Verbosity == ELogVerbosity::Warning)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, text);
                }
            }
#if WITH_EDITOR
            else if (line.Verbosity == ELogVerbosity::Error)
            {
                // The original message already went to the Output Log.
                FMessageLog("PIE").SuppressLoggingToOutputLog().Error(FText::FromString(text));
            }
#endif
        }

        return true;
    }

private:
    TSet<FName> Categories;
};

#endif

void FSeedworkLogModule::StartupModule()
{
#if !NO_LOGGING
    SeedLog = MakeShared<FSeedLogOutputDevice>();

    SeedLogTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(SeedLog.Get(), &FSeedLogOutputDevice::Tick));
    FOutputDeviceRedirector::Get()->AddOutputDevice(SeedLog.Get());
#endif
}

void FSeedworkLogModule::ShutdownModule()
{
#if !NO_LOGGING
    FOutputDeviceRedirector::Get()->RemoveOutputDevice(SeedLog.Get());
    FTSTicker::RemoveTicker(SeedLogTickerHandle);
    SeedLogTickerHandle.Reset();
    SeedLog.Reset();
#endif
}

void FSeedworkLogModule::RegisterCategory(const FLogCategoryBase& category)
{
#if !NO_LOGGING
    SeedLog->RegisterCategory(category.GetCategoryName());
#endif
}

void FSeedworkLogModule::UnregisterCategory(const FLogCategoryBase& category)
{
#if !NO_LOGGING
    SeedLog->UnregisterCategory(category.GetCategoryName());
#endif
}

IMPLEMENT_MODULE(FSeedworkLogModule, SeedworkLog)
