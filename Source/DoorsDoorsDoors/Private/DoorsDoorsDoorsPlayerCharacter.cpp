// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorsDoorsDoorsPlayerCharacter.h"
#include "DoorsDoorsDoorsPlayerController.h"
#include "DoorsDoorsDoorsPlayerState.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Components/InputComponent.h"
#include "Particles/ParticleSystemComponent.h"

#include "HealthComponent.h"
#include "DamageHandlerComponent.h"
#include "ThrowableActor.h"

#include "VisualLogger/VisualLogger.h"

constexpr int CVSphereCastPlayerView = 0;
constexpr int CVSphereCastActorTransform = 1;
constexpr int CVLineCastActorTransform = 2;

//add cvars for debug
static TAutoConsoleVariable<int> CVarTraceMode(
	TEXT("DoorsDoorsDoors.Character.Debug.TraceMode"),
	0,
	TEXT("    0: Sphere cast PlayerView is used for direction/rotation (default).\n")
	TEXT("    1: Sphere cast using ActorTransform \n")
	TEXT("    2: Line cast using ActorTransform \n"),
	ECVF_Default);


static TAutoConsoleVariable<bool> CVarDisplayTrace(
	TEXT("DoorsDoorsDoors.Character.Debug.DisplayTrace"),
	false,
	TEXT("Display Trace"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarDisplayThrowVelocity(
	TEXT("DoorsDoorsDoors.Character.Debug.DisplayThrowVelocity"),
	false,
	TEXT("Display Throw Velocity"),
	ECVF_Default);

DEFINE_LOG_CATEGORY_STATIC(LogDoorsDoorsDoorsChar, Verbose, Verbose)

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

	bReplicates = true;
	SetReplicateMovement(true);
}

void ADoorsDoorsDoorsPlayerCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.Condition = COND_SkipOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ADoorsDoorsDoorsPlayerCharacter, CharacterThrowState, SharedParams);

	//SharedParams.Condition = COND_None;
	//DOREPLIFETIME_WITH_PARAMS_FAST(ADoorsDoorsDoorsPlayerCharacter, bIsBeingRescued, SharedParams);
	//DOREPLIFETIME_WITH_PARAMS_FAST(ADoorsDoorsDoorsPlayerCharacter, LastGroundPosition, SharedParams);
}

// Called when the game starts or when spawned
void ADoorsDoorsDoorsPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	//use possess/unpossess
	PC = GetWorld()->GetFirstPlayerController();

	EffectCooldown = DefautlEffectCooldown;
	if (GetCharacterMovement())
	{
		MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}
}

// Called every frame
void ADoorsDoorsDoorsPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//this is done on the clients to ensure the anim looks correct
	//no need to spam network traffic with curve value
	if (CharacterThrowState == ECharacterThrowState::Throwing)
	{
		UpdateThrowMontagePlayRate();
		return;
	}

	if (!IsLocallyControlled())
	{
		return;
	}

	//these should run on the authority to prevent cheating
	if (bIsStunned)
	{
		UpdateStun(DeltaTime);
		return;
	}

	if (bIsUnderEffect)
	{
		UpdateEffect(DeltaTime);
		return;
	}
	//~move to authority, and place the start as a server rpc

	//move to a function and improve this in the future
	//only locallly controlled character needs to worry about below code
	if (CharacterThrowState == ECharacterThrowState::None || CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		switch (CVarTraceMode->GetInt())
		{
		case CVSphereCastPlayerView:
			SphereCastPlayerView();
			break;
		case CVSphereCastActorTransform:
			SphereCastActorTransform();
			break;
		case CVLineCastActorTransform:
			LineCastActorTransform();
			break;
		default:
			SphereCastPlayerView();
			break;
		}
	}
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

void ADoorsDoorsDoorsPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	//this should only be authority or maybe locally controlled...

	//custom landed code
	ADoorsDoorsDoorsPlayerController* DoorsDoorsDoorsPlayerController = GetController<ADoorsDoorsDoorsPlayerController>();
	if (DoorsDoorsDoorsPlayerController)
	{
		const float FallImpactSpeed = FMath::Abs(GetVelocity().Z);
		if (FallImpactSpeed < MinImpactSpeed)
		{
			//nothing to do, very light fall
			return;
		}
		else
		{
			//SoundCue Triggers
			if (HeavyLandSound && GetOwner())
			{
				FVector CharacterLocation = GetOwner()->GetActorLocation();
				UGameplayStatics::PlaySoundAtLocation(this, HeavyLandSound, CharacterLocation);
			}
		}

		const float DeltaImpact = MaxImpactSpeed - MinImpactSpeed;
		const float FallRatio = FMath::Clamp((FallImpactSpeed - MinImpactSpeed) / DeltaImpact, 0.0f, 1.0f);
		const bool bAffectSmall = FallRatio <= 0.5;
		const bool bAffectLarge = FallRatio > 0.5;
		DoorsDoorsDoorsPlayerController->PlayDynamicForceFeedback(FallRatio, 0.5f, bAffectLarge, bAffectSmall, bAffectLarge, bAffectSmall);

		if (bAffectLarge)
		{
			OnStunBegin(FallRatio);
		}
	}
}

void ADoorsDoorsDoorsPlayerCharacter::FellOutOfWorld(const UDamageType& dmgType)
{
	HealthComponent->SetCurrentHealth(0.0f);
	OnDeath(true);
}


void ADoorsDoorsDoorsPlayerCharacter::RequestSprintStart()
{
	if (!bIsStunned)
	{
		//handle the local speed as we are still controlling this character and want to avoid any stutter between client and server
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		bIsSprinting = true;
		ServerSprintStart();
	}

}
void ADoorsDoorsDoorsPlayerCharacter::RequestSprintEnd()
{
	//handle the local speed as we are still controlling this character and want to avoid any stutter between client and server
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	bIsSprinting = false;
	ServerSprintEnd();
}

void ADoorsDoorsDoorsPlayerCharacter::ServerSprintStart_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ADoorsDoorsDoorsPlayerCharacter::ServerSprintEnd_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
}

void ADoorsDoorsDoorsPlayerCharacter::RequestThrowObject()
{
	if (CanThrowObject())
	{
		//to give a responsive feel start playing on the locally owned Actor
		if (PlayThrowMontage())
		{
			CharacterThrowState = ECharacterThrowState::Throwing;
			//now play on all clients
			ServerRequestThrowObject();

		}
		else
		{
			ResetThrowableObject();
		}
	}
}

bool ADoorsDoorsDoorsPlayerCharacter::ServerRequestThrowObject_Validate()
{
	//can check the state or if the throwable actor exists etc to prevent this being broadcasted
	return true;
}

void ADoorsDoorsDoorsPlayerCharacter::ServerRequestThrowObject_Implementation()
{
	//server needs to call the multicast
	MulticastRequestThrowObject();
}

void ADoorsDoorsDoorsPlayerCharacter::MulticastRequestThrowObject_Implementation()
{
	//locally controlled actor has already set up binding and played montage
	if (IsLocallyControlled())
	{
		return;
	}

	PlayThrowMontage();
	//if we were aiming transition out of that camera
	CharacterThrowState = ECharacterThrowState::Throwing;
}

void ADoorsDoorsDoorsPlayerCharacter::RequestPullObject()
{
	//make sure we are in idle
	if (!bIsStunned && CharacterThrowState == ECharacterThrowState::None)
	{
		CharacterThrowState = ECharacterThrowState::RequestingPull;
		ServerRequestPullObject(true);
	}
}

void ADoorsDoorsDoorsPlayerCharacter::RequestAim()
{
	if (!bIsStunned && CharacterThrowState == ECharacterThrowState::Attached)
	{
		CharacterThrowState = ECharacterThrowState::Aiming;
		ServerRequestToggleAim(true);
	}
}

