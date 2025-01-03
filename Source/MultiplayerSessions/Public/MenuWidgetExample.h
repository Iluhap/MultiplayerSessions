// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "MenuWidgetExample.generated.h"


UCLASS()
class MULTIPLAYERSESSIONS_API UMenuWidgetExample : public UMenuWidgetBase
{
	GENERATED_BODY()

public:
	UMenuWidgetExample();

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

protected:
	virtual void OnCreateSession_Implementation(bool bWasSuccessful) override;
	virtual void OnFindSessions_Implementation(const TArray<FSessionInfo>& Sessions, bool bWasSuccessful) override;
	virtual void OnJoinSession_Implementation(ESessionJoinResult Result) override;

private:
	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();

	void MenuTeardown();

protected:
	UPROPERTY(meta=(BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta=(BindWidget))
	UButton* JoinButton;

private:
	UPROPERTY(EditAnywhere, Category="Multiplayer Config", meta = (AllowPrivateAccess = "true"))
	FName SessionName;

	UPROPERTY(EditAnywhere, Category="Multiplayer Config", meta = (AllowPrivateAccess = "true"))
	ESessionType SessionType;

	UPROPERTY(EditAnywhere, Category="Multiplayer Config", meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections;

	UPROPERTY(EditAnywhere, Category="Multiplayer Config", meta = (AllowPrivateAccess = "true"))
	int32 MaxSearchResults;

	UPROPERTY(EditAnywhere, Category="Multiplayer Config", meta = (AllowPrivateAccess = "true"))
	FString MatchType;

	UPROPERTY(EditAnywhere, Category="Multiplayer Config", meta = (AllowPrivateAccess = "true"))
	FString LobbyPath;

	UPROPERTY(EditAnywhere, Category="Multiplayer Config", meta = (AllowPrivateAccess = "true"))
	bool JoinAnyOnFindSession;
};
