#pragma once

#include "SessionType.generated.h"

UENUM(BlueprintType)
enum class ESessionType : uint8
{
	ST_Public UMETA(DisplayName = "Public"),
	ST_FriendsOnly UMETA(DisplayName = "Friends Only"),
	ST_LAN UMETA(DisplayName = "LAN"),
	ST_Solo UMETA(DisplayName = "Solo"),

	ST_MAX UMETA(DisplayName = "Default MAX")
};
