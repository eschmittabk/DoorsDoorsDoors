// Copyright Epic Games, Inc. All Rights Reserved.


#include "DoorsDoorsDoorsGameModeBase.h"
//#include "DoorsDoorsDoorsGameInstance.h"
#include "DoorsDoorsDoorsGameStateBase.h"
#include "DoorsDoorsDoorsPlayerController.h"
#include "DoorsDoorsDoorsPlayerState.h"
//#include "DoorsDoorsDoorsAIController.h"
#include "DoorsDoorsDoorsGameWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


ADoorsDoorsDoorsGameModeBase::ADoorsDoorsDoorsGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADoorsDoorsDoorsGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	if (ADoorsDoorsDoorsGameStateBase* DoorsDoorsDoorsGameState = GetGameState<ADoorsDoorsDoorsGameStateBase>())
	{
		DoorsDoorsDoorsGameState->SetGameState(EGameState::Waiting);
	}
}

void ADoorsDoorsDoorsGameModeBase::AttemptStartGame()
{
	if (ADoorsDoorsDoorsGameStateBase* DoorsDoorsDoorsGameState = GetGameState<ADoorsDoorsDoorsGameStateBase>())
	{
		DoorsDoorsDoorsGameState->SetGameState(EGameState::Waiting);
	}
	if (GetNumPlayers() == NumExpectedPlayers)
	{
		//this needs to be replicated, call a function on game instance and replicate
		DisplayCountdown();
		if (GameCountdownDuration > SMALL_NUMBER)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ADoorsDoorsDoorsGameModeBase::StartGame, GameCountdownDuration, false);
		}
		else
		{
			//this is always called from the authority, aka here
			StartGame();
		}

	}
}

void ADoorsDoorsDoorsGameModeBase::DisplayCountdown()
{
	//set the hud for the game instances here, and the controller can then do what it needs
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			/*if (ADoorsDoorsDoorsPlayerController* DoorsDoorsDoorsPlayerController = Cast<ADoorsDoorsDoorsPlayerController>(PlayerController))
			{
				DoorsDoorsDoorsPlayerController->ClientDisplayCountdown(GameCountdownDuration, GameWidgetClass);
			}*/
		}
	}
}
//broadcast this?
void ADoorsDoorsDoorsGameModeBase::StartGame()
{
	if (ADoorsDoorsDoorsGameStateBase* DoorsDoorsDoorsGameState = GetGameState<ADoorsDoorsDoorsGameStateBase>())
	{
		DoorsDoorsDoorsGameState->SetGameState(EGameState::Playing);
		DoorsDoorsDoorsGameState->ClearResults();
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			//cast and start?
			//this does not work on all controllers...
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);

			ADoorsDoorsDoorsPlayerState* PlayerState = PlayerController->GetPlayerState<ADoorsDoorsDoorsPlayerState>();
			if (PlayerState)
			{
				PlayerState->SetCurrentState(EPlayerGameState::Playing);
				PlayerState->SetIsWinner(false);
			}
		}
	}

	//for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	//{
	//	ADoorsDoorsDoorsAIController* DoorsDoorsDoorsAIController = Cast<ADoorsDoorsDoorsAIController>(Iterator->Get());
	//	if (DoorsDoorsDoorsAIController)
	//	{
	//		ADoorsDoorsDoorsPlayerState* PlayerState = DoorsDoorsDoorsAIController->GetPlayerState<ADoorsDoorsDoorsPlayerState>();
	//		if (PlayerState)
	//		{
	//			PlayerState->SetCurrentState(EPlayerGameState::Playing);
	//			PlayerState->SetIsWinner(false);
	//		}
	//	}
	//}

}

void ADoorsDoorsDoorsGameModeBase::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (APlayerController* PlayerController = Cast<APlayerController>(NewPlayer))
	{
		if (PlayerController->GetCharacter() && PlayerController->GetCharacter()->GetCharacterMovement())
		{
			PlayerController->GetCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			ADoorsDoorsDoorsPlayerState* PlayerState = PlayerController->GetPlayerState<ADoorsDoorsDoorsPlayerState>();
			if (PlayerState)
			{
				PlayerState->SetCurrentState(EPlayerGameState::Waiting);
			}
		}
	}

	AttemptStartGame();
}

void ADoorsDoorsDoorsGameModeBase::RestartGame()
{
	//destroy the actor
	//for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	//{
	//	ADoorsDoorsDoorsAIController* DoorsDoorsDoorsAIController = Cast<ADoorsDoorsDoorsAIController>(Iterator->Get());
	//	if (DoorsDoorsDoorsAIController && DoorsDoorsDoorsAIController->GetPawn())
	//	{
	//		DoorsDoorsDoorsAIController->Destroy(true);
	//	}
	//}

	ResetLevel();
	//RestartGame();
	//GetWorld()->ServerTravel(TEXT("?Restart"), false);
	//ProcessServerTravel("?Restart");
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (ADoorsDoorsDoorsPlayerController* DoorsDoorsDoorsPlayerController = Cast<ADoorsDoorsDoorsPlayerController>(PlayerController))
			{
				DoorsDoorsDoorsPlayerController->ClientRestartGame();
			}
			RestartPlayer(PlayerController);
		}
	}
}