bool ADoorsDoorsDoorsPlayerCharacter::AttemptPullObjectAtLocation(const FVector& InLocation)
{
	if (CharacterThrowState != ECharacterThrowState::None && CharacterThrowState != ECharacterThrowState::RequestingPull)
	{
		return false;
	}

	FVector StartPos = GetActorLocation();
	FVector EndPos = InLocation;
	FHitResult HitResult;
	GetWorld() ? GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_Visibility) : false;
#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		DrawDebugLine(GetWorld(), StartPos, EndPos, HitResult.bBlockingHit ? FColor::Red : FColor::White, false);
	}
#endif
	CharacterThrowState = ECharacterThrowState::RequestingPull;
	ProcessTraceResult(HitResult, false);
	if (CharacterThrowState == ECharacterThrowState::Pulling)
	{
		return true;
	}

	CharacterThrowState = ECharacterThrowState::None;
	return false;
}

void ADoorsDoorsDoorsPlayerCharacter::RequestStopPullObject()
{
	//if was pulling an object, drop it
	if (CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		CharacterThrowState = ECharacterThrowState::None;
		ServerRequestPullObject(false);
		//ResetThrowableObject();
	}
}

void ADoorsDoorsDoorsPlayerCharacter::RequestStopAim()
{
	if (CharacterThrowState == ECharacterThrowState::Aiming)
	{
		CharacterThrowState = ECharacterThrowState::Attached;
		ServerRequestToggleAim(false);
	}
}

void ADoorsDoorsDoorsPlayerCharacter::ServerRequestToggleAim_Implementation(bool IsAiming)
{
	CharacterThrowState = IsAiming ? ECharacterThrowState::Aiming : ECharacterThrowState::Attached;
}

void ADoorsDoorsDoorsPlayerCharacter::ServerRequestPullObject_Implementation(bool bIsPulling)
{
	CharacterThrowState = bIsPulling ? ECharacterThrowState::RequestingPull : ECharacterThrowState::None;
}

void ADoorsDoorsDoorsPlayerCharacter::ServerPullObject_Implementation(AThrowableActor* InThrowableActor)
{
	if (InThrowableActor && InThrowableActor->Pull(this))
	{
		CharacterThrowState = ECharacterThrowState::Pulling;
		ThrowableActor = InThrowableActor;
		ThrowableActor->ToggleHighlight(false);
	}
}

void ADoorsDoorsDoorsPlayerCharacter::ClientThrowableAttached_Implementation(AThrowableActor* InThrowableActor)
{
	CharacterThrowState = ECharacterThrowState::Attached;
	ThrowableActor = InThrowableActor;
	MoveIgnoreActorAdd(ThrowableActor);
}

void ADoorsDoorsDoorsPlayerCharacter::ServerBeginThrow_Implementation()
{
	//ignore collisions otherwise the throwable object hits the player capsule and doesn't travel in the desired direction
	if (ThrowableActor->GetRootComponent())
	{
		UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent());
		if (RootPrimitiveComponent)
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, true);
		}
	}
	//const FVector& Direction = GetMesh()->GetSocketRotation(TEXT("ObjectAttach")).Vector() * -ThrowSpeed;
	const FVector& Direction = GetActorForwardVector() * ThrowSpeed;
	ThrowableActor->Launch(Direction);

	if (CVarDisplayThrowVelocity->GetBool())
	{
		const FVector& Start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
		DrawDebugLine(GetWorld(), Start, Start + Direction, FColor::Red, false, 5.0f);
	}

	const FVector& Start = GetMesh()->GetSocketLocation(TEXT("ObjectAttach"));
	UE_VLOG_ARROW(this, LogDoorsDoorsDoorsChar, Verbose, Start, Start + Direction, FColor::Red, TEXT("Throw Direction"));


}

void ADoorsDoorsDoorsPlayerCharacter::ServerFinishThrow_Implementation()
{
	//put all this in a function that runs on the server
	CharacterThrowState = ECharacterThrowState::None;
	//this only happened on the locally controlled actor
	MoveIgnoreActorRemove(ThrowableActor);
	if (ThrowableActor->GetRootComponent())
	{
		UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent());
		if (RootPrimitiveComponent)
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, false);
		}
	}
	ThrowableActor = nullptr;
}

