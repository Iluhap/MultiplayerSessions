// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
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

		// if (GEngine)
		// {
		// 	GEngine->AddOnScreenDebugMessage(
		// 		-1, 15, FColor::Cyan,
		// 		FString::Printf(TEXT("Found subsystem: %s"), *Subsystem->GetSubsystemName().ToString()));
		// }

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
}

void UMultiplayerSessionsSubsystem::CreateSession(const int32& NumPublicConnections, const FString& MatchType)
{
	if (not SessionInterface)
	{
		return;
	}

	if (const auto* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession); ExistingSession)
	{
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		SessionInterface->DestroySession(NAME_GameSession);
	}

	OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		OnCreateSessionCompleteDelegate
	);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings {});

	if (LastSessionSettings)
	{
		LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
		LastSessionSettings->NumPublicConnections = NumPublicConnections;
		LastSessionSettings->bAllowJoinInProgress = true;
		LastSessionSettings->bAllowJoinViaPresence = true;
		LastSessionSettings->bAllowInvites = true;
		LastSessionSettings->bShouldAdvertise = true;
		LastSessionSettings->bUsesPresence = true;
		LastSessionSettings->bUseLobbiesIfAvailable = true;
		LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

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

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	if (not SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
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
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
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

		MultiplayerOnFindSessionComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
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
		CreateSession(LastNumPublicConnections, LastMatchType);
	}

	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}
