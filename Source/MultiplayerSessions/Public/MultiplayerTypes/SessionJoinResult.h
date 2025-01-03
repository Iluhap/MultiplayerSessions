#pragma once

#include "Interfaces/OnlineSessionInterface.h"
#include "SessionJoinResult.generated.h"


UENUM(BlueprintType)
enum class ESessionJoinResult : uint8
{
	SJR_Success UMETA(DisplayName = "Success"),
	SJR_SessionIsFull UMETA(DisplayName = "SessionIsFull"),
	SJR_SessionDoesNotExist UMETA(DisplayName = "SessionDoesNotExist"),
	SJR_CouldNotRetrieveAddress UMETA(DisplayName = "CouldNotRetrieveAddress"),
	SJR_AlreadyInSession UMETA(DisplayName = "AlreadyInSession"),
	SJR_UnknownError UMETA(DisplayName = "UnknownError"),

	SJR_MAX UMETA(DisplayName = "Default MAX")
};

inline ESessionJoinResult MakeSessionJoinResultFrom(
	const EOnJoinSessionCompleteResult::Type& Result)
{
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		{
			return ESessionJoinResult::SJR_Success;
		}
	case EOnJoinSessionCompleteResult::SessionIsFull:
		{
			return ESessionJoinResult::SJR_SessionIsFull;
		}
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		{
			return ESessionJoinResult::SJR_SessionDoesNotExist;
		}
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		{
			return ESessionJoinResult::SJR_CouldNotRetrieveAddress;
		}
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		{
			return ESessionJoinResult::SJR_AlreadyInSession;
		}
	default:
		{
			return ESessionJoinResult::SJR_UnknownError;
		}
	}
}
