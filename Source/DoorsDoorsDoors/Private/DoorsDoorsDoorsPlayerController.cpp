// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorsDoorsDoorsPlayerController.h"
#include "DoorsDoorsDoorsPlayerCharacter.h"
#include "DoorsDoorsDoorsGameModeBase.h"
//#include "DoorsDoorsDoorsGameInstance.h"
#include "DoorsDoorsDoorsGameStateBase.h"
#include "DoorsDoorsDoorsPlayerState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

static TAutoConsoleVariable<bool> CVarDisplayLaunchInputDelta(
	TEXT("DoorsDoorsDoors.Character.Debug.DisplayLaunchInputDelta"),
	false,
	TEXT("Display Launch Input Delta"),
	ECVF_Default);

void ADoorsDoorsDoorsPlayerController::BeginPlay()
{
	Super::BeginPlay();
	DoorsDoorsDoorsGameState = GetWorld()->GetGameState<ADoorsDoorsDoorsGameStateBase>();
	//ensureMsgf(DoorsDoorsDoorsGameState, TEXT("ADoorsDoorsDoorsPlayerController::BeginPlay Invalid DoorsDoorsDoorsGameState"));
}
//called from gamemode, so only on authority will we get these calls
void ADoorsDoorsDoorsPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	UE_LOG(LogTemp, Warning, TEXT("OnPossess: %s"), *GetName());
}

void ADoorsDoorsDoorsPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogTemp, Warning, TEXT("OnUnPossess: %s"), *GetName());
}

void ADoorsDoorsDoorsPlayerController::ClientDisplayCountdown_Implementation(float GameCountdownDuration, TSubclassOf<UDoorsDoorsDoorsGameWidget> InGameWidgetClass)
{
	if (!DoorsDoorsDoorsGameWidget)
	{
		DoorsDoorsDoorsGameWidget = CreateWidget<UDoorsDoorsDoorsGameWidget>(this, InGameWidgetClass);
	}

	if (DoorsDoorsDoorsGameWidget)
	{
		DoorsDoorsDoorsGameWidget->AddToPlayerScreen();
		DoorsDoorsDoorsGameWidget->StartCountdown(GameCountdownDuration, this);
	}
}

void ADoorsDoorsDoorsPlayerController::ClientRestartGame_Implementation()
{
	if (DoorsDoorsDoorsGameWidget)
	{
		DoorsDoorsDoorsGameWidget->RemoveResults();
		//restore game input 
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
	}
}

void ADoorsDoorsDoorsPlayerController::ClientReachedEnd_Implementation()
{
	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		//Player->ServerPlayCelebrateMontage();
		DDDPlayer->GetCharacterMovement()->DisableMovement();

		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(true);

		if (DoorsDoorsDoorsGameWidget)
		{
			DoorsDoorsDoorsGameWidget->DisplayResults();
		}
	}
}

void ADoorsDoorsDoorsPlayerController::OnRetrySelected()
{
	ServerRestartLevel();
}

void ADoorsDoorsDoorsPlayerController::ServerRestartLevel_Implementation()
{
	ADoorsDoorsDoorsGameModeBase* DoorsDoorsDoorsGameMode = GetWorld()->GetAuthGameMode<ADoorsDoorsDoorsGameModeBase>();
	if (ensureMsgf(DoorsDoorsDoorsGameMode, TEXT("ADoorsDoorsDoorsPlayerController::ServerRestartLevel_Implementation Invalid GameMode")))
	{
		DoorsDoorsDoorsGameMode->RestartGame();

	}
}

void ADoorsDoorsDoorsPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		if (HUDClass)
		{
			HUDWidget = CreateWidget(this, HUDClass);
			if (HUDWidget)
			{
				//HUDWidget->AddToViewport();
				HUDWidget->AddToPlayerScreen();
			}
		}
	}

}

void ADoorsDoorsDoorsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		InputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ADoorsDoorsDoorsPlayerController::RequestJump);
		InputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Released, this, &ADoorsDoorsDoorsPlayerController::RequestStopJump);

		InputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &ADoorsDoorsDoorsPlayerController::RequestCrouchStart);
		InputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Released, this, &ADoorsDoorsDoorsPlayerController::RequestCrouchEnd);

		InputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Pressed, this, &ADoorsDoorsDoorsPlayerController::RequestSprintStart);
		InputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Released, this, &ADoorsDoorsDoorsPlayerController::RequestSprintEnd);

		InputComponent->BindAction(TEXT("PullorAimObject"), EInputEvent::IE_Pressed, this, &ADoorsDoorsDoorsPlayerController::RequestPullorAimObject);
		InputComponent->BindAction(TEXT("PullorAimObject"), EInputEvent::IE_Released, this, &ADoorsDoorsDoorsPlayerController::RequestStopPullorAimObject);

		InputComponent->BindAxis(TEXT("MoveForward/Backward"), this, &ADoorsDoorsDoorsPlayerController::RequestMoveForward);
		InputComponent->BindAxis(TEXT("MoveRight/Left"), this, &ADoorsDoorsDoorsPlayerController::RequestMoveRight);
		InputComponent->BindAxis(TEXT("LookUp"), this, &ADoorsDoorsDoorsPlayerController::RequestLookUp);
		InputComponent->BindAxis(TEXT("Turn"), this, &ADoorsDoorsDoorsPlayerController::RequestLookRight);
		InputComponent->BindAxis(TEXT("ThrowObject"), this, &ADoorsDoorsDoorsPlayerController::RequestThrowObject);

	}
}

