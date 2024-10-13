// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, const FString& TypeOfMatch, const FString& PathToLobby)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	LobbyPath = FString::Printf(TEXT("%s?listen"), *PathToLobby);

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if (auto* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PlayerController->SetInputMode(InputModeData);

		PlayerController->SetShowMouseCursor(true);
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession);
		MultiplayerSessionSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this, &UMenu::OnFindSession);
		MultiplayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);
		MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenu::OnDestroySession);
		MultiplayerSessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenu::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (not Super::Initialize())
	{
		return false;
	}

	if (IsValid(HostButton))
	{
		HostButton->OnClicked.AddDynamic(this, &UMenu::OnHostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenu::OnJoinButtonClicked);
	}

	return true;
}

void UMenu::NativeDestruct()
{
	Super::NativeDestruct();

	MenuTeardown();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(LobbyPath);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5,
			FColor::Red, TEXT("Failed to created session!"));

		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (not IsValid(MultiplayerSessionSubsystem))
	{
		return;
	}

	for (const auto& Result : SearchResults)
	{
		FString SettingsMatchValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsMatchValue);

		if (SettingsMatchValue == MatchType)
		{
			MultiplayerSessionSubsystem->JoinSession(Result);
			return;
		}
	}

	if (not bWasSuccessful or SearchResults.IsEmpty())
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
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

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::OnHostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (IsValid(MultiplayerSessionSubsystem))
	{
		MultiplayerSessionSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::OnJoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (IsValid(MultiplayerSessionSubsystem))
	{
		MultiplayerSessionSubsystem->FindSessions(10000);
	}
}

void UMenu::MenuTeardown()
{
	RemoveFromParent();

	if (auto* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(false);
	}
}
