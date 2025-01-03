#pragma once

#include "SessionType.h"

#include "SessionCreateParams.generated.h"


USTRUCT(BlueprintType)
struct FSessionCreateParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName SessionName;

	UPROPERTY(BlueprintReadWrite)
	ESessionType SessionType;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxNumPlayers;

	UPROPERTY(BlueprintReadWrite)
	FString MatchType;
};
