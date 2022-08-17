// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorInteractionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/TriggerBox.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UDoorInteractionComponent::UDoorInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDoorInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	startRotation = GetOwner()->GetActorRotation();
	finalRotation = GetOwner()->GetActorRotation() + desiredRotation;
	//ensure timeToRotate is greater than EPSILON
	currentRotationTime = 0.0f;
	// ...
	
}


// Called every frame
void UDoorInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (currentRotationTime < timeToRotate)
	{
		if (triggerBox && GetWorld() && GetWorld()->GetFirstLocalPlayerFromController())
		{
			APawn* playerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
			if (playerPawn && triggerBox->IsOverlappingActor(playerPawn))
			{
				currentRotationTime += DeltaTime;
				const float timeRatio = FMath::Clamp(currentRotationTime / timeToRotate, 0.0f, 1.0f);
				const float rotationAlpha = openCurve.GetRichCurveConst()->Eval(timeRatio);
				const FRotator currentRotation = FMath::Lerp(startRotation, finalRotation, rotationAlpha);
				GetOwner()->SetActorRotation(currentRotation);
			}
			/*else if (playerPawn && !triggerBox->IsOverlappingActor(playerPawn))
			{

			}*/
		}
	}
	// ...
}

