// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveFloat.h"
#include "DoorInteractionComponent.generated.h"

class ATriggerBox;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DOORSDOORSDOORS_API UDoorInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDoorInteractionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	FRotator desiredRotation = FRotator::ZeroRotator;

	FRotator startRotation = FRotator::ZeroRotator;
	FRotator finalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	float timeToRotate = 1.0f;

	float currentRotationTime = 0.0f;
	
	UPROPERTY(EditAnywhere)
	ATriggerBox* triggerBox;

	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve openCurve;
};
