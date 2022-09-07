// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorsDoorsDoorsGameStateBase.h"
#include "DoorsDoorsDoorsPlayerCharacter.h"
#include "DoorsDoorsDoorsPlayerController.h"
#include "DoorsDoorsDoorsPlayerState.h"
//#include "DoorsDoorsDoorsAIController.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

void ADoorsDoorsDoorsGameStateBase::UpdateResults(ADoorsDoorsDoorsPlayerState* PlayerState, ADoorsDoorsDoorsPlayerCharacter* DoorsDoorsDoorsCharacter)
{
	if (!PlayerState || !DoorsDoorsDoorsCharacter)
	{
		return;
	}

	const bool IsWinner = Results.Num() == 0;
	PlayerState->SetIsWinner(IsWinner);
	PlayerState->SetCurrentState(EPlayerGameState::Finished);

	FGameResult Result;
	Result.Name = DoorsDoorsDoorsCharacter->GetName();
	//TODO get the actual time it took in order to post to a leaderboard/results widget
	Result.Time = 5.0f;
	Results.Add(Result);
}

//only ever called by the authority
void ADoorsDoorsDoorsGameStateBase::OnPlayerReachedEnd(ADoorsDoorsDoorsPlayerCharacter* DoorsDoorsDoorsCharacter)
{
	ensureMsgf(HasAuthority(), TEXT("ADoorsDoorsDoorsGameStateBase::OnPlayerReachedEnd being called from Non Authority!"));

	//two cases, Player or AI reaches the end
	if (ADoorsDoorsDoorsPlayerController* DoorsDoorsDoorsPlayerController = DoorsDoorsDoorsCharacter->GetController<ADoorsDoorsDoorsPlayerController>())
	{
		DoorsDoorsDoorsPlayerController->ClientReachedEnd();
		DoorsDoorsDoorsCharacter->GetCharacterMovement()->DisableMovement();
		ADoorsDoorsDoorsPlayerState* PlayerState = DoorsDoorsDoorsPlayerController->GetPlayerState<ADoorsDoorsDoorsPlayerState>();
		UpdateResults(PlayerState, DoorsDoorsDoorsCharacter);

		//TODO this will not work once JIP(Join In Progress) is enabled
		if (Results.Num() >= PlayerArray.Num())
		{
			GameState = EGameState::GameOver;
		}
	}
	//else if (ADoorsDoorsDoorsAIController* DoorsDoorsDoorsAIController = TantrumnCharacter->GetController<ADoorsDoorsDoorsAIController>())
	//{
	//	ADoorsDoorsDoorsPlayerState* PlayerState = DoorsDoorsDoorsAIController->GetPlayerState<ADoorsDoorsDoorsPlayerState>();
	//	UpdateResults(PlayerState, TantrumnCharacter);
	//	DoorsDoorsDoorsAIController->OnReachedEnd();
	//}

}

void ADoorsDoorsDoorsGameStateBase::ClearResults()
{
	Results.Empty();
}

void ADoorsDoorsDoorsGameStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	//SharedParams.bIsPushBased = true;
	//SharedParams.Condition = COND_SkipOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ADoorsDoorsDoorsGameStateBase, GameState, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ADoorsDoorsDoorsGameStateBase, Results, SharedParams);

	//DOREPLIFETIME(ADoorsDoorsDoorsPlayerCharacter, CharacterThrowState);
}

void ADoorsDoorsDoorsGameStateBase::OnRep_GameState(const EGameState& OldGameState)
{
	UE_LOG(LogTemp, Warning, TEXT("OldGameState: %s"), *UEnum::GetDisplayValueAsText(OldGameState).ToString());
	UE_LOG(LogTemp, Warning, TEXT("GameState: %s"), *UEnum::GetDisplayValueAsText(GameState).ToString());
}