// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectiveComponent.generated.h"

class UObjectiveComponent;

UENUM()
enum class EObjectiveState
{
	OS_Inactive = 0		UMETA(DisplayName = "Inactive"),
	OS_Active = 1		UMETA(DisplayName = "Active"),
	OS_Completed = 2	UMETA(DisplayName = "Completed"),
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnObjectiveStateChanged, const UObjectiveComponent*, EObjectiveState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DOORSDOORSDOORS_API UObjectiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UObjectiveComponent();

	UFUNCTION(BlueprintCallable)
		const FString& GetDescription() const { return Description; }

	FOnObjectiveStateChanged OnStateChanged;

	UFUNCTION(BlueprintCallable)
		EObjectiveState GetState() const { return State; }

	void SetState(EObjectiveState NewState);
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeComponent() override;

	UPROPERTY(EditAnywhere)
		FString Description;

	UPROPERTY(EditAnywhere)
		EObjectiveState State;

};