// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
	:
	OnCreateSessionCompleteDelegate(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	OnFindSessionsCompleteDelegate(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	OnJoinSessionCompleteDelegate(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	OnStartSessionCompleteDelegate(
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	OnDestroySessionCompleteDelegate(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();

		OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
			OnFindSessionsCompleteDelegate
		);
		OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
			OnJoinSessionCompleteDelegate
		);
		OnStartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
			OnStartSessionCompleteDelegate
		);
		OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			OnDestroySessionCompleteDelegate
		);
	}

	LastSessionParams = {};
}

void UMultiplayerSessionsSubsystem::CreateSession(const FSessionCreateParams& Params)
{
	if (not SessionInterface)
	{
		return;
	}

	if (const auto* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession); ExistingSession)
	{
		LastSessionParams = Params;
		SessionInterface->DestroySession(NAME_GameSession);
	}

	OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		OnCreateSessionCompleteDelegate
	);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings {});

	if (LastSessionSettings)
	{
		const bool bIsLAN = Params.SessionType == ESessionType::ST_LAN
			or IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
		const bool bIsSolo = Params.SessionType == ESessionType::ST_Solo;
		const bool bIsPublic = Params.SessionType == ESessionType::ST_Public;

		LastSessionSettings->bIsLANMatch = bIsLAN;
		LastSessionSettings->NumPublicConnections = Params.MaxNumPlayers;
		LastSessionSettings->bAllowJoinInProgress = not bIsSolo;
		LastSessionSettings->bAllowJoinViaPresence = not bIsSolo;
		LastSessionSettings->bAllowInvites = not bIsSolo;
		LastSessionSettings->bShouldAdvertise = bIsPublic;
		LastSessionSettings->bUsesPresence = true;
		LastSessionSettings->bUseLobbiesIfAvailable = true;
		LastSessionSettings->Set(FName("MatchType"), Params.MatchType,
		                         EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		if (const auto LocalPlayerNetID = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();
			not SessionInterface->CreateSession(*LocalPlayerNetID, NAME_GameSession, *LastSessionSettings))
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

			MultiplayerOnCreateSessionComplete.Broadcast(false);
		}
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(const int32& MaxSearchResults)
{
	if (not SessionInterface.IsValid())
	{
		return;
	}

	OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		OnFindSessionsCompleteDelegate);

	LastSessionSearch = MakeShared<FOnlineSessionSearch>();

	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if (const auto LocalPlayerNetID = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();
		not SessionInterface->FindSessions(*LocalPlayerNetID, LastSessionSearch.ToSharedRef()))
	{
		MultiplayerOnFindSessionComplete.Broadcast({}, false);

		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FString& SessionId)
{
	if (not LastSessionSearch.IsValid())
		return;

	if (const auto* SearchResultIter = LastSessionSearchResultsMap.Find(SessionId);
		SearchResultIter)
	{
		JoinSession(**SearchResultIter);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	if (not SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(ESessionJoinResult::SJR_UnknownError);
		return;
	}

	OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		OnJoinSessionCompleteDelegate
	);

	const FString SessionID = SessionSearchResult.GetSessionIdStr();
	const FString User = SessionSearchResult.Session.OwningUserName;

	GEngine->AddOnScreenDebugMessage(
		-1, 5, FColor::Yellow,
		FString::Printf(TEXT("Joining user: %s session: %s"), *User, *SessionID));

	if (const auto LocalPlayerNetID = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();
		not SessionInterface->JoinSession(*LocalPlayerNetID, NAME_GameSession, SessionSearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(ESessionJoinResult::SJR_UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (not SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		OnDestroySessionCompleteDelegate
	);

	if (not SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

IOnlineSessionPtr UMultiplayerSessionsSubsystem::GetSessionInterface() const
{
	return SessionInterface;
}

TArray<FSessionInfo> UMultiplayerSessionsSubsystem::GetLastSessions() const
{
	TArray<FSessionInfo> Sessions;

	for (const auto& SearchResult : LastSessionSearch->SearchResults)
	{
		Sessions.Add(FSessionInfo::MakeFrom(SearchResult));
	}

	return Sessions;
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch.IsValid())
	{
		if (LastSessionSearch->SearchResults.IsEmpty())
		{
			MultiplayerOnFindSessionComplete.Broadcast({}, false);
			return;
		}

		LastSessionSearchResultsMap.Empty();

		for (auto SearchResultIter = LastSessionSearch->SearchResults.CreateConstIterator();
		     SearchResultIter; ++SearchResultIter)
		{
			LastSessionSearchResultsMap.Add(SearchResultIter->GetSessionIdStr(), SearchResultIter);
		}

		MultiplayerOnFindSessionComplete.Broadcast(GetLastSessions(), bWasSuccessful);
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(MakeSessionJoinResultFrom(Result));
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
	}

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastSessionParams);
	}

	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}
