// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DoorsDoorsDoorsGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class DOORSDOORSDOORS_API UDoorsDoorsDoorsGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
		void StartCountdown(float CountdownTime, class ADoorsDoorsDoorsPlayerController* DoorsDoorsDoorsPlayerController);

	UFUNCTION(BlueprintImplementableEvent)
		void LevelComplete();

	UFUNCTION(BlueprintImplementableEvent)
		void DisplayResults();

	UFUNCTION(BlueprintImplementableEvent)
		void RemoveResults();
};
