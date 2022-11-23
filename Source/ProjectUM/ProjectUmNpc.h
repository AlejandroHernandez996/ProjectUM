// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableObjectInterface.h"
#include "NpcState.h"
#include "ProjectUmNpc.generated.h"

UCLASS()
class PROJECTUM_API AProjectUmNpc : public ACharacter, public IInteractableObjectInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AProjectUmNpc();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

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

	void Interact_Implementation(AProjectUMCharacter* Interactor) override;

	UFUNCTION()
		UProjectUMInventoryComponent* GetInventory() { return Inventory; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* DeathMontage;
protected:
	UFUNCTION(NetMulticast, Reliable)
		void PlayNpcAnimMontage(UAnimMontage* AnimMontage);

	void PlayNpcAnimMontage_Implementation(UAnimMontage* AnimMontage);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		class UProjectUMInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		class UProjectUMLootTable* LootTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
		class UPawnSensingComponent* PawnSensor;

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UParticleSystem* ExplosionEffect;

	/** The player's maximum health. This is the highest value of their health can be. This value is a value of the player's health, which starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		float LeashRange;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FVector SpawnLocation;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		APawn* FocusPawn;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		ENpcState State;

	UFUNCTION()
		void OnSeePawn(APawn* OtherPawn);

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
		void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle AttackingTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
		float AttackRate;

	bool bIsAttacking;

	UFUNCTION(BlueprintCallable, Category = "Attack")
		void StartAttack();

	UFUNCTION(BlueprintCallable, Category = "Attack")
		void StopAttack();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void HandleAttack();

	UFUNCTION()
		void OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* AttackHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = "true"))
		float AttackDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* MeleeAttackMontage;

	UFUNCTION(NetMulticast, Reliable)
		void PlayProjectUMCharacterAnimMontage(UAnimMontage* AnimMontage);

	void PlayProjectUMCharacterAnimMontage_Implementation(UAnimMontage* AnimMontage);

	UPROPERTY()
		TSet<FString> AttackedCharactersSet;

	int LastTick;
};
