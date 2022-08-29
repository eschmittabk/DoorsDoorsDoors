// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorsDoorsDoorsPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/DamageType.h"
#include "HealthComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/InputComponent.h"
#include "DamageHandlerComponent.h"

// Sets default values
ADoorsDoorsDoorsPlayerCharacter::ADoorsDoorsDoorsPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	DamageHandlerComponent = CreateDefaultSubobject<UDamageHandlerComponent>(TEXT("DamageHandlerComponent"));
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADoorsDoorsDoorsPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	//use possess/unpossess
	PC = GetWorld()->GetFirstPlayerController();
}

// Called every frame
void ADoorsDoorsDoorsPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADoorsDoorsDoorsPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//these functions fire off events
	//interaction component listens to them
	PlayerInputComponent->BindAction(FName("InteractionStart"), IE_Pressed, this, &ADoorsDoorsDoorsPlayerCharacter::InteractionStartRequested);
	PlayerInputComponent->BindAction(FName("InteractionCancel"), IE_Pressed, this, &ADoorsDoorsDoorsPlayerCharacter::InteractionCancelRequested);
}

void ADoorsDoorsDoorsPlayerCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
	HealthComponent->SetCurrentHealth(0.0f);
	OnDeath(true);
}

float ADoorsDoorsDoorsPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	UE_LOG(LogTemp, Warning, TEXT("AAbstractionPlayerCharacter::TakeDamage Damage %.2f"), Damage);
	if (HealthComponent && !HealthComponent->IsDead())
	{
		HealthComponent->TakeDamage(Damage);
		if (HealthComponent->IsDead())
		{
			OnDeath(false);
		}
	}
	return Damage;
}

const bool ADoorsDoorsDoorsPlayerCharacter::isAlive() const
{
	if (HealthComponent)
	{
		return !HealthComponent->IsDead();
	}

	return false;
}

const float ADoorsDoorsDoorsPlayerCharacter::getCurrentHealth() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetCurrentHealth();
	}

	return 0.0f;
}

void ADoorsDoorsDoorsPlayerCharacter::SetOnFire(float BaseDamage, float DamageTotalTime, float TakeDamageInterval)
{
	if (DamageHandlerComponent)
	{
		DamageHandlerComponent->TakeFireDamage(BaseDamage, DamageTotalTime, TakeDamageInterval);
	}
}

void ADoorsDoorsDoorsPlayerCharacter::OnDeath(bool IsFellOut)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController)
	{
		PlayerController->DisableInput(PlayerController);
	}

	if (!RestartLevelTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(RestartLevelTimerHandle, this, &ADoorsDoorsDoorsPlayerCharacter::OnDeathTimerFinished, TimeRestartLevelAfterDeath, false);
	}
}

void ADoorsDoorsDoorsPlayerCharacter::OnDeathTimerFinished()
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController)
	{
		PlayerController->RestartLevel();
	}
}

void ADoorsDoorsDoorsPlayerCharacter::InteractionStartRequested()
{
	OnInteractionStartRequested.Broadcast(this);
}

void ADoorsDoorsDoorsPlayerCharacter::InteractionCancelRequested()
{
	OnInteractionCancelRequested.Broadcast();
}

void ADoorsDoorsDoorsPlayerCharacter::HandleItemCollected()
{
	ItemsCollected++;
	// Play Effects here.
	//PC->PlayerCameraManager->PlayCameraShake(CamShake, 1.0f);
	//GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(CamShake);
	PC->PlayDynamicForceFeedback(ForceFeedbackIntensity, ForceFeedbackDuration, true, false, true, false,
		EDynamicForceFeedbackAction::Start);

	ItemCollected();
}