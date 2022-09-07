// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "InteractableDoor.generated.h"

class UDoorInteractionComponent;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoorOpen);

UCLASS()
class DOORSDOORSDOORS_API AInteractableDoor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	AInteractableDoor();
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category = "Door Interaction")
		FOnDoorOpen OnDoorOpen;

	UFUNCTION(BlueprintCallable)
		void OpenDoor();
protected:
	UFUNCTION()
		void OnInteractionSuccess();

	UPROPERTY(EditAnywhere, NoClear)
		UDoorInteractionComponent* DoorInteractionComponent;

	UPROPERTY(EditAnywhere)
		float doorRadius = 65.0f;

	UPROPERTY(EditAnywhere)
		float doorHalfHeight = 115.0f;

	UPROPERTY(EditAnywhere)
		float deltaXLocation = -50.0f;

	UPROPERTY(EditAnywhere)
		float deltaYLocation = 0.0f;

	UPROPERTY(EditAnywhere)
		float deltaZLocation = 100.0f;

	UPROPERTY(EditAnywhere)
		UAudioComponent* AudioComponent;
	
};
