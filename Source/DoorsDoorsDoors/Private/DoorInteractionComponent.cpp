// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorInteractionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "ObjectiveComponent.h"
#include "InteractionComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TextRenderComponent.h"
#include "DoorsDoorsDoorsPlayerCharacter.h"

constexpr float FLT_METERS(float meters) { return meters * 100.0f; }

static TAutoConsoleVariable<bool> CVarToggleDebugDoor(
	TEXT("Abstraction.DoorInteractionComponent.Debug"),
	false,
	TEXT("Toggle DoorInteractionComponent debug display."),
	ECVF_Default);

// Sets default values for this component's properties
UDoorInteractionComponent::UDoorInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	DoorState = EDoorState::DS_Closed;

	CVarToggleDebugDoor.AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&UDoorInteractionComponent::OnDebugToggled));
}


// Called when the game starts
void UDoorInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	startRotation = GetOwner()->GetActorRotation();
	finalRotation = GetOwner()->GetActorRotation() + desiredRotation;
	//ensure TimeToRotate is greater than EPSILON
	currentRotationTime = 0.0f;

	AudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();
	//check(AudioComponent);
	if (!AudioComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDoorInteractionComponent::BeginPlay() Missing Audio Component"));
	}

	TextRenderComponent = GetOwner()->FindComponentByClass<UTextRenderComponent>();
	
}


// Called every frame
void UDoorInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DoorState == EDoorState::DS_Opening)
	{
		currentRotationTime += DeltaTime;
		const float TimeRatio = FMath::Clamp(currentRotationTime / timeToRotate, 0.0f, 1.0f);
		const float RotationAlpha = openCurve.GetRichCurveConst()->Eval(TimeRatio);
		const FRotator CurrentRotation = FMath::Lerp(startRotation, finalRotation, RotationAlpha);
		GetOwner()->SetActorRotation(CurrentRotation);
		if (TimeRatio >= 1.0f)
		{
			OnDoorOpen();
		}
	}

	DebugDraw();
}

void UDoorInteractionComponent::OpenDoor()
{
	if (IsOpen() || DoorState == EDoorState::DS_Opening)
	{
		return;
	}

	if (AudioComponent)
	{
		AudioComponent->Play();
	}

	DoorState = EDoorState::DS_Opening;
	currentRotationTime = 0.0f;
}

void UDoorInteractionComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("UDoorInteractionComponent::OnOverlapBegin"));
	//we already have somebody interacting, currently we don't support multiple interactions
	if (InteractingActor.IsValid() || !bActive)
	{
		return;
	}

	//for now we will get that component and set visible
	if (OtherActor->ActorHasTag("Player"))
	{
		InteractingActor = OtherActor;
		if (TextRenderComponent)
		{
			TextRenderComponent->SetText(InteractionPrompt);
			TextRenderComponent->SetVisibility(true);
		}
	}
}

void UDoorInteractionComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("UDoorInteractionComponent::OnOverlapEnd"));
	if (OtherActor == InteractingActor)
	{
		InteractingActor = nullptr;
		if (TextRenderComponent)
		{
			TextRenderComponent->SetVisibility(false);
		}
	}
}

void UDoorInteractionComponent::InteractionRequested(AActor* RequestingActor)
{
	//ideally we would make sure this is allowed
	if (InteractingActor.IsValid() && InteractingActor == RequestingActor)
	{
		bActive = false;
		if (TextRenderComponent)
		{
			TextRenderComponent->SetText(InteractionPrompt);
			TextRenderComponent->SetVisibility(false);
		}

		ADoorsDoorsDoorsPlayerCharacter* DDDPC = Cast<ADoorsDoorsDoorsPlayerCharacter>(InteractingActor);
		if (DDDPC)
		{
			DDDPC->DoorOpenInteractionStarted(GetOwner());
		}

		//this will be called from the owner to be in sync with animation
		//OpenDoor();
	}
}

void UDoorInteractionComponent::OnDoorOpen()
{
	DoorState = EDoorState::DS_Open;
	UObjectiveComponent* ObjectiveComponent = GetOwner()->FindComponentByClass<UObjectiveComponent>();
	if (ObjectiveComponent)
	{
		ObjectiveComponent->SetState(EObjectiveState::OS_Completed);
	}

	//hide prompt
	//disable interaction

	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("DoorOpened"));
	//tell any listeners that the interaction is successful
	InteractionSuccess.Broadcast();
}

void UDoorInteractionComponent::OnDebugToggled(IConsoleVariable* Var)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDebugToggled"));
}

void UDoorInteractionComponent::DebugDraw()
{
	if (CVarToggleDebugDoor->GetBool())
	{
		FVector Offset(FLT_METERS(-0.75f), 0.0f, FLT_METERS(2.5f));
		FVector StartLocation = GetOwner()->GetActorLocation() + Offset;
		FString EnumAsString = TEXT("Door State: ") + UEnum::GetDisplayValueAsText(DoorState).ToString();
		DrawDebugString(GetWorld(), Offset, EnumAsString, GetOwner(), FColor::Blue, 0.0f);
	}
}