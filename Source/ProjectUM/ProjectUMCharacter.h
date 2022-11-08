// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectUMCharacter.generated.h"

UENUM(BlueprintType)
namespace AttackTypeEnum
{
	enum AttackType
	{
		FIST UMETA(DisplayName = "FIST ATTACK"),
		MELEE_WEAPON = 1  UMETA(DisplayName = "MELEE WEAPON ATTACK")
	};
}

UCLASS(config=Game)
class AProjectUMCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/** Inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iventory", meta = (AllowPrivateAccess = "true"))
		class UProjectUMInventoryComponent* Inventory;

public:
	AProjectUMCharacter();

	// Called after constructor
	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Attack")
		FORCEINLINE bool IsAttacking() const { return bIsAttacking; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* MeleeAttackMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* FistComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class AProjectUMWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Parameters")
		TSubclassOf<AProjectUMWeapon> WeaponClass;

	UFUNCTION()
		void OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	UPROPERTY()
		TEnumAsByte<AttackTypeEnum::AttackType> AttackType;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	void CrouchStarted();

	void CrouchStopped();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** The player's maximum health. This is the highest value of their health can be. This value is a value of the player's health, which starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
		void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
		TSubclassOf<class AProjectUmProjectile> ProjectileClass;

	/** Delay between shots in seconds. Used to control fire rate for your test projectile, but also to prevent an overflow of server functions from binding SpawnProjectile directly to input.*/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float FireRate;

	/** A timer handle used for providing the Attack rate delay in-between attacks.*/
	FTimerHandle FiringTimer;

	/** If true, you are in the process of firing projectiles. */
	bool bIsFiringWeapon;

	/** Function for beginning weapon fire.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartFire();

	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFire();

	/** Server function for spawning projectiles.*/
	UFUNCTION(Server, Reliable)
		void HandleFire();

	/** A timer handle used for providing the Attack rate delay in-between attacks.*/
	FTimerHandle AttackingTimer;

	/** Delay between Attacks in seconds. Used to control Attack rate.*/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float AttackRate;

	/** If true, you are in the process of firing projectiles. */
	bool bIsAttacking;

	/** Function for beginning attack.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartAttack();

	/** Function for ending attack. Once this is called, the player can use StartAttack again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopAttack();

	/** Server function for attack.*/
	UFUNCTION(Server, Reliable)
		void HandleAttack();

	void HandleAttack_Implementation();

	/** Delay between Attacks in seconds. Used to control Attack rate.*/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float EquipRate;
	/** A timer handle used for providing the Attack rate delay in-between attacks.*/
	FTimerHandle EquippingTimer;

	/** If true, you are in the process of firing projectiles. */
	bool bIsEquipping;

	/** Function for beginning attack.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartEquipping();

	/** Function for ending attack. Once this is called, the player can use StartAttack again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopEquipping();

	/** Server function for attack.*/
	UFUNCTION(Server, Reliable)
		void HandleEquipWeapon();

	void HandleEquipWeapon_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void PlayProjectUMCharacterAnimMontage(UAnimMontage* AnimMontage);

	UFUNCTION(Server,Reliable, BlueprintCallable, Category = "Item")
		void UseItem(class UProjectUMItem* Item);

	void UseItem_Implementation(class UProjectUMItem* Item);

};

