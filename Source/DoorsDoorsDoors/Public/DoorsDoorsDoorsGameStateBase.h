// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DoorsDoorsDoorsGameStateBase.generated.h"

// Enum to track the current state of the game 
UENUM(BlueprintType)
enum class EGameState : uint8
{
	None		UMETA(DisplayName = "None"),
	Waiting		UMETA(DisplayName = "Waiting"),
	Playing		UMETA(DisplayName = "Playing"),
	Paused		UMETA(DisplayName = "Paused"),
	GameOver	UMETA(DisplayName = "GameOver"),
};

class ADoorsDoorsDoorsPlayerCharacter;
class ADoorsDoorsDoorsPlayerState;

USTRUCT()
struct FGameResult
{
	GENERATED_BODY()

		UPROPERTY()
		FString Name;

	UPROPERTY()
		float Time;
};

UCLASS()
class DOORSDOORSDOORS_API ADoorsDoorsDoorsGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
		void SetGameState(EGameState InGameState) { GameState = InGameState; }

	UFUNCTION(BlueprintPure)
		EGameState GetGameState() const { return GameState; }

	UFUNCTION(BlueprintPure)
		bool IsPlaying() const { return GameState == EGameState::Playing; }

	//this will only be called on authority
	void OnPlayerReachedEnd(ADoorsDoorsDoorsPlayerCharacter* DoorsDoorsDoorsCharacter);

	UFUNCTION()
		void ClearResults();

protected:
	void UpdateResults(ADoorsDoorsDoorsPlayerState* PlayerState, ADoorsDoorsDoorsPlayerCharacter* DoorsDoorsDoorsCharacter);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GameState, Category = "States")
		EGameState GameState = EGameState::None;

	UFUNCTION()
		void OnRep_GameState(const EGameState& OldGameState);

	UPROPERTY(VisibleAnywhere, replicated, Category = "States")
		TArray<FGameResult> Results;
};
