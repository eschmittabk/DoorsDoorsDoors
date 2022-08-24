// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "DoorsDoorsDoorsGameModeBase.generated.h"

/**
 to retrieve this object use below code
AGameModeBase* GameMode = Cast<ADoorsDoorsDoorsGameModeBase>(World->GetAuthGameMode());
 */
UCLASS()
class DOORSDOORSDOORS_API ADoorsDoorsDoorsGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	

public:
	ADoorsDoorsDoorsGameModeBase() {}

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UUserWidget> ObjectiveWidgetClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UUserWidget> ObjectivesCompleteWidgetClass;
};
