// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidgetBase.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

bool UMenuWidgetBase::Initialize()
{
	if (not Super::Initialize())
	{
		return false;
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (SessionSubsystem)
	{
		SessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenuWidgetBase::OnCreateSession);
		SessionSubsystem->MultiplayerOnFindSessionComplete.AddDynamic(this, &UMenuWidgetBase::OnFindSessions);
		SessionSubsystem->MultiplayerOnJoinSessionComplete.AddDynamic(this, &UMenuWidgetBase::OnJoinSession);
		SessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenuWidgetBase::OnDestroySession);
		SessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenuWidgetBase::OnStartSession);
	}

	return true;
}

void UMenuWidgetBase::OnCreateSession_Implementation(bool bWasSuccessful)
{
}

void UMenuWidgetBase::OnFindSessions_Implementation(const TArray<FSessionInfo>& Sessions,
                                                    bool bWasSuccessful)
{
}

void UMenuWidgetBase::OnJoinSession_Implementation(ESessionJoinResult Result)
{
	if (not IsValid(SessionSubsystem))
		return;

	if (const auto SessionInterface = SessionSubsystem->GetSessionInterface();
		SessionInterface.IsValid())
	{
		if (FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
		{
			if (auto* PlayerController = GetGameInstance()->GetFirstLocalPlayerController(); PlayerController)
			{
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}
}

void UMenuWidgetBase::OnDestroySession_Implementation(bool bWasSuccessful)
{
}

void UMenuWidgetBase::OnStartSession_Implementation(bool bWasSuccessful)
{
}
