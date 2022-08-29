// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveFloat.h"
#include "InteractionComponent.h"
#include "DoorInteractionComponent.generated.h"

//class ATriggerBox;
class IConsoleVariable;
class UAudioComponent;
class UTextRenderComponent;

UENUM()
enum class EDoorState
{
	DS_Closed = 0	UMETA(DisplayName = "Closed"),
	DS_Opening = 1	UMETA(DisplayName = "Opening"),
	DS_Open = 2		UMETA(DisplayName = "Open"),
	DS_Locked = 3	UMETA(DisplayName = "Locked"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DOORSDOORSDOORS_API UDoorInteractionComponent : public UInteractionComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDoorInteractionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//request to open the door
	UFUNCTION(BlueprintCallable)
		void OpenDoor();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//UInteractionComponent
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	void InteractionRequested(AActor* RequestingActor) override;

	//called internally when door has finished opening
	void OnDoorOpen();

	UFUNCTION(BlueprintCallable)
		bool IsOpen() { return DoorState == EDoorState::DS_Open; }

	void DebugDraw();
	static void OnDebugToggled(IConsoleVariable* Var);
	
	UPROPERTY(EditAnywhere)
	FRotator desiredRotation = FRotator::ZeroRotator;

	FRotator startRotation = FRotator::ZeroRotator;
	FRotator finalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	float timeToRotate = 1.0f;

	float currentRotationTime = 0.0f;

	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve openCurve;

	//UPROPERTY(EditAnywhere)
	//	ATriggerBox* triggerBox;

	UPROPERTY(BlueprintReadOnly)
		EDoorState DoorState;

	UPROPERTY()
		UAudioComponent* AudioComponent = nullptr;
	UPROPERTY()
		UTextRenderComponent* TextRenderComponent = nullptr;
};
