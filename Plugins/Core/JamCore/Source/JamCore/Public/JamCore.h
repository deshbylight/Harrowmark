#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

// Shared log category for every jam module: UE_LOG(LogJam, Display, TEXT("[Feature] ..."));
JAMCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogJam, Log, All);