bool ADoorsDoorsDoorsPlayerController::CanProcessRequest() const
{
	if (DoorsDoorsDoorsGameState && DoorsDoorsDoorsGameState->IsPlaying())
	{
		if (ADoorsDoorsDoorsPlayerState* DoorsDoorsDoorsPlayerState = GetPlayerState<ADoorsDoorsDoorsPlayerState>())
		{
			return (DoorsDoorsDoorsPlayerState->GetCurrentState() == EPlayerGameState::Playing);
		}
	}

	return false;
}

void ADoorsDoorsDoorsPlayerController::RequestMoveForward(float AxisValue)
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (AxisValue != 0.f)
	{
		FRotator const ControlSpaceRot = GetControlRotation();
		// transform to world space and add it
		GetPawn()->AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X), AxisValue);
	}
}

void ADoorsDoorsDoorsPlayerController::RequestMoveRight(float AxisValue)
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (AxisValue != 0.f)
	{
		FRotator const ControlSpaceRot = GetControlRotation();
		// transform to world space and add it
		GetPawn()->AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Y), AxisValue);
	}
}

void ADoorsDoorsDoorsPlayerController::RequestLookUp(float AxisValue)
{
	AddPitchInput(AxisValue * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ADoorsDoorsDoorsPlayerController::RequestLookRight(float AxisValue)
{
	AddYawInput(AxisValue * BaseLookRightRate * GetWorld()->GetDeltaSeconds());
}

void ADoorsDoorsDoorsPlayerController::RequestThrowObject(float AxisValue)
{
	//if (!CanProcessRequest())
	//{
	//	return;
	//}

	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		if (DDDPlayer->CanThrowObject())
		{
			float currentDelta = AxisValue - LastAxis;

			//debug
			if (CVarDisplayLaunchInputDelta->GetBool())
			{
				if (fabs(currentDelta) > 0.0f)
				{
					UE_LOG(LogTemp, Warning, TEXT("Axis: %f LastAxis: %f currentDelta: %f"), AxisValue, LastAxis, currentDelta);
				}
			}
			LastAxis = AxisValue;
			//prevent the case where we hold the axis at the threshold and then release
			//perhaps rolling average
			const bool IsFlick = fabs(currentDelta) > FlickThreshold;
			if (IsFlick)
			{
				if (currentDelta > 0)
				{
					DDDPlayer->RequestThrowObject();
				}
				else
				{
					DDDPlayer->RequestUseObject();
				}
			}
		}
		else
		{
			LastAxis = 0.0f;
		}
	}
}

void ADoorsDoorsDoorsPlayerController::RequestPullorAimObject()
{
	//if (!CanProcessRequest())
	//{
	//	return;
	//}

	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		if (DDDPlayer->CanAim())
		{
			DDDPlayer->RequestAim();
		}
		else
		{
			DDDPlayer->RequestPullObject();
		}

	}
}

void ADoorsDoorsDoorsPlayerController::RequestStopPullorAimObject()
{
	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		if (DDDPlayer->IsAiming())
		{
			DDDPlayer->RequestStopAim();
		}
		else
		{
			DDDPlayer->RequestStopPullObject();
		}

	}
}

void ADoorsDoorsDoorsPlayerController::RequestJump()
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		DDDPlayer->Jump();

		//SoundCue Triggers
		if (JumpSound && DDDPlayer->GetCharacterMovement()->IsMovingOnGround())
		{
			FVector CharacterLocation = DDDPlayer->GetActorLocation();
			UGameplayStatics::PlaySoundAtLocation(this, JumpSound, CharacterLocation);
		}
	}
}

void ADoorsDoorsDoorsPlayerController::RequestStopJump()
{
	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		DDDPlayer->StopJumping();
	}
}

void ADoorsDoorsDoorsPlayerController::RequestCrouchStart()
{
	//if (!CanProcessRequest())
	//{
	//	return;
	//}

	ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter());
	if (!DDDPlayer || !DDDPlayer->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	DDDPlayer->Crouch();
}

void ADoorsDoorsDoorsPlayerController::RequestCrouchEnd()
{
	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		DDDPlayer->UnCrouch();
	}
}

void ADoorsDoorsDoorsPlayerController::RequestSprintStart()
{
	//if (!CanProcessRequest())
	//{
	//	return;
	//}

	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		DDDPlayer->RequestSprintStart();
	}
}

void ADoorsDoorsDoorsPlayerController::RequestSprintEnd()
{
	if (ADoorsDoorsDoorsPlayerCharacter* DDDPlayer = Cast<ADoorsDoorsDoorsPlayerCharacter>(GetCharacter()))
	{
		DDDPlayer->RequestSprintEnd();
	}
}