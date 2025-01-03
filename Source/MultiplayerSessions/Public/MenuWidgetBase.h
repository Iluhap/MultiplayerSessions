// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerTypes/OnlineSessionTypes.h"

#include "MenuWidgetBase.generated.h"



UCLASS()
class MULTIPLAYERSESSIONS_API UMenuWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnCreateSession(bool bWasSuccessful);

	UFUNCTION(BlueprintNativeEvent)
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION(BlueprintNativeEvent)
	void OnStartSession(bool bWasSuccessful);

	UFUNCTION(BlueprintNativeEvent)
	void OnFindSessions(const TArray<FSessionInfo>& Sessions, bool bWasSuccessful);

	UFUNCTION(BlueprintNativeEvent)
	void OnJoinSession(ESessionJoinResult Result);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMultiplayerSessionsSubsystem* SessionSubsystem;
};
