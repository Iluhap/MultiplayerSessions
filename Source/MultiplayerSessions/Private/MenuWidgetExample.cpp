// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidgetExample.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"


UMenuWidgetExample::UMenuWidgetExample()
{
	SessionName = FName { "Default Session Name" };
	SessionType = ESessionType::ST_Public;
	NumPublicConnections = 4;
	MaxSearchResults = 10000;
	MatchType = "FreeForAll";
	LobbyPath = "Lobby";

	JoinAnyOnFindSession = true;

	HostButton = nullptr;
	JoinButton = nullptr;
}

void UMenuWidgetExample::MenuSetup()
{
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
}

bool UMenuWidgetExample::Initialize()
{
	if (not Super::Initialize())
	{
		return false;
	}

	if (IsValid(HostButton))
	{
		HostButton->OnClicked.AddDynamic(this, &UMenuWidgetExample::OnHostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenuWidgetExample::OnJoinButtonClicked);
	}

	return true;
}

void UMenuWidgetExample::NativeDestruct()
{
	Super::NativeDestruct();

	MenuTeardown();
}

void UMenuWidgetExample::MenuTeardown()
{
	RemoveFromParent();

	if (auto* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(false);
	}
}

void UMenuWidgetExample::OnHostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (IsValid(SessionSubsystem))
	{
		SessionSubsystem->CreateSession({
			.SessionName = SessionName,
			.SessionType = SessionType,
			.MaxNumPlayers = NumPublicConnections,
			.MatchType = MatchType,
		});
	}
}

void UMenuWidgetExample::OnJoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (IsValid(SessionSubsystem))
	{
		SessionSubsystem->FindSessions(MaxSearchResults);
	}
}

void UMenuWidgetExample::OnCreateSession_Implementation(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(FString::Printf(TEXT("%s?listen"), *LobbyPath));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5,
			FColor::Red, TEXT("Failed to created session!"));

		HostButton->SetIsEnabled(true);
	}
}

void UMenuWidgetExample::OnFindSessions_Implementation(const TArray<FSessionInfo>& Sessions, bool bWasSuccessful)
{
	if (JoinAnyOnFindSession)
	{
		if (IsValid(SessionSubsystem))
		{
			for (const auto& Session : Sessions)
			{
				if (MatchType == Session.MatchType)
				{
					SessionSubsystem->JoinSession(Session.SessionId);
					return;
				}
			}
		}
		JoinButton->SetIsEnabled(true);
	}
}

void UMenuWidgetExample::OnJoinSession_Implementation(ESessionJoinResult Result)
{
	Super::OnJoinSession_Implementation(Result);

	JoinButton->SetIsEnabled(true);
}
