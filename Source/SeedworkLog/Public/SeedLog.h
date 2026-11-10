// Copyright (c) Mistfly Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Containers/Ticker.h"
#include "Templates/SharedPointer.h"

struct FLogCategoryBase;

class SEEDWORKLOG_API FSeedworkLogModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** Registered categories show warnings on screen and errors in the PIE Message Log. */
    void RegisterCategory(const FLogCategoryBase& category);
    void UnregisterCategory(const FLogCategoryBase& category);

private:
#if !NO_LOGGING
    TSharedPtr<class FSeedLogOutputDevice> SeedLog;
    FTSTicker::FDelegateHandle SeedLogTickerHandle;
#endif
};
