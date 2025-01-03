#pragma once

#include "OnlineSessionSettings.h"
#include "SessionInfo.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SessionId;

	UPROPERTY(BlueprintReadWrite)
	FString UserName;

	UPROPERTY(BlueprintReadWrite)
	int32 NumCurrentPlayers;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxNumPlayers;

	UPROPERTY(BlueprintReadWrite)
	int32 PingInMs;

	UPROPERTY(BlueprintReadWrite)
	FString MatchType;

	// TODO Add more Session related information

public:
	static FSessionInfo MakeFrom(const FOnlineSessionSearchResult& SearchResult);
};


inline FSessionInfo FSessionInfo::MakeFrom(const FOnlineSessionSearchResult& SearchResult)
{
	const auto& NumPublicConnections = SearchResult.Session.SessionSettings.NumPublicConnections;
	const auto& NumOpenPublicConnections = SearchResult.Session.NumOpenPublicConnections;

	FString SettingsMatchValue;
	SearchResult.Session.SessionSettings.Get(FName("MatchType"), SettingsMatchValue);

	return {
		.SessionId = SearchResult.GetSessionIdStr(),
		.UserName = SearchResult.Session.OwningUserName,
		.NumCurrentPlayers = NumPublicConnections - NumOpenPublicConnections,
		.MaxNumPlayers = SearchResult.Session.SessionSettings.NumPublicConnections,
		.PingInMs = SearchResult.PingInMs,
		.MatchType = SettingsMatchValue
	};
}
