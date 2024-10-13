// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"


/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4,
	               const FString& TypeOfMatch = "FreeForAll",
	               const FString& PathToLobby = "Lobby");

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();

	void MenuTeardown();

private:
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionSubsystem;

private:
	UPROPERTY()
	int32 NumPublicConnections { 4 };

	UPROPERTY()
	FString MatchType { "FreeForAll" };

	UPROPERTY()
	FString LobbyPath {};

private:
	UPROPERTY(meta=(BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta=(BindWidget))
	UButton* JoinButton;
};
