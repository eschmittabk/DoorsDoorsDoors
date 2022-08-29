// Fill out your copyright notice in the Description page of Project Settings.


#include "DealDamageComponent.h"
#include "Components/CapsuleComponent.h"
#include "DoorsDoorsDoorsPlayerCharacter.h"
#include "Engine/EngineBaseTypes.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UDealDamageComponent::UDealDamageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TriggerCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Trigger Capsule"));
	//TriggerCapsule->SetupAttachment(RootComponent);

	TriggerCapsule->OnComponentBeginOverlap.AddDynamic(this, &UDealDamageComponent::OnOverlapBegin);
	TriggerCapsule->OnComponentEndOverlap.AddDynamic(this, &UDealDamageComponent::OnOverlapEnd);
}

// Called when the game starts
void UDealDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bActive && bPlayerOverlapped)
	{
		ADoorsDoorsDoorsPlayerCharacter* PlayerCharacter = Cast<ADoorsDoorsDoorsPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (PlayerCharacter)
		{
			PlayerCharacter->SetOnFire(BaseDamage, DamageTotalTime, TakeDamageInterval);
		}
	}
}

// Called when the game starts
void UDealDamageComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDealDamageComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("UDealDamageComponent::OnOverlapBegin"));

	//if (OtherActor == GetOwner())
	//{
	//	return;
	//}

	//if (!IsActive())
	//{
	//	return;
	//}

	ADoorsDoorsDoorsPlayerCharacter* PlayerCharacter = Cast<ADoorsDoorsDoorsPlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		/*PlayerCharacter->SetOnFire(BaseDamage, DamageTotalTime, TakeDamageInterval);*/
		bPlayerOverlapped = true;
	}
}

void UDealDamageComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("UDealDamageComponent::OnOverlapEnd"));

	ADoorsDoorsDoorsPlayerCharacter* PlayerCharacter = Cast<ADoorsDoorsDoorsPlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		bPlayerOverlapped = false;
	}
}
