// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Http.h"
#include "Containers/Map.h"
#include "SkillEnums.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "ProjectUMCharacter.generated.h"

class UNiagaraSystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthAndBlockUpdateEvent, const TArray<float>&, _Items);
UCLASS(config=Game)
class AProjectUMCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void ProcessAnimationQueue();

public:
	UPROPERTY(BlueprintAssignable, Category = "Stat Update")
		FHealthAndBlockUpdateEvent OnHealthAndBlockUpdate;

	UFUNCTION(Client, Reliable)
		void UpdateHealthAndBlockMeter();

	void UpdateHealthAndBlockMeter_Implementation();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_MaxBlockMeter)
		float BlockMeterMax = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Block", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CurrentBlockMeter)
		float BlockMeterCurrent = 100.0f;


	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_MaxHealth)
		float MaxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	void StartAnimationMontage(UAnimMontage* AnimMontage);
	TQueue<UAnimMontage*> AnimationQueue;

	UFUNCTION(NetMulticast, Reliable)
		void PlayProjectUMCharacterAnimMontage(UAnimMontage* AnimMontage, FName StartSectionName);

	void PlayProjectUMCharacterAnimMontage_Implementation(UAnimMontage* AnimMontage, FName StartSectionName);

	UFUNCTION(NetMulticast, Reliable)
		void PlayAnimationMontageWithOverride(UAnimMontage* Montage);

	void PlayAnimationMontageWithOverride_Implementation(UAnimMontage* Montage);

	UFUNCTION()
		class UCameraComponent* GetFollowCamera() {
			return FollowCamera;
		}

	AProjectUMCharacter();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentHealth(float healthValue);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentMaxHealth(float healthValue);

	UFUNCTION(BlueprintPure, Category = "BlockMeter")
		FORCEINLINE float GetMaxBlockMeter() const { return BlockMeterMax; }

	UFUNCTION(BlueprintPure, Category = "BlockMeter")
		FORCEINLINE float GetCurrentBlockMeter() const { return BlockMeterCurrent; }

	UFUNCTION(BlueprintCallable, Category = "BlockMeter")
		void SetCurrentBlockMeter(float BlockMeterValue);

	UFUNCTION(BlueprintCallable, Category = "BlockMeter")
		void SetMaxBlockMeter(float BlockMeterValue);

	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* DanceMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* BlockMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* EndBlockMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		TArray<class UAnimMontage*> HitstunMontages;
	int32 HitstunIndex;
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Block", meta = (AllowPrivateAccess = "true"))
		bool bIsBlocking;

	UPROPERTY(EditDefaultsOnly, Category = "Hitstun")
		float HitstunDuration;

	UFUNCTION()
		void StartBlock();

	UFUNCTION(Server, Reliable)
		void HandleStartBlock();
	void HandleStartBlock_Implementation();

	UFUNCTION()
		void StopBlock();

	UFUNCTION(Server, Reliable)
		void HandleStopBlock();
	void HandleStopBlock_Implementation();

	UFUNCTION()
		void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	UPROPERTY()
		FTimerHandle HitstunTimerHandle;

	UPROPERTY()
		bool bIsInHitstun;

	void EndHitstun()
	{
		bIsInHitstun = false;
	}

	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void CrouchStarted();

	void CrouchStopped();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
		void OnRep_MaxHealth();

	void OnHealthUpdate();

	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseMaxHealth;

	UFUNCTION()
		void OnRep_CurrentBlockMeter();

	UFUNCTION()
		void OnRep_MaxBlockMeter();

	void OnBlockMeterUpdate();

	

	UPROPERTY(BlueprintReadOnly, Category = "Block", meta = (AllowPrivateAccess = "true"))
		float BlockRegenerationRate = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		TMap<ESkillEnums, UAnimMontage*> SkillAnimMontageMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		TMap<ESkillEnums, UBoxComponent*> SkillHurtboxes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
		UNiagaraSystem* SparkEffectComponent;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		virtual void StartJump();

	FTimerHandle PrimaryAttackTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TArray<float> PrimaryAttackRates;

	int32 PrimaryAttackIndex;

	bool bIsPrimaryAttacking;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartPrimaryAttack();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		virtual void StopPrimaryAttack();

	UFUNCTION(Server, Reliable)
		virtual void HandlePrimaryAttack();

	virtual void HandlePrimaryAttack_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartSkill(int32 SkillIndex);

	UFUNCTION(Server, Reliable)
		virtual void HandleSkill(int32 SkillIndex);

	virtual void HandleSkill_Implementation(int32 SkillIndex);

	virtual void StopSkill(int32 SkillIndex);

	bool bIsDashing;

	float DashRate;

	FTimerHandle DashTimer;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartDash();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		virtual void StopDash();

	UFUNCTION(Server, Reliable)
		virtual void HandleDash();

	virtual void HandleDash_Implementation();

public:
	UFUNCTION(Server, Reliable)
		void Dance();

	void Dance_Implementation();

	void Disconnect();

	UFUNCTION(Client, Reliable)
		void DisconnectClient();

	void DisconnectClient_Implementation();

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TMap<int32, ESkillEnums> SkillIndexToEnumMap;
	TMap<ESkillEnums, float> SkillCooldowns;
	TMap<ESkillEnums, bool> SkillOnCooldown;
	TMap<ESkillEnums, FTimerHandle> SkillTimers;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (EditInline))
	TMap<FName, UBoxComponent*> HitboxAppendageMap;

};

