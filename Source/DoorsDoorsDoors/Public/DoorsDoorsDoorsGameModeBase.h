// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "DoorsDoorsDoorsGameWidget.h"
#include "GameFramework/GameModeBase.h"
#include "DoorsDoorsDoorsGameModeBase.generated.h"


class AController;
class ADoorsDoorsDoorsPlayerController;

/**
 to retrieve this object use below code
AGameModeBase* GameMode = Cast<ADoorsDoorsDoorsGameModeBase>(World->GetAuthGameMode());
 */
UCLASS()
class DOORSDOORSDOORS_API ADoorsDoorsDoorsGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	

public:
	ADoorsDoorsDoorsGameModeBase();

	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void RestartGame();

private:

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UUserWidget> ObjectiveWidgetClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UUserWidget> ObjectivesCompleteWidgetClass;

	//// --- VARS --- //
	//UPROPERTY(EditAnywhere, Category = "Widget")
	//	TSubclassOf<UDoorDoorsDoorsGameWidget> GameWidgetClass; // Exposed class to check the type of widget to display

		// Countdown before gameplay state begins. Exposed so we can easily change this in BP editor. 
	UPROPERTY(EditAnywhere, Category = "Game Details")
		float GameCountdownDuration = 4.0f;

	UFUNCTION(BlueprintCallable, Category = "Game Details")
		void SetNumExpectedPlayers(uint8 InNumExpectedPlayers) { NumExpectedPlayers = InNumExpectedPlayers; }

	UPROPERTY(EditAnywhere, Category = "Game Details")
		uint8 NumExpectedPlayers = 1u;


	FTimerHandle TimerHandle;


	// --- FUNCTIONS --- //
	void AttemptStartGame();
	void DisplayCountdown();
	void StartGame();
};
