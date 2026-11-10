// Copyright (c) Mistfly Games. All Rights Reserved.

#include "SeedLog.h"
#include "SeedLogInternal.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Misc/BufferedOutputDevice.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/ScopeLock.h"

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#endif

DEFINE_LOG_CATEGORY(TheGame)

#if !NO_LOGGING

namespace
{
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
                // Copy the text before the logging callback returns.
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

    FSeedLogOutputDevice& GetSeedLogOutputDevice()
    {
        static FSeedLogOutputDevice outputDevice;
        return outputDevice;
    }

    FTSTicker::FDelegateHandle SeedLogTickerHandle;
}

void DoTraceGameLog(const FLogCategoryBase* category, const FString& text)
{
    const FString categoryName = category->GetCategoryName().ToString();

    const FString finalString = FString::Printf(TEXT("    [%s]: %s"), *categoryName, *text);

    UE_LOG(TheGame, Display, TEXT("%s"), *finalString);
}

void DoTraceWarning(const FString& text)
{
    const FString warnText = FString::Printf(TEXT("[warning]: %s"), *text);

    UE_LOG(TheGame, Warning, TEXT("%s"), *warnText);
}

void DoTraceError(const FString& text)
{
    const FString errText = FString::Printf(TEXT("[error]: %s"), *text);

    UE_LOG(TheGame, Error, TEXT("%s"), *errText);
}

#endif

void SeedLog::RegisterCategory(FName category)
{
#if !NO_LOGGING
    GetSeedLogOutputDevice().RegisterCategory(category);
#endif
}

void SeedLog::UnregisterCategory(FName category)
{
#if !NO_LOGGING
    GetSeedLogOutputDevice().UnregisterCategory(category);
#endif
}

void SeedLog::Private::Initialize()
{
#if !NO_LOGGING
    auto& outputDevice = GetSeedLogOutputDevice();
    outputDevice.RegisterCategory(FName(TheGame.GetCategoryName()));
    SeedLogTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(&outputDevice, &FSeedLogOutputDevice::Tick));
    FOutputDeviceRedirector::Get()->AddOutputDevice(&outputDevice);
#endif
}

void SeedLog::Private::Shutdown()
{
#if !NO_LOGGING
    FOutputDeviceRedirector::Get()->RemoveOutputDevice(&GetSeedLogOutputDevice());
    FTSTicker::RemoveTicker(SeedLogTickerHandle);
    SeedLogTickerHandle.Reset();
#endif
}
// Copyright (c) Mistfly Games. All Rights Reserved.

#include "SeedworkGame.h"
#include "SeedLogInternal.h"

void FSeedworkGameModule::StartupModule()
{
    SeedLog::Private::Initialize();
}

void FSeedworkGameModule::ShutdownModule()
{
    SeedLog::Private::Shutdown();
}

IMPLEMENT_MODULE(FSeedworkGameModule, SeedworkGame)
