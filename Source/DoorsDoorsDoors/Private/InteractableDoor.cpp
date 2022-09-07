// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableDoor.h"
#include "DoorInteractionComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"

AInteractableDoor::AInteractableDoor()
{
	DoorInteractionComponent = CreateDefaultSubobject<UDoorInteractionComponent>(TEXT("DoorInteractionComponent"));
	if (DoorInteractionComponent->GetTriggerCapsule())
	{
		DoorInteractionComponent->GetTriggerCapsule()->SetCapsuleSize(doorRadius, doorHalfHeight);
		DoorInteractionComponent->GetTriggerCapsule()->SetupAttachment(RootComponent);
		DoorInteractionComponent->GetTriggerCapsule()->AddRelativeLocation({ deltaXLocation, deltaYLocation, deltaZLocation });
	}

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
}

void AInteractableDoor::BeginPlay()
{
	Super::BeginPlay();
	DoorInteractionComponent->InteractionSuccess.AddDynamic(this, &AInteractableDoor::OnInteractionSuccess);
}

void AInteractableDoor::OpenDoor()
{
	DoorInteractionComponent->OpenDoor();
}

void AInteractableDoor::OnInteractionSuccess()
{
	OnDoorOpen.Broadcast();
}


