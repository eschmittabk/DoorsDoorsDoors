// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "IDetailTreeNode.h"
#include "InteractInterface.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"
#include "DoorsDoorsDoorsPlayerCharacter.generated.h"

class UDamageHandlerComponent;
class UHealthComponent;
class UParticleSystemComponent;
class AThrowableActor;

UENUM(BlueprintType)
enum class ECharacterThrowState : uint8
{
	None			UMETA(DisplayName = "None"),
	RequestingPull	UMETA(DisplayName = "RequestingPull"),
	Pulling			UMETA(DisplayName = "Pulling"),
	Attached		UMETA(DisplayName = "Attached"),
	Throwing		UMETA(DisplayName = "Throwing"),
	Aiming			UMETA(DisplayName = "Aiming"),
};

//these are input bindings
//DECLARE_MULTICAST_DELEGATE(FInteractionStartRequest);
DECLARE_MULTICAST_DELEGATE_OneParam(FInteractionStartRequest, class AActor*);
DECLARE_MULTICAST_DELEGATE(FInteractionCancelRequest);

UCLASS()
class DOORSDOORSDOORS_API ADoorsDoorsDoorsPlayerCharacter : public ACharacter, public IInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	//ADoorsDoorsDoorsPlayerCharacter();

	/** Default UObject constructor. */
	ADoorsDoorsDoorsPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	/** Called when the actor falls out of the world 'safely' (below KillZ and such) */
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	void RequestSprintStart();
	void RequestSprintEnd();

	void RequestThrowObject();
	void RequestPullObject();
	void RequestAim();

	void RequestStopPullObject();
	void RequestStopAim();
	void ResetThrowableObject();

	void RequestUseObject();

	void OnThrowableAttached(AThrowableActor* InThrowableActor);
	//this can now be a mask/bitflag as both can be true
	bool CanThrowObject() const { return CharacterThrowState == ECharacterThrowState::Attached || CharacterThrowState == ECharacterThrowState::Aiming; }

	UFUNCTION(BlueprintPure)
		bool IsPullingObject() const { return CharacterThrowState == ECharacterThrowState::RequestingPull || CharacterThrowState == ECharacterThrowState::Pulling; }

	//this function is mainly for AI
	//makes sure the linetrace is a success, and bypasses the tracing done in Tick()
	UFUNCTION(BlueprintCallable)
		bool AttemptPullObjectAtLocation(const FVector& InLocation);

	UFUNCTION(BlueprintPure)
		bool IsThrowing() const { return CharacterThrowState == ECharacterThrowState::Throwing; }

	UFUNCTION(BlueprintPure)
		bool CanAim() const { return CharacterThrowState == ECharacterThrowState::Attached; }

	UFUNCTION(BlueprintPure)
		bool IsAiming() const { return CharacterThrowState == ECharacterThrowState::Aiming; }

	UFUNCTION(BlueprintPure)
		ECharacterThrowState GetCharacterThrowState() const { return CharacterThrowState; }

	UFUNCTION(BlueprintPure)
		bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintCallable)
		void NotifyHitByThrowable(AThrowableActor* InThrowable);

	UFUNCTION(BlueprintPure)
		bool IsHovering() const;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		const bool isAlive() const;

	UFUNCTION(BlueprintCallable)
		const float getCurrentHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "DoorsDoorsDoors")
		void SetOnFire(float BaseDamage, float DamageTotalTime, float TakeDamageInterval);

	UFUNCTION(BlueprintCallable)
		void HandleItemCollected();

	UFUNCTION(BlueprintImplementableEvent)
		void ItemCollected();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int ItemsCollected = 0;

	//bindings, a hack atm as the interactble components in the game world get the player and sign themselves up to these events
	//to know when the player has pressed the input binding for interacting
	FInteractionStartRequest OnInteractionStartRequested;
	FInteractionCancelRequest OnInteractionCancelRequested;

	UFUNCTION(BlueprintImplementableEvent)
		void DoorOpenInteractionStarted(AActor* InteractableActor);

	//this can be an array or moved later as needed
	UPROPERTY(EditAnywhere)
		UParticleSystemComponent* ParticleSystemComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SphereCastPlayerView();
	void SphereCastActorTransform();
	void LineCastActorTransform();
	void ProcessTraceResult(const FHitResult& HitResult, bool bHighlight = true);


	//RPC's actions that can need to be done on the server in order to replicate
	UFUNCTION(Server, Reliable)
		void ServerSprintStart();

	UFUNCTION(Server, Reliable)
		void ServerSprintEnd();

	UFUNCTION(Server, Reliable)
		void ServerPullObject(AThrowableActor* InThrowableActor);

	UFUNCTION(Server, Reliable)
		void ServerRequestPullObject(bool bIsPulling);

	UFUNCTION(Server, Reliable)
		void ServerRequestToggleAim(bool IsAiming);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerRequestThrowObject();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastRequestThrowObject();

	UFUNCTION(Client, Reliable)
		void ClientThrowableAttached(AThrowableActor* InThrowableActor);

	UFUNCTION(Server, Reliable)
		void ServerBeginThrow();

	UFUNCTION(Server, Reliable)
		void ServerFinishThrow();

	bool PlayThrowMontage();
	void UpdateThrowMontagePlayRate();
	void UnbindMontage();

	UFUNCTION()
		void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
		void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
		void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
		void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	void OnStunBegin(float StunRatio);
	void UpdateStun(float DeltaTime);
	void OnStunEnd();

	UPROPERTY(EditAnywhere, Category = "Movement")
		float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
		float MinImpactSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
		float MaxImpactSpeed = 1200.0f;

	//Time in Seconds
	UPROPERTY(EditAnywhere, Category = "Fall Impact")
		float MinStunTime = 1.0f;
	//Time in Seconds
	UPROPERTY(EditAnywhere, Category = "Fall Impact")
		float MaxStunTime = 1.0f;

	//Sound Cue Fall Impact
	UPROPERTY(EditAnywhere, Category = "Fall Impact")
		USoundCue* HeavyLandSound = nullptr;

	float StunTime = 0.0f;
	//float StunBeginTimestamp = 0.0f;
	float CurrentStunTimer = 0.0f;

	bool bIsStunned = false;
	bool bIsSprinting = false;

	float MaxWalkSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterThrowState, Category = "Throw")
		//UPROPERTY(VisibleAnywhere, replicated, Category = "Throw")
		ECharacterThrowState CharacterThrowState = ECharacterThrowState::None;

	UFUNCTION()
		void OnRep_CharacterThrowState(const ECharacterThrowState& OldCharacterThrowState);

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0", Unit = "ms"))
		float ThrowSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Animation")
		UAnimMontage* ThrowMontage = nullptr;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;

	void OnDeath(bool IsFellOut);

	UFUNCTION()
	void OnDeathTimerFinished();

	//Input Bindings
	void InteractionStartRequested();
	void InteractionCancelRequested();

	UPROPERTY(EditAnywhere)
		UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere)
		UDamageHandlerComponent* DamageHandlerComponent;

	UPROPERTY(EditAnywhere)
		float TimeRestartLevelAfterDeath = 2.0f;

	//Handle to manage the death timer
	FTimerHandle RestartLevelTimerHandle;

	APlayerController* PC;

	//UPROPERTY(EditAnywhere, Category = "Effects")
		//TSubclassOf<UCameraShake> CamShake;
		//TSubclassOf<UMatineeCameraShake> CamShake;

	// Force Feedback values.
	UPROPERTY(EditAnywhere, Category = "Force Feedback")
		float ForceFeedbackIntensity = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Force Feedback")
		float ForceFeedbackDuration = 1.0f;

private:

	UPROPERTY()
		AThrowableActor* ThrowableActor;

	void ApplyEffect_Implementation(EEffectType EffectType, bool bIsBuff) override;
	void UpdateEffect(float DeltaTime);
	void EndEffect();

	bool bIsUnderEffect = false;
	bool bIsEffectBuff = false;

	float DefautlEffectCooldown = 5.0f;
	float EffectCooldown = 0.0f;

	EEffectType CurrentEffect = EEffectType::None;
};