void ADoorsDoorsDoorsPlayerCharacter::ResetThrowableObject()
{
	//drop object
	if (ThrowableActor)
	{
		ThrowableActor->Drop();
	}
	CharacterThrowState = ECharacterThrowState::None;
	ThrowableActor = nullptr;
}

void ADoorsDoorsDoorsPlayerCharacter::RequestUseObject()
{
	ApplyEffect_Implementation(ThrowableActor->GetEffectType(), true);
	ThrowableActor->Destroy();
	ResetThrowableObject();
}

void ADoorsDoorsDoorsPlayerCharacter::OnThrowableAttached(AThrowableActor* InThrowableActor)
{
	CharacterThrowState = ECharacterThrowState::Attached;
	ThrowableActor = InThrowableActor;
	MoveIgnoreActorAdd(ThrowableActor);
	ClientThrowableAttached(InThrowableActor);
	//InThrowableActor->ToggleHighlight(false);
}

//EXERCISE - REPLICATE STUN
void ADoorsDoorsDoorsPlayerCharacter::NotifyHitByThrowable(AThrowableActor* InThrowable)
{
	OnStunBegin(1.0f);
}

bool ADoorsDoorsDoorsPlayerCharacter::IsHovering() const
{
	if (ADoorsDoorsDoorsPlayerState* DoorsDoorsDoorsPlayerState = GetPlayerState<ADoorsDoorsDoorsPlayerState>())
	{
		return DoorsDoorsDoorsPlayerState->GetCurrentState() != EPlayerGameState::Playing;
	}

	return false;
}

void ADoorsDoorsDoorsPlayerCharacter::SphereCastPlayerView()
{
	FVector Location;
	FRotator Rotation;
	if (!GetController())
	{
		return;
	}
	GetController()->GetPlayerViewPoint(Location, Rotation);
	const FVector PlayerViewForward = Rotation.Vector();
	const float AdditionalDistance = (Location - GetActorLocation()).Size();
	FVector EndPos = Location + (PlayerViewForward * (1000.0f + AdditionalDistance));

	const FVector CharacterForward = GetActorForwardVector();
	const float DotResult = FVector::DotProduct(PlayerViewForward, CharacterForward);
	//prevent picking up objects behind us, this is when the camera is looking directly at the characters front side
	if (DotResult < -0.23f)
	{
		if (ThrowableActor)
		{
			ThrowableActor->ToggleHighlight(false);
			ThrowableActor = nullptr;
		}
		return;
		//UE_LOG(LogTemp, Warning, TEXT("Dot Result: %f"), DotResult);
	}


	FHitResult HitResult;
	EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), Location, EndPos, 70.0f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, DebugTrace, HitResult, true);
	ProcessTraceResult(HitResult);

#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		static float FovDeg = 90.0f;
		DrawDebugCamera(GetWorld(), Location, Rotation, FovDeg);
		DrawDebugLine(GetWorld(), Location, EndPos, HitResult.bBlockingHit ? FColor::Red : FColor::White);
		DrawDebugPoint(GetWorld(), EndPos, 70.0f, HitResult.bBlockingHit ? FColor::Red : FColor::White);
	}
#endif

}

void ADoorsDoorsDoorsPlayerCharacter::SphereCastActorTransform()
{
	FVector StartPos = GetActorLocation();
	FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);

	//sphere trace
	EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	FHitResult HitResult;
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartPos, EndPos, 70.0f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, TArray<AActor*>(), DebugTrace, HitResult, true);
	ProcessTraceResult(HitResult);
}

void ADoorsDoorsDoorsPlayerCharacter::LineCastActorTransform()
{

	FVector StartPos = GetActorLocation();
	FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);
	FHitResult HitResult;
	GetWorld() ? GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_Visibility) : false;
