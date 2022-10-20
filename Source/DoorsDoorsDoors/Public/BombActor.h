// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableActor.h"
#include "BombActor.generated.h"

class UDealDamageComponent;
class UParticleSystemComponent;

UCLASS()
class DOORSDOORSDOORS_API ABombActor : public AThrowableActor
{
	GENERATED_BODY()

public:
	ABombActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		bool Ignite();

	UPROPERTY(EditAnywhere)
		float IgniteTime = 5.0f;
	
	float CurrentTimer = 0.0f;

	UPROPERTY(EditAnywhere)
		UDealDamageComponent* DealDamageComponent;
	UPROPERTY(EditAnywhere)
		UParticleSystemComponent* ParticleSystemComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	float explodeRadius = 1000.0f;
	bool bIsIgnited = false;
};
