// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableObjectInterface.h"
#include "ProjectUMCharacter.generated.h"

UCLASS(config=Game)
class AProjectUMCharacter : public ACharacter, public IInteractableObjectInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iventory", meta = (AllowPrivateAccess = "true"))
		class UProjectUMInventoryComponent* Inventory;

public:
	AProjectUMCharacter();

	UFUNCTION()
		UProjectUMInventoryComponent* GetInventory() { return Inventory; }

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

	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Attack")
		FORCEINLINE bool IsAttacking() const { return bIsAttacking; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* MeleeAttackMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* FistComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* InteractComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Inventory")
		TMap<EEquippableSlotsEnum, AProjectUMEquipment*> EquippedItemMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
		TMap<EEquippableSlotsEnum, TSubclassOf<AProjectUMEquipment>> EquipmentClassMap;

	UPROPERTY()
		TMap<EEquippableSlotsEnum, FName> EquipSlotSkeletonMapping;

		TSet<IInteractableObjectInterface*> InteractableObjects;

	UFUNCTION()
		TSubclassOf<AProjectUMEquipment> GetEquipmentClass(EEquippableSlotsEnum EquipmentSlot) { return EquipmentClassMap[EquipmentSlot]; }

	UFUNCTION()
		AProjectUMEquipment* GetEquippedItem(EEquippableSlotsEnum EquipmentSlot) { return EquippedItemMap[EquipmentSlot]; }

	UFUNCTION()
		void OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void OnInteractOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnInteractOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void AttachEquipment(TSubclassOf<AProjectUMEquipment> Equipment, EEquippableSlotsEnum EquipSlot);

	UFUNCTION()
	void DeAttachEquipment(EEquippableSlotsEnum EquipSlot);

protected:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void CrouchStarted();

	void CrouchStopped();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
		float MaxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float BaseMaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
		void OnRep_MaxHealth();

	void OnHealthUpdate();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
		TSubclassOf<class AProjectUmProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float FireRate;

	FTimerHandle FiringTimer;

	bool bIsFiringWeapon;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFire();

	UFUNCTION(Server, Reliable)
		void HandleFire();

	UPROPERTY()
		TSet<FString> AttackedCharactersSet;

	FTimerHandle AttackingTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float AttackRate;

	bool bIsAttacking;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartAttack();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopAttack();

	UFUNCTION(Server, Reliable)
		void HandleAttack();

	void HandleAttack_Implementation();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float EquipRate;
	FTimerHandle EquippingTimer;

	bool bIsEquipping;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartEquipping(EEquippableSlotsEnum EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopEquipping();

	UFUNCTION()
		void HandleEquip(EEquippableSlotsEnum EquipSlot);

	UFUNCTION(NetMulticast, Reliable)
		void PlayProjectUMCharacterAnimMontage(UAnimMontage* AnimMontage);


	UPROPERTY(EditDefaultsOnly, Category = "Item")
		float UseItemRate;

	FTimerHandle UseItemTimer;

	bool bIsUsingItem;

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = "Item")
		void StartUsingItem(class UProjectUMItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Item")
		void StopUsingItem();

	UFUNCTION(Server, Reliable, Category = "Item")
		void HandleUseItemServer(class UProjectUMItem* Item);

	void HandleUseItemServer_Implementation(class UProjectUMItem* Item);

	UFUNCTION(NetMulticast, Reliable, Category = "Item")
		void HandleUseItemClient(class UProjectUMItem* Item);

	void HandleUseItemClient_Implementation(class UProjectUMItem* Item);

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
		float InteractRate = .25f;

	FTimerHandle InteractTimer;

	bool bIsInteracting;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void StartInteracting();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void StopInteracting();

	UFUNCTION(NetMulticast, Reliable, Category = "Interaction")
		void HandleInteraction();

	void HandleInteraction_Implementation();

	void Interact_Implementation(AProjectUMCharacter* Interactor) override;

	UFUNCTION(NetMulticast, Reliable)
		void SpawnItems();

	void SpawnItems_Implementation();
};