#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		DrawDebugLine(GetWorld(), StartPos, EndPos, HitResult.bBlockingHit ? FColor::Red : FColor::White, false);
	}
#endif
	ProcessTraceResult(HitResult);
}

void ADoorsDoorsDoorsPlayerCharacter::ProcessTraceResult(const FHitResult& HitResult, bool bHighlight /* = true */)
{
	//check if there was an existing throwable actor
	//remove the hightlight to avoid wrong feedback 
	AThrowableActor* HitThrowableActor = HitResult.bBlockingHit ? Cast<AThrowableActor>(HitResult.GetActor()) : nullptr;
	const bool IsSameActor = (ThrowableActor == HitThrowableActor);
	const bool IsValidTarget = HitThrowableActor && HitThrowableActor->IsIdle();

	//clean up old actor
	if (ThrowableActor && (!IsValidTarget || !IsSameActor))
	{
		ThrowableActor->ToggleHighlight(false);
		ThrowableActor = nullptr;
	}

	//no target, early out
	if (!IsValidTarget)
	{
		return;
	}

	//new target, set the variable and proceed
	if (!IsSameActor)
	{
		ThrowableActor = HitThrowableActor;
		if (bHighlight)
		{
			ThrowableActor->ToggleHighlight(true);
		}
	}

	if (CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		//don't allow for pulling objects while running/jogging
		if (GetVelocity().SizeSquared() < 100.0f)
		{
			ServerPullObject(ThrowableActor);
			CharacterThrowState = ECharacterThrowState::Pulling;
			ThrowableActor->ToggleHighlight(false);
		}
	}
}

bool ADoorsDoorsDoorsPlayerCharacter::PlayThrowMontage()
{
	const float PlayRate = 1.0f;
	const FName StartSectionName = IsAiming() ? TEXT("AimStart") : TEXT("Default");
	bool bPlayedSuccessfully = PlayAnimMontage(ThrowMontage, PlayRate, StartSectionName) > 0.f;
	if (bPlayedSuccessfully)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (!BlendingOutDelegate.IsBound())
		{
			BlendingOutDelegate.BindUObject(this, &ADoorsDoorsDoorsPlayerCharacter::OnMontageBlendingOut);
		}
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ThrowMontage);

		if (!MontageEndedDelegate.IsBound())
		{
			MontageEndedDelegate.BindUObject(this, &ADoorsDoorsDoorsPlayerCharacter::OnMontageEnded);
		}
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ThrowMontage);

		if (IsLocallyControlled())
		{
			AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ADoorsDoorsDoorsPlayerCharacter::OnNotifyBeginReceived);
			AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ADoorsDoorsDoorsPlayerCharacter::OnNotifyEndReceived);
		}

	}

	return bPlayedSuccessfully;
}

void ADoorsDoorsDoorsPlayerCharacter::UpdateThrowMontagePlayRate()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (UAnimMontage* CurrentAnimMontage = AnimInstance->GetCurrentActiveMontage())
		{
			//speed up the playrate when at the throwing part of the animation, as the initial interaction animation wasn't intended as a throw so it's rather slow
			const float PlayRate = AnimInstance->GetCurveValue(TEXT("ThrowCurve"));
			AnimInstance->Montage_SetPlayRate(CurrentAnimMontage, PlayRate);
		}
	}
}

void ADoorsDoorsDoorsPlayerCharacter::UnbindMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ADoorsDoorsDoorsPlayerCharacter::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ADoorsDoorsDoorsPlayerCharacter::OnNotifyEndReceived);
	}
}

void ADoorsDoorsDoorsPlayerCharacter::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
}

void ADoorsDoorsDoorsPlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (IsLocallyControlled())
	{
		UnbindMontage();
	}

	if (Montage == ThrowMontage)
	{
		if (IsLocallyControlled())
		{
			CharacterThrowState = ECharacterThrowState::None;
			ServerFinishThrow();
			ThrowableActor = nullptr;
		}
	}
	/*else if (Montage == CelebrateMontage)
	{
		if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
		{
			if (TantrumnPlayerState->IsWinner())
			{
				float length = PlayAnimMontage(CelebrateMontage, 1.0f, TEXT("Winner"));
				ensureAlwaysMsgf(length > 0.f, TEXT("ATantrumnCharacterBase::OnMontageEnded Could Not Player Winner Animation"));
			}
		}
	}*/
}

void ADoorsDoorsDoorsPlayerCharacter::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	//do this on server, since server owns the object we are throwing...
	ServerBeginThrow();
}


void ADoorsDoorsDoorsPlayerCharacter::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{

}

void ADoorsDoorsDoorsPlayerCharacter::OnStunBegin(float StunRatio)
{
	if (bIsStunned)
	{
		//for now just early exit, alternative option would be to add to the stun time
		return;
	}

	const float StunDelt = MaxStunTime - MinStunTime;
	StunTime = MinStunTime + (StunRatio * StunDelt);
	CurrentStunTimer = 0.0f;
	//StunBeginTimestamp = FApp::GetCurrentTime();
	bIsStunned = true;
	if (bIsSprinting)
	{
		RequestSprintEnd();
	}
}

void ADoorsDoorsDoorsPlayerCharacter::UpdateStun(float DeltaTime)
{
	if (bIsStunned)
	{
		CurrentStunTimer += DeltaTime;
		bIsStunned = CurrentStunTimer < StunTime;
		//bIsStunned = (FApp::GetCurrentTime() - StunBeginTimestamp) < StunTime;
		if (!bIsStunned)
		{
			OnStunEnd();
		}
	}
}

void ADoorsDoorsDoorsPlayerCharacter::OnStunEnd()
{
	CurrentStunTimer = 0.0f;
	//StunBeginTimestamp = 0.0f;
	StunTime = 0.0f;
}

void ADoorsDoorsDoorsPlayerCharacter::OnRep_CharacterThrowState(const ECharacterThrowState& OldCharacterThrowState)
{
	if (CharacterThrowState != OldCharacterThrowState)
	{
		UE_LOG(LogTemp, Warning, TEXT("OldThrowState: %s"), *UEnum::GetDisplayValueAsText(OldCharacterThrowState).ToString());
		UE_LOG(LogTemp, Warning, TEXT("CharacterThrowState: %s"), *UEnum::GetDisplayValueAsText(CharacterThrowState).ToString());
	}
}

void ADoorsDoorsDoorsPlayerCharacter::ApplyEffect_Implementation(EEffectType EffectType, bool bIsBuff)
{
	if (bIsUnderEffect) return;

	CurrentEffect = EffectType;
	bIsUnderEffect = true;
	bIsEffectBuff = bIsBuff;

	switch (CurrentEffect)
	{
	case EEffectType::Speed:
		bIsEffectBuff ? SprintSpeed *= 2 : GetCharacterMovement()->DisableMovement();
		break;
	case EEffectType::Jump:
		// Implement Jump Buff/Debuff
		break;
	case EEffectType::Power:
		// Implement Power Buff/Debuff
		break;
	default:
		break;
	}
}

void ADoorsDoorsDoorsPlayerCharacter::UpdateEffect(float DeltaTime)
{
	if (EffectCooldown > 0)
	{
		EffectCooldown -= DeltaTime;
	}
	else
	{
		bIsUnderEffect = false;
		EffectCooldown = DefautlEffectCooldown;
		EndEffect();
	}
}

void ADoorsDoorsDoorsPlayerCharacter::EndEffect()
{
	bIsUnderEffect = false;
	switch (CurrentEffect)
	{
	case EEffectType::Speed:
		bIsEffectBuff ? SprintSpeed /= 2, RequestSprintEnd() : GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		break;
	case EEffectType::Jump:
		// Implement Jump Buff/Debuff
		break;
	case EEffectType::Power:
		// Implement Power Buff/Debuff
		break;
	default:
		break;
	}
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