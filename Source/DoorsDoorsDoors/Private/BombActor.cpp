// Fill out your copyright notice in the Description page of Project Settings.


#include "BombActor.h"
#include "DealDamageComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "DoorsDoorsDoorsPlayerCharacter.h"

// Sets default values
ABombActor::ABombActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DealDamageComponent = CreateDefaultSubobject<UDealDamageComponent>(TEXT("Deal Damage"));
	DealDamageComponent->SetActive(false);
	if (DealDamageComponent->GetTriggerCapsule())
	{
		DealDamageComponent->GetTriggerCapsule()->SetCapsuleSize(explodeRadius, explodeRadius);
		DealDamageComponent->GetTriggerCapsule()->SetupAttachment(RootComponent);
		DealDamageComponent->GetTriggerCapsule()->AddRelativeLocation({ 0.0f, 0.0f, 50.0f });
		//RootComponent = DealDamageComponent->GetTriggerCapsule();
	}
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle System"));
	ParticleSystemComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABombActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (DealDamageComponent)
	{
		if (bIsIgnited)
		{
			CurrentTimer += DeltaTime;
			if (CurrentTimer >= IgniteTime)
			{
				if (ParticleSystemComponent)
				{
					ParticleSystemComponent->ToggleActive();
				}
				DealDamageComponent->SetActive(!DealDamageComponent->IsActive());
				CurrentTimer = 0.0f;
				this->StaticMeshComponent->DestroyComponent();
				bIsIgnited = false;
			}
		}
	}
}

bool ABombActor::Ignite()
{
	return bIsIgnited = true;
}